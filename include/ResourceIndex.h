#ifndef RESOURCEINDEX_H
#define RESOURCEINDEX_H

#include "JSONPointer.h"
#include "Schema.h"
#include "SchemaInitializer.h"
#include "UriWrapper.h"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

/// @brief The central index of known json elements.
/// @details This index maps out json instances for schemmas and constructs
/// schema instances where applicable.
class ResourceIndex {
public:
  /// @brief Represents a single json shallow object.
  /// @details A json shallow object means that this resource only really cares
  /// about the value of this current json object, not it's children (if
  /// applicable) If this object is determined to be an evaluatable schema, the
  /// Schema pointer will become non-nullptr.
  struct Resource {
    /// @brief A reference to the shallow json object it manages.
    const nlohmann::json &json;
    /// @brief A string representing the base of the URI of the resource
    const std::string baseUri;
    /// @brief A JSON Pointer to the resource relative to the base URI
    const JSONPointer jsonPointer;

    /// @brief Which draft this schema is most likely adhering to
    Draft draft = Draft::UNKNOWN;

    /// @brief An instance of a schema object that is generated from the  json
    /// file.
    /// @note This schema object may be empty if the index hasn't determined
    /// that this is indeed a schema that should have it's object generated.
    std::unique_ptr<Schema> schema;
  };

private:
  /// @brief Main map of resources that holds URI keys that map to the shallow
  /// json reference resource.
  std::map<std::string, std::shared_ptr<Resource>> resources;
  /// @brief This set of pointers mark which resources should be built, but yet
  /// have not been.
  std::set<std::shared_ptr<Resource>> resourcesRequiringBuilding;
  /// @brief This set of pointers mark which resources have been built.
  std::set<std::shared_ptr<Resource>> resourcesBuilt;

  /// @brief Normalizes a uri string to ensure best possible map key access.
  /// @details Non-normalized uris may be equivalent, but not be recognized as
  /// equivalent in a map. This method normalizes strings to a canonical form,
  /// so equivalent URIs are resolved to the same map.
  static std::string normalizeUri(const std::string &uri) {
    UriWrapper uri_(uri);
    std::string fragment = uri_.getFragment().value_or("");
    uri_.setFragment(fragment, false);
    uri_.normalize();
    return uri_.toString().value();
  }

public:
  /// @brief Adds a resource to the index
  /// @param json A JSON object reference to the resource
  /// @param refs A set of known references to the resource
  /// @param baseUri The base URI of the resource
  /// @warning provided JSON must outlive the ResourceIndex object
  void addResource(const nlohmann::json &json, std::set<std::string> refs,
                   std::string baseUri, JSONPointer jsonPointer = JSONPointer(),
                   Draft draft = Draft::UNKNOWN) {
    // TODO: Draft recognition is a bit iffy at this stage, consider doing this
    // seperately in a previous step.
    if (draft == Draft::UNKNOWN) {
      // If we don't know the draft, try to figure it out.
      // This is a simple hack to make early exits work.
      [&]() {
        // Check $schema
        if (json.contains("$schema") && json["$schema"].is_string()) {
          // normalize schema field.
          auto schemaField = normalizeUri(json["$schema"].get<std::string>());
          std::string draft07SchemaUri =
              normalizeUri("http://json-schema.org/draft-07/schema#");
          if (schemaField == draft07SchemaUri) {
            draft = Draft::DRAFT_07;
            return;
          } else {
            std::cerr << "Unknown Schema attempted resolution. Schema field: "
                      << schemaField << "draft07 uri\n"
                      << draft07SchemaUri << std::endl;
          }
        }

        // TODO: Add other detection methods to know in case schema field is
        // missing or doesn't match recognized schemas.
      }();
    }
    if (draft == Draft::UNKNOWN) {
      std::cerr << "Unknown draft version still, assuming Draft07:" << std::endl
                << json.dump(1) << std::endl;
    }
    std::string baseUri_ = baseUri;

    // If this object has an id, we must change the base uri to the id
    // for further correct uri resolution.
    if (json.is_object() && json.contains("$id") && json["$id"].is_string()) {
      std::string id = json["$id"].get<std::string>();
      UriWrapper uri(id);
      UriWrapper base(baseUri);
      UriWrapper fullUri = UriWrapper::applyUri(base, uri);
      const auto fragment = fullUri.getFragment().value_or("");
      if (fragment == "") {
        baseUri_ = fullUri.toFragmentlessString().value();
        jsonPointer = JSONPointer();
        refs.emplace(baseUri_);
        refs.emplace(baseUri_ + "#");
      } else {
        refs.emplace(fullUri.toString().value());
      }
    }

    // Anchors register an object to be accessible from a different
    if (json.is_object() && json.contains("anchor") &&
        json["anchor"].is_string()) {
      std::string anchor = json["anchor"].get<std::string>();
      UriWrapper base(baseUri);
      base.setFragment(anchor, false);
      refs.emplace(base.toString().value());
    }
    std::shared_ptr<Resource> resource = std::make_shared<Resource>(
        std::cref(json), baseUri_, jsonPointer, draft);

    // Since schemas can only be objects or booleans, we can safely assume that
    // no $ref will reference other types, since that would be an invalid JSON
    // schema anyway. That means we can store only the URIs of boolean and
    // object schemas.
    if (json.is_object() || json.is_boolean()) {
      for (auto &ref : refs) {
        std::string ref_ = UriWrapper(ref).toString().value();
        (*this)[ref_] = resource;
      }
    }

    std::vector<UriWrapper> uris;
    for (auto &ref : refs) {
      uris.emplace_back(ref);
    }
    std::vector<JSONPointer> pointers;
    for (auto &uri : uris) {
      pointers.emplace_back(
          JSONPointer::fromURIString(uri.getFragment().value_or("")));
    }

    const auto addItem = [&pointers, &uris, &baseUri_, &jsonPointer, draft,
                          this](const nlohmann::json &json,
                                std::string navKey) {
      std::vector<UriWrapper> uris_ = uris;
      std::vector<JSONPointer> pointers_ = pointers;

      std::set<std::string> refs;
      for (size_t i = 0; i < pointers.size(); i++) {
        auto &pointer = pointers_[i];
        auto &uri = uris_[i];
        pointer.add(navKey);
        const auto fragment = pointer.toFragment();
        uri.setFragment(fragment, true);
        refs.emplace(uri.toString().value_or(""));
      }
      addResource(std::cref(json), refs, baseUri_, jsonPointer / navKey, draft);
    };
    if (json.is_object() || json.is_array()) {
      for (auto &[key, value] : json.items()) {
        addItem(value, key);
      }
    }
  }

