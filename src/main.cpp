#include "CodeProperties.h"
#include "Document.h"
#include "Draft07.h"
#include "DraftInterpreter.h"
#include "DraftRecognisedDocument.h"
#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"
#include "LinkedSchema.h"
#include "SchemaResource.h"
#include "SyncedSchema.h"
#include "UriWrapper.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Note: This is the general class flow for the code generation.
// 1. Document - Open a file and read the JSON content into a nlohmann::json
// 2. DraftRecognisedDocument - Identifies the draft version of the JSON schema
// 3. SchemaResource - Represents the immediate JSON object in the document
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
  std::map<UriWrapper, std::string> preferredIdentifiers;
  CodeProperties codeProperties;
  bool dumpSchemas = false;
  bool combineSourceFiles = true;

  // configure extra options here
  for (int i = 1; i < argc; ++i) {
    if (args[i] == "--help" || args[i] == "-h") {
      std::cout << "Usage: " << args[0] << " [options] [files*]" << std::endl;
      std::cout << "Options:" << std::endl;
      std::cout << "  --help: Display this help message" << std::endl;
      std::cout << "  --dump-schemas, -d: Dump schemas at each stage"
                << std::endl;
      std::cout << "  --output-directory, -o: Specify the output directory"
                << std::endl;
      std::cout << "  --require, -r: Specify a URI to a schema that must have "
                   "it's code generated"
                << std::endl;
      std::cout << "  --preferred-identifier, -p:" << std::endl
                << "    Usage: --preferred-identifier <uri> <identifier>"
                << "    Specify a preferred identifier for a specific schema"
                << "    Note: The identifier must be unique and a valid C++ "
                   "identifier"
                << std::endl;
      std::cout << "  --namespace, -n: Specify the global namespace"
                << std::endl;
      std::cout << "  --no-namespace, -nn: Do not use a global namespace"
                << std::endl;
      std::cout << "  --define-prefix, -dp: Specify the prefix for #define "
                   "guards"
                << std::endl;
      std::cout << "  --use-pragma, -up: Use #pragma once for header guards"
                << std::endl;
      std::cout << "  --use-ifndef, -ui: Use #ifndef for header guards"
                << std::endl;
      std::cout << "  --indent, -i: Specify the indentation string"
                << std::endl;
      std::cout << "  --no-combine, -nc: Do not combine source files into one"
                << std::endl;

      return 0;
    }
    if (args[i] == "--dump-schemas" || args[i] == "-d") {
      dumpSchemas = true;
      continue;
    }
    if (args[i] == "--output-directory" || args[i] == "-o") {
      if (i + 1 < argc) {
        outputDirectory = args[i + 1];
        if (std::filesystem::exists(outputDirectory) &&
            !std::filesystem::is_directory(outputDirectory)) {
          std::cerr << "Error: --output-directory must be a directory."
                    << std::endl;
          return 1;
        }
        if (!std::filesystem::exists(outputDirectory)) {
          std::filesystem::create_directory(outputDirectory);
        }
        ++i;
        continue;
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
        continue;
      } else {
        std::cerr << "Error: --require requires an argument." << std::endl;
        return 1;
      }
    }
    if (args[i] == "--preferred-identifier" || args[i] == "-p") {
      if (i + 2 < argc) {
        preferredIdentifiers.emplace(UriWrapper(std::string(args[i + 1])),
                                     args[i + 2]);
        i += 2;
        continue;
      } else {
        std::cerr << "Error: --preferred-identifier requires two arguments."
                  << std::endl;
        return 1;
      }
    }
    if (args[i] == "--namespace" || args[i] == "-n") {
      if (i + 1 < argc) {
        codeProperties.globalNamespace_ = args[i + 1];
        i += 1;
        continue;
      } else {
        std::cerr << "Error: --namespace requires an argument." << std::endl;
        return 1;
      }
    }
    if (args[i] == "--no-namespace" || args[i] == "-nn") {
      codeProperties.globalNamespace_ = std::nullopt;
      continue;
    }
    if (args[i] == "--define-prefix" || args[i] == "-dp") {
      if (i + 1 < argc) {
        codeProperties.definePrefix_ = args[i + 1];
        i += 1;
        continue;
      } else {
        std::cerr << "Error: --define-prefix requires an argument."
                  << std::endl;
        return 1;
      }
    }
    if (args[i] == "--use-pragma" || args[i] == "-up") {
      codeProperties.headerGuardType_ = CodeProperties::HeaderGuard::Pragma;
      continue;
    }
    if (args[i] == "--use-ifndef" || args[i] == "-ui") {
      codeProperties.headerGuardType_ = CodeProperties::HeaderGuard::Ifndef;
      continue;
    }
    if (args[i] == "--indent" || args[i] == "-i") {
      if (i + 1 < argc) {
        codeProperties.indent_ = args[i + 1];
        i += 1;
        continue;
      } else {
        std::cerr << "Error: --indent requires an argument." << std::endl;
        return 1;
      }
    }
    if (args[i] == "--no-combine" || args[i] == "-nc") {
      combineSourceFiles = false;
      continue;
    }
    if (args[i].starts_with('-')) {
      std::cerr << "Error: Unknown option " << args[i] << std::endl;
      return 1;
    }
    inputFiles.push_back(args[i]);
  }

  if (inputFiles.empty()) {
    std::cerr << "Error: No input files specified." << std::endl;
    return 1;
  }

  for (auto& file : inputFiles) {
    file = std::filesystem::absolute(file);
    if (!std::filesystem::exists(file)) {
      std::cerr << "Error: File not found: " << file << std::endl;
      return 1;
    }
  }

  if (requiredFiles.empty()) {
    for (const auto& file : inputFiles) {
      requiredFiles.push_back("file://" + file.string());
    }
  }
  std::set<UriWrapper> requiredReferences;
  for (const auto& file : requiredFiles) {
    requiredReferences.emplace(file);
  }

  // Pipeline
  std::vector<Document> documents = loadDocuments(inputFiles);
  std::vector<DraftRecognisedDocument> draftDocs =
      DraftRecognisedDocument::performDraftRecognition(std::move(documents));
  auto schemaResources = SchemaResource::generateSetMap(draftDocs);

  if (dumpSchemas)
    SchemaResource::dumpSchemas(schemaResources, outputDirectory);

  auto linkedSchemas =
      resolveDependencies(std::move(schemaResources), requiredReferences);

  if (dumpSchemas)
    LinkedSchema::dumpSchemas(linkedSchemas, outputDirectory);
  const auto issues = LinkedSchema::generateIssuesList(linkedSchemas);
  if (!issues.empty()) {
    std::cerr << "Error: Issues with the schemas:" << std::endl;
    for (const auto& issue : issues) {
      std::cerr << issue << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << "Exiting due to issues with the schemas." << std::endl;
    return 1;
  }

  auto identifiableSchemas = IdentifiableSchema::transition(
      std::move(linkedSchemas), preferredIdentifiers);

  if (dumpSchemas)
    IdentifiableSchema::dumpSchemas(identifiableSchemas, outputDirectory);

  auto indexedSyncedSchemas = interpretSchemas(identifiableSchemas);

  if (dumpSchemas)
    IndexedSyncedSchema::dumpSchemas(indexedSyncedSchemas, outputDirectory);

  auto syncedSchemas = SyncedSchema::resolveIndexedSchema(
      std::move(indexedSyncedSchemas), codeProperties);

  SyncedSchema::dumpSchemas(syncedSchemas, outputDirectory);

  if (combineSourceFiles) {
    std::ofstream source(outputDirectory / "schemas.cpp");
    if (!source.is_open()) {
      std::cerr << "Error: Could not open schemas.cpp for writing."
                << std::endl;
      return 1;
    }
    for (auto& schema : syncedSchemas) {
      std::ofstream header(outputDirectory / schema->getHeaderFileName());
      if (!header.is_open()) {
        std::cerr << "Error: Could not open " << schema->getHeaderFileName()
                  << " for writing." << std::endl;
        return 1;
      }
      header << schema->generateDeclaration().str();
      header.close();
      source << schema->generateDefinition().str();
    }
    source.close();
  } else {
    for (auto& schema : syncedSchemas) {
      std::ofstream header(outputDirectory / schema->getHeaderFileName());
      if (!header.is_open()) {
        std::cerr << "Error: Could not open " << schema->getHeaderFileName()
                  << " for writing." << std::endl;
        return 1;
      }
      try {
        header << schema->generateDeclaration().str();
      } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
      }
      header.close();
      std::ofstream source(outputDirectory / schema->getSourceFileName());
      if (!source.is_open()) {
        std::cerr << "Error: Could not open " << schema->getSourceFileName()
                  << " for writing." << std::endl;
        return 1;
      }
      try {
        source << schema->generateDefinition().str();
      } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
      }
      source.close();
    }
  }
}