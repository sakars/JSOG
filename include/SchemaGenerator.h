#ifndef SCHEMAGENERATOR_H
#define SCHEMAGENERATOR_H
#include <filesystem>
#include "Document.h"
#include "ResourceIndex.h"
#include "UriWrapper.h"

/// @brief The main class that oversees the generation of schemas from raw documents to output C++ files.
class SchemaGenerator
{
  // initial data - cleared after generation
  std::vector<UriWrapper> schemasToBuild;
  std::vector<Document> documents;

public:
  SchemaGenerator() = default;
  void addDocument(Document &&doc)
  {
    documents.emplace_back(std::move(doc));
  }
  void markSchemaForBuild(const std::string &uri)
  {
    schemasToBuild.emplace_back(uri);
  }

private:
  void addDocumentsToIndex(ResourceIndex &index)
  {
    for (auto &doc : documents)
    {
      index.addResource(doc.json, {doc.fileUri.toString().value_or("")}, doc.fileUri.toString().value_or(""));
    }
  }

  void buildSchemas(ResourceIndex &index)
  {
    for (auto &schema : schemasToBuild)
    {
      index.markForBuild(schema.toString().value_or(""));
    }
    index.build();
  }

  static void resolveSchemas(ResourceIndex &index)
  {
    index.generateUniqueSchemaNames();
    index.resolveReferences();
  }

  static void extractSchemasFromIndex(ResourceIndex &index, std::vector<std::unique_ptr<Schema>> &schemas)
  {
    schemas = std::move(index.extractSchemas());
  }

public:
  std::map<std::filesystem::path, std::string> generateSchemas(std::filesystem::path srcDir, std::optional<std::filesystem::path> includeDir = std::nullopt)
  {
    // Setup
    ResourceIndex index;
    std::vector<std::unique_ptr<Schema>> schemas;

    // Process
    addDocumentsToIndex(index);
    buildSchemas(index);
    resolveSchemas(index);
    extractSchemasFromIndex(index, schemas);

    // Cleanup
    schemasToBuild.clear();
    documents.clear();
    return {};
  }
};

#endif // SCHEMAGENERATOR_H