#ifndef LINKED_SCHEMA_H
#define LINKED_SCHEMA_H

#include "DraftRecognisedDocument.h"
#include "JSONPointer.h"
#include "SchemaResource.h"
#include "SetMap.h"
#include "StringUtils.h"
#include "UriWrapper.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

/// @brief A fully instantiated instance of LinkedSchema has its dependencies
/// set and is ready to be fully constructed.
class LinkedSchema : public SchemaResource {
public:
  std::map<UriWrapper, size_t> dependencies_;

  LinkedSchema(const SchemaResource& schemaResource,
               std::map<UriWrapper, size_t>& dependencies)
      : SchemaResource(schemaResource), dependencies_(dependencies) {}

  LinkedSchema(const SchemaResource& schemaResource,
               std::map<UriWrapper, size_t>&& dependencies)
      : SchemaResource(schemaResource), dependencies_(dependencies) {}

  LinkedSchema(const nlohmann::json& json, const UriWrapper& baseUri,
               const JSONPointer& pointer, Draft draft,
               std::map<UriWrapper, size_t> dependencies)
      : SchemaResource(json, baseUri, pointer, draft),
        dependencies_(dependencies) {}

public:
  LinkedSchema(const LinkedSchema&) = delete;
  LinkedSchema(LinkedSchema&&) = default;

  /// @brief Returns the preferred identifier for the schema.
  /// @return The preferred identifier for the schema.
  /// @warning This function has to return a valid C++ identifier. Use
  /// normalizeString to ensure that.
  std::string getPreferredIdentifier() const {
    if (json_.get().contains("title")) {
      auto title = json_.get()["title"].get<std::string>();
      if (title != "") {
        return sanitizeString(title);
      }
    }
    return "Schema";
  }

  /// @brief Returns the preferred identifier for the schema.
  /// @param schemas
  /// @param outputDirectory
  static void dumpSchemas(std::vector<std::unique_ptr<LinkedSchema>>& schemas,
                          std::filesystem::path outputDirectory = ".") {

    auto linkedDump = nlohmann::json::object();
    for (const auto& lSchema : schemas) {
      auto& uriDump = linkedDump[lSchema.get()->baseUri_.toString().value()];
      auto& ptrDump = uriDump[lSchema.get()->pointer_.toFragment()];
      ptrDump["deps"] = nlohmann::json::object();
      for (const auto& [uri, idx] : lSchema.get()->dependencies_) {
        const auto idxSchema = schemas[idx].get();
        ptrDump["deps"][uri.toString().value()] =
            idxSchema->baseUri_.withPointer(idxSchema->pointer_)
                .toString()
                .value();
      }
    }

    std::ofstream linkedDumpFile(outputDirectory / "linked.dump.json");
    linkedDumpFile << linkedDump.dump(2);
    linkedDumpFile.close();
  }

  /// @brief Generates a list of issues with the schemas
  /// @param schemas
  static std::vector<std::string>
  generateIssuesList(const std::vector<std::unique_ptr<LinkedSchema>>& schemas);
};

/// @brief A map to hold the linker functions for each schema version
extern std::map<Draft,
                std::set<UriWrapper> (*)(const nlohmann::json&,
                                         const UriWrapper&, const JSONPointer&)>
    linkers;

/// @brief A map to hold the issue checker functions for each schema version
extern std::map<Draft, std::vector<std::string> (*)(const nlohmann::json&,
                                                    const UriWrapper&,
                                                    const JSONPointer&)>
    issueCheckers;

/// @brief Deconstructs a set map of unresolved schemas into a vector of
/// schemas and a map of idendifiers to indices to the schemas via vector
std::tuple<std::vector<std::unique_ptr<SchemaResource>>,
           std::map<UriWrapper, size_t>>
deconstructSchemaResourceMap(SetMap<UriWrapper, SchemaResource>&& setMap);

/// @brief Resolves the dependencies of a set map of schemas
/// @param setMap The set map of schemas
/// @param requiredReferences
std::vector<std::unique_ptr<LinkedSchema>>
resolveDependencies(SetMap<UriWrapper, SchemaResource>&& setMap,
                    const std::set<UriWrapper>& requiredReferences);

#endif // LINKED_SCHEMA_H