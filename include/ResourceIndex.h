#ifndef RESOURCEINDEX_H
#define RESOURCEINDEX_H

#include "JSONPointer.h"
#include "Schema.h"
#include "SchemaInitializer.h"
#include "UriWrapper.h"
#include "DraftRecogniser.h"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <optional>
#include <format>
#include <vector>
#include <fstream>

/// @brief The central index of known json elements.
/// @details This index maps out json instances for schemmas and constructs
/// schema instances where applicable.
class ResourceIndex
{
public:
  /// @brief Represents a single json shallow object.
  /// @details A json shallow object means that this resource only really cares
  /// about the value of this current json object, not it's children (if
  /// applicable) If this object is determined to be an evaluatable schema, the
  /// Schema pointer will become non-nullptr.
  struct Resource
  {
    /// @brief A reference to the shallow json object it manages.
    const nlohmann::json &json;
    /// @brief A string representing the base of the URI of the resource
    const std::string baseUri;
    /// @brief A JSON Pointer to the resource relative to the base URI
    const JSONPointer jsonPointer;

    std::set<UriWrapper> refs;

    /// @brief Returns the preferred identifier for the schema.
    /// @return The preferred identifier for the schema.
    /// @warning This function has to return a valid C++ identifier. Use normalizeString to ensure that.
    std::string getPreferredIdentifier() const
    {
      if (json.contains("title"))
      {
        auto title = json["title"].get<std::string>();
        if (title != "")
        {
          return normalizeString(title);
        }
      }
      return "Schema";
    }

    // /// @brief Which draft this schema is most likely adhering to
    // Draft draft = Draft::DRAFT_07;

    // /// @brief An instance of a schema object that is generated from the  json
    // /// file.
    // /// @note This schema object may be empty if the index hasn't determined
    // /// that this is indeed a schema that should have it's object generated.
    // std::unique_ptr<Schema> schema;
  };

private:
  /// @brief Main map of resources that holds URI keys that map to the shallow
  /// json reference resource.
  std::map<UriWrapper, std::shared_ptr<Resource>> resources;
  /// @brief This set of pointers mark which resources should be built, but yet
  /// have not been.
  std::set<std::shared_ptr<Resource>> resourcesRequiringBuilding;
  /// @brief This set of pointers mark which resources have been built.
  std::set<std::shared_ptr<Resource>> resourcesBuilt;
  std::vector<std::unique_ptr<Schema>> schemas;
  std::map<UriWrapper, std::reference_wrapper<Schema>> schemaMap;

  /// @brief Normalizes a uri string to ensure best possible map key access.
  /// @details Non-normalized uris may be equivalent, but not be recognized as
  /// equivalent in a map. This method normalizes strings to a canonical form,
  /// so equivalent URIs are resolved to the same map.
  static UriWrapper normalizeUri(const UriWrapper &uri)
  {
    UriWrapper uri_(uri);
    std::string fragment = uri_.getFragment().value_or("");
    uri_.setFragment(fragment, false);
    uri_.normalize();
    return uri_;
  }

public:
  /// @brief Adds a resource to the index
  /// @param json A JSON object reference to the resource
  /// @param refs A set of known references to the resource
  /// @param baseUri The base URI of the resource
  /// @warning provided JSON must outlive the ResourceIndex object
  void addResource(const nlohmann::json &json, std::set<std::string> refs,
                   std::string baseUri, JSONPointer jsonPointer = JSONPointer(),
                   std::optional<Draft> draft = std::nullopt)
  {
    // TODO: Draft recognition is a bit iffy at this stage, consider doing this
    // seperately in a previous step.
    if (!draft.has_value())
    {
      draft = identifySchemaDraft(json);
    }
    if (!draft.has_value())
    {
      std::cerr << "Unknown draft version, assuming Draft07:" << std::endl
                << json.dump(1) << std::endl;
      draft = Draft::DRAFT_07;
    }
    std::string baseUri_ = baseUri;

    // If this object has an id, we must change the base uri to the id
    // for further correct uri resolution.
    if (json.is_object() && json.contains("$id") && json["$id"].is_string())
    {
      std::string id = json["$id"].get<std::string>();
      UriWrapper uri(id);
      UriWrapper base(baseUri);
      UriWrapper fullUri = UriWrapper::applyUri(base, uri);
      const auto fragment = fullUri.getFragment().value_or("");
      if (fragment == "")
      {
        baseUri_ = fullUri.toFragmentlessString().value();
        jsonPointer = JSONPointer();
        refs.emplace(baseUri_);
        refs.emplace(baseUri_ + "#");
      }
      else
      {
        refs.emplace(fullUri.toString().value());
      }
    }

    // Anchors register an object to be accessible from a different
    if (json.is_object() && json.contains("anchor") &&
        json["anchor"].is_string())
    {
      std::string anchor = json["anchor"].get<std::string>();
      UriWrapper base(baseUri);
      base.setFragment(anchor, false);
      refs.emplace(base.toString().value());
    }

    // Since schemas can only be objects or booleans, we can safely assume that
    // no $ref will reference other types, since that would be an invalid JSON
    // schema anyway. That means we can store only the URIs of boolean and
    // object schemas.
    if (json.is_object() || json.is_boolean())
    {
      std::set<UriWrapper> refs_;
      for (auto &ref : refs)
      {
        refs_.emplace(ref);
      }
      auto resourcePtr = std::make_shared<Resource>(Resource(
          std::cref(json), baseUri_, jsonPointer, refs_));
      for (auto &ref : refs)
      {
        std::string ref_ = UriWrapper(ref).toString().value();
        setResource(ref_, resourcePtr);
      }
    }

    std::vector<UriWrapper> uris;
    for (auto &ref : refs)
    {
      uris.emplace_back(ref);
    }
    std::vector<JSONPointer> pointers;
    for (auto &uri : uris)
    {
      pointers.emplace_back(
          JSONPointer::fromURIString(uri.getFragment().value_or("")));
    }

    const auto addItem = [&pointers, &uris, &baseUri_, &jsonPointer, draft,
                          this](const nlohmann::json &json,
                                std::string navKey)
    {
      std::vector<UriWrapper> uris_ = uris;
      std::vector<JSONPointer> pointers_ = pointers;

      std::set<std::string> refs;
      for (size_t i = 0; i < pointers.size(); i++)
      {
        auto &pointer = pointers_[i];
        auto &uri = uris_[i];
        pointer.add(navKey);
        const auto fragment = pointer.toFragment();
        uri.setFragment(fragment, true);
        refs.emplace(uri.toString().value_or(""));
      }
      addResource(std::cref(json), refs, baseUri_, jsonPointer / navKey, draft);
    };
    if (json.is_object() || json.is_array())
    {
      for (auto &[key, value] : json.items())
      {
        addItem(value, key);
      }
    }
  }

