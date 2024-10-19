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

class ResourceIndex {
public:
  struct Resource {
    const nlohmann::json &json;
    const std::string baseUri;
    std::unique_ptr<Schema> schema;
  };

private:
  std::map<std::string, std::shared_ptr<Resource>> resources;
  std::set<std::shared_ptr<Resource>> resourcesRequiringBuilding;
  std::set<std::shared_ptr<Resource>> resourcesBuilt;

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
                   std::string baseUri) {
    std::string baseUri_ = baseUri;

    if (json.is_object() && json.contains("$id") && json["$id"].is_string()) {
      std::string id = json["$id"].get<std::string>();
      UriWrapper uri(id);
      UriWrapper base(baseUri);
      UriWrapper fullUri = UriWrapper::applyUri(base, uri);
      const auto fragment = fullUri.getFragment().value_or("");
      if (fragment == "") {
        baseUri_ = fullUri.toFragmentlessString().value();
        refs.emplace(baseUri_);
        refs.emplace(baseUri_ + "#");
      } else {
        refs.emplace(fullUri.toString().value());
      }
    }

    if (json.is_object() && json.contains("anchor") &&
        json["anchor"].is_string()) {
      std::string anchor = json["anchor"].get<std::string>();
      UriWrapper base(baseUri);
      base.setFragment(anchor, false);
      refs.emplace(base.toString().value());
    }
    std::shared_ptr<Resource> resource =
        std::make_shared<Resource>(std::cref(json), baseUri_);

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

    const auto addItem = [&pointers, &uris, &baseUri_, this](
                             const nlohmann::json &json, std::string navKey) {
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
      addResource(std::cref(json), refs, baseUri_);
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
          std::move(initializeSchema(resource->json, resource->baseUri));
      resourcesBuilt.insert(resource);
      resourcesRequiringBuilding.erase(resource);
      auto dependencies = resource->schema->getDeps();
      for (const auto &dep : dependencies) {
        markForBuild(dep);
      }
    }
  }
};

#endif // RESOURCEINDEX_H