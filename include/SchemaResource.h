#ifndef SCHEMARESOURCE_H
#define SCHEMARESOURCE_H
#include "DraftRecognisedDocument.h"
#include "JSONPointer.h"
#include "SetMap.h"
#include "UriWrapper.h"
#include <string>

/// @brief Represents a schema resource - a JSON value found in a schema.
/// One or more URIs can reference the same schema resource.
class SchemaResource {
public:
  /// @brief The json content of the schema. This does not own the JSON
  /// content, and the JSON content must outlive this object.
  std::reference_wrapper<const nlohmann::json> json_;
  /// @brief Pointer to the current schema from baseUri_
  /// @brief The base URI of the schema
  UriWrapper baseUri_;

  /// @brief The draft of the schema
  Draft draft_;

  /// @brief Pointer to the current schema from baseUri_
  JSONPointer pointer_ = JSONPointer();

  SchemaResource() = delete;
  explicit SchemaResource(const DraftRecognisedDocument& draft)
      : json_(std::cref(draft.json_)), baseUri_(draft.fileUri_),
        draft_(draft.draft_) {}
  SchemaResource(const nlohmann::json& json, UriWrapper baseUri,
                 JSONPointer pointer, Draft draft = Draft::DRAFT_07)
      : json_(json), baseUri_(baseUri), draft_(draft), pointer_(pointer) {}

private:
  void addChildItemToMap(SetMap<UriWrapper, SchemaResource>& setMap,
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
    SchemaResource child(value, baseUri_, pointer_ / key);
    child.recursiveAddToMap(setMap, childRefs);
  }

  void recursiveAddToMap(SetMap<UriWrapper, SchemaResource>& setMap,
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
  static SetMap<UriWrapper, SchemaResource>
  generateSetMap(const std::vector<DraftRecognisedDocument>& docs) {
    try {
      SetMap<UriWrapper, SchemaResource> setMap;
      for (const auto& doc : docs) {
        std::set<UriWrapper> refs;
        refs.insert(doc.fileUri_);
        SchemaResource schema(doc);
        schema.recursiveAddToMap(setMap, refs);
      }
      return setMap;
    } catch (const std::exception& e) {
      std::cerr << "Error caught in SchemaResource::generateSetMap:\n";
      throw e;
    }
  }

  static void dumpSchemas(SetMap<UriWrapper, SchemaResource>& schemaResources,
                          std::filesystem::path outputDirectory = ".") {
    nlohmann::json schemaResourceDump = nlohmann::json::object();
    // iterate over all the schemas.
    for (const auto& [uris, schema] : schemaResources) {
      auto& baseUriList =
          schemaResourceDump[schema.get().baseUri_.toString().value()];
      if (!baseUriList[schema.get().pointer_.toFragment()].is_array()) {
        baseUriList[schema.get().pointer_.toFragment()] =
            nlohmann::json::array();
      }
      for (const auto& uri : uris.get()) {
        baseUriList[schema.get().pointer_.toFragment()].push_back(
            {{"uri", uri.toFragmentlessString().value_or("")},
             {"frag", uri.getFragment().value_or("")}});
      }
    }
    std::ofstream resourceDump(outputDirectory / "resource.dump.json");
    resourceDump << schemaResourceDump.dump(2);
    resourceDump.close();
  }
};

#endif // SCHEMARESOURCE_H