  bool contains(const std::string &uri) const
  {
    return resources.contains(normalizeUri(uri));
  }

  std::shared_ptr<Resource> &getResource(const UriWrapper &uri)
  {
    auto uri_ = normalizeUri(uri);
    if (!resources.contains(uri_))
    {
      throw std::runtime_error(
          std::format("Resource {} not found in index", uri_.toString().value_or("INVALID_URI")));
    }
    return resources[uri_];
  }

  const std::shared_ptr<Resource> &getResource(const UriWrapper &uri) const
  {
    UriWrapper uri_ = normalizeUri(uri);
    return resources.at(uri_);
  }

  void setResource(const std::string &uri, std::shared_ptr<Resource> resource)
  {
    resources[normalizeUri(uri)] = resource;
  }

  std::shared_ptr<Resource> &operator[](const std::string &uri)
  {
    return getResource(uri);
  }

  std::map<UriWrapper, std::shared_ptr<Resource>>::iterator begin()
  {
    return resources.begin();
  }

  std::map<UriWrapper, std::shared_ptr<Resource>>::iterator end()
  {
    return resources.end();
  }

  void markForBuild(const std::string &uri)
  {
    const auto &resource = getResource(uri);
    if (resourcesBuilt.contains(resource))
    {
      return;
    }
    resourcesRequiringBuilding.insert(resource);
  }

  void build()
  {
    while (!resourcesRequiringBuilding.empty())
    {
      auto resource = *resourcesRequiringBuilding.begin();
      auto schema = initializeSchema(resource->json, resource->baseUri,
                                     resource->jsonPointer, identifySchemaDraft(resource->json));
      for (auto &ref : resource->refs)
      {
        schemaMap.insert({ref, std::ref(*schema)});
      }
      resourcesRequiringBuilding.erase(resource);
      resourcesBuilt.insert(resource);
      auto dependencies = schema->getDeps();
      schemas.emplace_back(std::move(schema));

      for (const auto &dep : dependencies)
      {
        markForBuild(dep);
      }
    }
  }