  bool contains(const std::string &uri) const {
    return resources.contains(normalizeUri(uri));
  }

  std::shared_ptr<Resource> &getResource(const std::string &uri) {
    std::string uri_ = normalizeUri(uri);
    if (!resources.contains(uri_)) {
      resources[uri_] = nullptr;
    }
    return resources[uri_];
  }

  const std::shared_ptr<Resource> &getResource(const std::string &uri) const {
    std::string uri_ = normalizeUri(uri);
    return resources.at(uri_);
  }

  std::shared_ptr<Resource> &operator[](const std::string &uri) {
    return getResource(uri);
  }

  std::map<std::string, std::shared_ptr<Resource>>::iterator begin() {
    return resources.begin();
  }

  std::map<std::string, std::shared_ptr<Resource>>::iterator end() {
    return resources.end();
  }

  void markForBuild(const std::string &uri) {
    const auto &resource = getResource(uri);
    if (resourcesBuilt.contains(resource)) {
      return;
    }
    resourcesRequiringBuilding.insert(resource);
  }

  void build() {
    while (!resourcesRequiringBuilding.empty()) {
      auto resource = *resourcesRequiringBuilding.begin();
      resource->schema =
          std::move(initializeSchema(resource->json, resource->baseUri,
                                     resource->jsonPointer, resource->draft));
      resourcesBuilt.insert(resource);
      resourcesRequiringBuilding.erase(resource);
      auto dependencies = resource->schema->getDeps();
      for (const auto &dep : dependencies) {
        markForBuild(dep);
      }
    }
  }

  /// @brief Takes all of the names of the resources that have already been
  /// built and overrides them with suffixes to make them all unique.
  void generateUniqueSchemaNames() {
    std::set<std::string> existingRealNames;
    for (auto &builtResource : resourcesBuilt) {
      if (builtResource->schema == nullptr) {
        std::cerr << "Built resource not actually built. WTF???";
        continue;
      }
      auto &schema = builtResource->schema;
      if (!std::holds_alternative<Schema::Stage1>(schema->stage_)) {
        continue;
      }
      auto &stage1 = std::get<Schema::Stage1>(schema->stage_);
      auto name = stage1.getPreferredIdentifier();
      if (existingRealNames.contains(name)) {
        size_t uniqueSuffix = 1;
        auto name_ends_with_number = std::isdigit(name.back());
        size_t maxUniqueSuffix = 69696969;
        while (uniqueSuffix < maxUniqueSuffix) {
          auto newName = name + (name_ends_with_number ? "_" : "") +
                         std::to_string(uniqueSuffix);
          if (!existingRealNames.contains(newName)) {
            name = newName;
            break;
          }
          uniqueSuffix++;
        }
        if (uniqueSuffix == maxUniqueSuffix) {
          throw std::runtime_error(
              "Too many schemas with the same name, likely "
              "a bug in the schema generation code.");
        }
      }
      schema->transitionToStage2(name);
      existingRealNames.emplace(name);
    }
  }

private:
  /// @brief small utility method that is used for binding to a ResourceIndex
  /// and base URI to resolve JSON pointers.
  std::optional<std::reference_wrapper<Schema>>
  resolveSchemaRequest(std::string base, std::string ref) const {
    UriWrapper baseUri(base);
    UriWrapper refUri(ref);
    UriWrapper fullUri = UriWrapper::applyUri(baseUri, refUri);
    auto fullUriStr = fullUri.toString();
    if (!fullUriStr.has_value()) {
      return std::nullopt;
    }
    auto resource = getResource(fullUriStr.value());
    if (resource == nullptr) {
      return std::nullopt;
    }
    return *(resource->schema);
  }

public:
  /// @brief Calls resolveReferences on all schemas, giving a definitive method
  /// that resolves references.
  void resolveReferences() {
    for (auto &resource : resourcesBuilt) {
      if (resource->schema == nullptr) {
        continue;
      }
      resource->schema->resolveReferences(
          std::bind(&ResourceIndex::resolveSchemaRequest, this,
                    resource->baseUri, std::placeholders::_1));
    }
  }

  std::string generateDefinition() const {
    std::string definition;
    definition += "#pragma once\n";
    definition += "#include <variant>\n";
    definition += "#include <vector>\n";
    definition += "#include <string>\n";
    definition += "#include <tuple>\n";
    definition += "#include <map>\n";
    definition += "#include <set>\n";

    for (const auto &resource : resourcesBuilt) {
      if (resource == nullptr || resource->schema == nullptr) {
        continue;
      }
      // TODO: Continue here
      // definition += resource->schema->getDeps();
    }
    return definition;
  }
};

#endif // RESOURCEINDEX_H