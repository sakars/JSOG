#ifndef UNRESOLVEDSCHEMA_H
#define UNRESOLVEDSCHEMA_H
#include "DraftRecognisedDocument.h"
#include "JSONPointer.h"
#include "SetMap.h"
#include "UriWrapper.h"
#include <string>

class UnresolvedSchema {
public:
  /// @brief The json content of the schema. This does not own the JSON
  /// content, and the JSON content must outlive this object.
  std::reference_wrapper<const nlohmann::json> json_;
  /// @brief Pointer to the current schema from baseUri_
  /// @brief The base URI of the schema
  UriWrapper baseUri_;

  Draft draft_;

  /// @brief Pointer to the current schema from baseUri_
  JSONPointer pointer_ = JSONPointer();

  UnresolvedSchema() = delete;
  explicit UnresolvedSchema(const DraftRecognisedDocument& draft)
      : json_(std::cref(draft.json_)), baseUri_(draft.fileUri_),
        draft_(draft.draft_) {}
  UnresolvedSchema(const nlohmann::json& json, UriWrapper baseUri,
                   JSONPointer pointer, Draft draft = Draft::DRAFT_07)
      : json_(json), baseUri_(baseUri), draft_(draft), pointer_(pointer) {}

private:
  void addChildItemToMap(SetMap<UriWrapper, UnresolvedSchema>& setMap,
                         std::set<UriWrapper> refs, std::string key,
                         const nlohmann::json& value) {
    if (!(value.is_object() || value.is_array() || value.is_boolean())) {
      return;
    }
    std::set<UriWrapper> childRefs;
    for (auto childRef : refs) {
      childRef.normalize();
      JSONPointer childPointer =
          JSONPointer::fromURIString(childRef.getFragment().value_or(""),
                                     false) /
          key;
      childRef.setFragment(childPointer.toFragment(false), false);
      childRefs.emplace(childRef);
    }
    UnresolvedSchema child(value, baseUri_, pointer_ / key);
    child.recursiveAddToMap(setMap, childRefs);
  }

  void recursiveAddToMap(SetMap<UriWrapper, UnresolvedSchema>& setMap,
                         std::set<UriWrapper> refs) {
    const auto& json = json_.get();
    // If this object has an id, we must change the base uri to the id for
    // further correct uri resolution.
    if (json.is_object() && json.contains("$id") && json["$id"].is_string()) {
      std::string id = json["$id"].get<std::string>();
      UriWrapper uri(id);
      UriWrapper base(baseUri_);
      UriWrapper fullUri = UriWrapper::applyUri(base, uri);
      const auto fragment = fullUri.getFragment().value_or("");
      if (fragment == "") {
        baseUri_ = fullUri.toFragmentlessString().value();
        pointer_ = JSONPointer();
        refs.emplace(baseUri_);
      } else {
        std::cerr << "Warning: $id resolution with fragment is not supported."
                  << std::endl;
      }
    }

    // Anchors register an object to be accessible from a different URI
    if (json.is_object() && json.contains("anchor") &&
        json["anchor"].is_string()) {
      std::string anchor = json["anchor"].get<std::string>();
      UriWrapper base(baseUri_);
      UriWrapper anchorUri("#" + anchor);
      UriWrapper fullUri = UriWrapper::applyUri(base, anchorUri);
      refs.emplace(fullUri);
    }

    if (json.is_object()) {
      for (auto& v : json.items()) {
        const auto key = v.key();
        const auto& value = v.value();
        addChildItemToMap(setMap, refs, key, value);
      }
    } else if (json.is_array()) {
      for (size_t i = 0; i < json.size(); i++) {
        std::string key = std::to_string(i);
        const auto& value = json[i];
        addChildItemToMap(setMap, refs, key, value);
      }
    }

    setMap.bulkInsert(refs, *this);
  }

public:
  static SetMap<UriWrapper, UnresolvedSchema>
  generateSetMap(const std::vector<DraftRecognisedDocument>& docs) {
    try {
      SetMap<UriWrapper, UnresolvedSchema> setMap;
      for (const auto& doc : docs) {
        std::set<UriWrapper> refs;
        refs.insert(doc.fileUri_);
        UnresolvedSchema schema(doc);
        schema.recursiveAddToMap(setMap, refs);
      }
      return setMap;
    } catch (const std::exception& e) {
      std::cerr << "Error caught in UnresolvedSchema::generateSetMap:\n";
      throw e;
    }
  }

  static void
  dumpSchemas(SetMap<UriWrapper, UnresolvedSchema>& unresolvedSchemas) {
    nlohmann::json unresolvedSchemaDump = nlohmann::json::object();
    for (const auto& [uris, schema] : unresolvedSchemas) {
      auto& baseUriList =
          unresolvedSchemaDump[schema.get().baseUri_.toString().value()];
      baseUriList[schema.get().pointer_.toFragment()] = nlohmann::json::array();
      for (const auto& uri : uris.get()) {
        baseUriList[schema.get().pointer_.toFragment()].push_back(
            nlohmann::json::array({uri.toFragmentlessString().value_or(""),
                                   uri.getFragment().value_or("")}));
      }
    }
    std::ofstream unresolvedDump("unresolved.dump.json");
    unresolvedDump << unresolvedSchemaDump.dump(2);
    unresolvedDump.close();
  }
};

#endif // UNRESOLVEDSCHEMA_H