  /// @brief Takes all of the names of the resources that have already been
  /// built and overrides them with suffixes to make them all unique.
  void generateUniqueSchemaNames()
  {
    std::set<std::string> existingRealNames;
    for (auto &schema : schemas)
    {
      if (!std::holds_alternative<Schema::Stage1>(schema->stage_))
      {
        continue;
      }
      auto &stage1 = std::get<Schema::Stage1>(schema->stage_);
      auto name = stage1.getPreferredIdentifier();
      if (existingRealNames.contains(name))
      {
        size_t uniqueSuffix = 1;
        auto name_ends_with_number = std::isdigit(name.back());
        size_t maxUniqueSuffix = 69696969;
        while (uniqueSuffix < maxUniqueSuffix)
        {
          auto newName = name + (name_ends_with_number ? "_" : "") +
                         std::to_string(uniqueSuffix);
          if (!existingRealNames.contains(newName))
          {
            name = newName;
            break;
          }
          uniqueSuffix++;
        }
        if (uniqueSuffix == maxUniqueSuffix)
        {
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
  resolveSchemaRequest(std::string base, std::string ref) const
  {
    UriWrapper baseUri(base);
    UriWrapper refUri(ref);
    UriWrapper fullUri = UriWrapper::applyUri(baseUri, refUri);
    auto fullUriStr = fullUri.toString();
    if (!fullUriStr.has_value())
    {
      return std::nullopt;
    }
    if (!schemaMap.contains(fullUri))
    {
      return std::nullopt;
    }
    return schemaMap.at(fullUri);
  }

public:
  /// @brief Calls resolveReferences on all schemas, giving a definitive method
  /// that resolves references.
  void resolveReferences()
  {
    for (auto &schema : schemas)
    {
      schema->resolveReferences(
          std::bind(&ResourceIndex::resolveSchemaRequest, this,
                    schema->getBaseUri(), std::placeholders::_1));
    }
  }

  /// @brief Extracts all schemas from the index and resets the index to a clean state.
  /// @return A vector of unique pointers to the schemas that were extracted.
  std::vector<std::unique_ptr<Schema>> extractSchemas()
  {
    auto schemas = std::move(this->schemas);
    this->schemas.clear();
    resources.clear();
    resourcesBuilt.clear();
    resourcesRequiringBuilding.clear();
    schemaMap.clear();
    return schemas;
  }

  // TODO: Shift the commented out code to a separate entity.
  // Resource Index shouldn't be responsible for generating code,
  // but rather for only organizing resources and linking schemas.

  // /// @brief Returns a list of declarations for a given resource.
  // std::set<std::shared_ptr<Resource>> getRequiredResources(const Resource &resource) const
  // {
  //   std::set<std::shared_ptr<Resource>> dependantResources;
  //   if (resource.schema == nullptr)
  //   {
  //     return dependantResources;
  //   }
  //   if (std::holds_alternative<Schema::Stage1>(resource.schema->stage_))
  //   {
  //     return dependantResources;
  //   }
  //   const auto deps = resource.schema->getDeps();
  //   for (const auto &dep : deps)
  //   {
  //     dependantResources.insert(getResource(dep));
  //   }
  //   return dependantResources;
  // }

  // std::string generateRequiredIncludes(const Resource &resource) const
  // {
  //   std::string includes;
  //   const auto requiredResources = getRequiredResources(resource);
  //   for (const auto &reqResource : requiredResources)
  //   {
  //     const auto reqIdentifier = reqResource->schema->getIdentifier().value();
  //     includes += std::format("#include \"{}.h\"\n", reqIdentifier);
  //   }
  //   includes += "#include <nlohmann/json.hpp>\n";
  //   includes += "#include <variant>\n";
  //   includes += "#include <optional>\n";
  //   includes += "#include <vector>\n";
  //   includes += "#include <string>\n";
  //   includes += "#include <tuple>\n";
  //   includes += "#include <map>\n";
  //   includes += "#include <set>\n";
  //   includes += "\n";
  //   return includes;
  // }

  // std::string generateDeclaration(const Resource &resource) const
  // {
  //   std::string declaration;
  //   const auto identifier = resource.schema->getIdentifier().value();
  //   declaration += generateRequiredIncludes(resource);
  //   declaration += std::format("#ifndef JSOG_{}_H\n", identifier);
  //   declaration += std::format("#define JSOG_{}_H\n", identifier);
  //   declaration += resource.schema->generateStructs();
  //   declaration += "namespace " + identifier + "{\n";
  //   declaration += "std::optional<" + resource.schema->getTypeName() + "> create(const nlohmann::json &json);\n";
  //   declaration += "} // namespace " + identifier + "\n\n";
  //   declaration += "#endif // JSOG_" + identifier + "_H\n";
  //   return declaration;
  // }

  // std::string generateDefinition(const Resource &resource) const
  // {
  //   std::string identifier = resource.schema->getIdentifier().value();
  //   std::string definition;

  //   definition += generateRequiredIncludes(resource);
  //   definition += "#include \"" + identifier + ".h\"\n";

  //   definition += resource.schema->generateDefinition();
  //   definition += "\n";

  //   return definition;
  // }

  // void generateResources(std::filesystem::path srcDir, std::optional<std::filesystem::path> includeDir = std::nullopt) const
  // {
  //   includeDir = includeDir.value_or(srcDir);
  //   if (!std::filesystem::exists(srcDir))
  //   {
  //     std::filesystem::create_directories(srcDir);
  //   }
  //   if (!std::filesystem::exists(*includeDir))
  //   {
  //     std::filesystem::create_directories(*includeDir);
  //   }
  //   for (const auto &resource : resourcesBuilt)
  //   {
  //     if (resource->schema == nullptr)
  //     {
  //       continue;
  //     }
  //     auto identifier = resource->schema->getIdentifier().value();
  //     std::ofstream out(*includeDir / (identifier + ".h"));
  //     out << generateDeclaration(*resource);
  //     out.close();
  //     out.open(srcDir / (identifier + ".cpp"));
  //     out << generateDefinition(*resource);
  //     out.close();
  //   }
  // }
};

#endif // RESOURCEINDEX_H