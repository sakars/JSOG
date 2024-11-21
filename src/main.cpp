#include "Document.h"
#include "Draft07.h"
#include "DraftInterpreter.h"
#include "DraftRecognisedDocument.h"
#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"
#include "LinkedSchema.h"
#include "SyncedSchema.h"
#include "UnresolvedSchema.h"
#include "UriWrapper.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Note: This is the general class flow for the code generation.
// 1. Document - Open a file and read the JSON content into a nlohmann::json
// 2. DraftRecognisedDocument - Identifies the draft version of the JSON schema
// 3. UnresolvedSchema - Represents the immediate JSON object in the document
// 4. LinkedSchema - Represents a schema with all dependencies resolved and
// known.
// 5. IdentifiableSchema - Represents a schema with it's unique identifier.
// 6. DraftXSchemaInterpreter - Interprets the schema according to the draft
// version and outputs an IndexedSyncedSchema. Last point where both draft and
// the json schema itself are used.
// 7. IndexedSyncedSchema - Represents a schema that is ready to be generated
// but links aren't connected directly yet. This schema completely abstracts
// the draft version.
// 8. SyncedSchema - Represents a schema that is ready to be generated and
// linked to other schemas directly.
// 9. Header and Source files are generated for each SyncedSchema.
int main(int argc, char* argv[]) {
  std::vector<std::string_view> args(argc);
  for (int i = 0; i < argc; ++i) {
    args[i] = argv[i];
  }

  std::filesystem::path outputDirectory = ".";
  std::vector<std::filesystem::path> inputFiles;
  std::vector<std::string> requiredFiles;

  // configure extra options here
  for (int i = 1; i < argc; ++i) {
    if (args[i] == "--help") {
      std::cout << "Usage: " << args[0] << " [options] [file]" << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  --help: Display this help message" << std::endl;
      return 0;
    }
    if (args[i] == "--output-directory" || args[i] == "-o") {
      if (i + 1 < argc) {
        outputDirectory = args[i + 1];
        ++i;
      } else {
        std::cerr << "Error: --output-directory requires an argument."
                  << std::endl;
        return 1;
      }
    }
    if (args[i] == "--require" || args[i] == "-r") {
      if (i + 1 < argc) {
        requiredFiles.emplace_back(args[i + 1]);
        ++i;
      } else {
        std::cerr << "Error: --require requires an argument." << std::endl;
        return 1;
      }
    }
    if (args[i].starts_with('-')) {
      std::cerr << "Error: Unknown option " << args[i] << std::endl;
      return 1;
    }
    inputFiles.push_back(args[i]);
  }
  for (auto& file : inputFiles) {
    file = std::filesystem::absolute(file);
    if (!std::filesystem::exists(file)) {
      std::cerr << "Error: File " << file << " does not exist." << std::endl;
      return 1;
    }
  }

  if (requiredFiles.empty()) {
    for (const auto& file : inputFiles) {
      requiredFiles.push_back("file://" + file.string());
    }
  }
  std::vector<UriWrapper> requiredReferences;
  for (const auto& file : requiredFiles) {
    requiredReferences.emplace_back(file);
  }
  std::vector<Document> documents = loadDocuments(inputFiles);
  std::vector<DraftRecognisedDocument> draftDocs;
  draftDocs.reserve(documents.size());
  for (auto& doc : documents) {
    draftDocs.emplace_back(std::move(doc));
  }
  auto unresolvedSchemas = UnresolvedSchema::generateSetMap(draftDocs);

  UnresolvedSchema::dumpSchemas(unresolvedSchemas);

  auto linkedSchemas =
      resolveDependencies(std::move(unresolvedSchemas), requiredReferences);

  LinkedSchema::dumpSchemas(linkedSchemas);
  const auto issues = LinkedSchema::generateIssuesList(linkedSchemas);
  if (!issues.empty()) {
    std::cerr << "Issues with the schemas:" << std::endl;
    for (const auto& issue : issues) {
      std::cerr << issue << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << "Exiting due to issues with the schemas." << std::endl;
    return 1;
  }

  auto identifiableSchemas =
      IdentifiableSchema::transition(std::move(linkedSchemas));

  IdentifiableSchema::dumpSchemas(identifiableSchemas);

  auto indexedSyncedSchemas = interpretSchemas(identifiableSchemas);

  IndexedSyncedSchema::dumpSchemas(indexedSyncedSchemas);

  auto syncedSchemas =
      SyncedSchema::resolveIndexedSchema(std::move(indexedSyncedSchemas));

  SyncedSchema::dumpSchemas(syncedSchemas);

  const bool combineSourceFiles = true;

  if (combineSourceFiles) {
    std::ofstream source("../locals/schemas/schemas.cpp");
    for (auto& schema : syncedSchemas) {
      std::filesystem::create_directory("../locals/schemas");
      std::ofstream header("../locals/schemas/" + schema->getHeaderFileName());
      header << schema->generateDeclaration().str();
      header.close();
      source << schema->generateDefinition().str();
    }
    source.close();
  } else {
    for (auto& schema : syncedSchemas) {
      std::filesystem::create_directory("../locals/schemas");
      std::ofstream header("../locals/schemas/" + schema->getHeaderFileName());
      header << schema->generateDeclaration().str();
      header.close();
      std::ofstream source("../locals/schemas/" + schema->getSourceFileName());
      source << schema->generateDefinition().str();
      source.close();
    }
  }
}