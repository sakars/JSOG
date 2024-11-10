#ifndef DRAFTRECOGNISEDDOCUMENT_H
#define DRAFTRECOGNISEDDOCUMENT_H
#include "Document.h"
#include "UriWrapper.h"

enum class Draft { DRAFT_07 };

struct DraftRecognisedDocument {
  nlohmann::json json_;
  UriWrapper fileUri_;
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
        std::cerr << "Unknown Schema attempted resolution. Schema field: "
                  << schemaField << "draft07 uri\n"
                  << draft07SchemaUri << std::endl;
      }
    }

    // TODO: Add other detection methods to know in case schema field is
    // missing or doesn't match recognized schemas.
    return Draft::DRAFT_07;
  }

public:
  DraftRecognisedDocument() = delete;
  DraftRecognisedDocument(Document&& document)
      : json_(std::move(document.json_)),
        fileUri_(std::move(document.fileUri_)),
        draft_(identifySchemaDraft(json_)) {}
  DraftRecognisedDocument(const nlohmann::json& json, UriWrapper fileUri)
      : json_(json), fileUri_(fileUri), draft_(identifySchemaDraft(json_)) {}
};

inline std::vector<DraftRecognisedDocument>
performDraftRecognition(std::vector<Document>&& documents) {
  std::vector<DraftRecognisedDocument> recognisedDocuments;
  recognisedDocuments.reserve(documents.size());
  for (auto& document : documents) {
    recognisedDocuments.emplace_back(std::move(document));
  }
  return recognisedDocuments;
}

#endif // DRAFTRECOGNISEDDOCUMENT_H