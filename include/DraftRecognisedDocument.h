#ifndef DRAFTRECOGNISEDDOCUMENT_H
#define DRAFTRECOGNISEDDOCUMENT_H
#include "Document.h"
#include "UriWrapper.h"

/// @brief Represents the recognized schema drafts
/// @details This enum class represents the recognized schema drafts.
/// The schema drafts are used to determine how to interpret the schema.
/// Currently, only Draft 7 is supported.
enum class Draft { DRAFT_07 };

/// @brief Represents a JSON document with a recognized schema draft
struct DraftRecognisedDocument : public Document {
  Draft draft_;

  Draft identifySchemaDraft(const nlohmann::json& json) {
    // Check $schema
    if (json.contains("$schema") && json["$schema"].is_string()) {
      // normalize schema field.
      auto schemaField = UriWrapper(json["$schema"].get<std::string>());
      schemaField.normalize();
      auto draft07SchemaUri =
          UriWrapper("http://json-schema.org/draft-07/schema#");
      schemaField.normalize();
      if (schemaField == draft07SchemaUri) {
        return Draft::DRAFT_07;
      } else {
        std::cerr << "Error: Unsupported schema version: " << schemaField;
      }
    }

    // TODO: Add other detection methods to know in case schema field is
    // missing or doesn't match recognized schemas.
    return Draft::DRAFT_07;
  }

public:
  DraftRecognisedDocument() = delete;
  DraftRecognisedDocument(Document&& document)
      : Document(std::move(document)), draft_(identifySchemaDraft(json_)) {}
  DraftRecognisedDocument(nlohmann::json&& json, UriWrapper&& fileUri)
      : Document(std::move(json), std::move(fileUri)),
        draft_(identifySchemaDraft(json_)) {}

  /// @brief Transforms a vector of documents into a vector of draft-recognized
  /// documents
  /// @param documents
  static inline std::vector<DraftRecognisedDocument>
  performDraftRecognition(std::vector<Document>&& documents) {
    std::vector<DraftRecognisedDocument> recognisedDocuments;
    recognisedDocuments.reserve(documents.size());
    for (auto& document : documents) {
      recognisedDocuments.emplace_back(std::move(document));
    }
    return recognisedDocuments;
  }
};

#endif // DRAFTRECOGNISEDDOCUMENT_H