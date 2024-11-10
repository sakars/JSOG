#include "DraftInterpreter.h"
#include "Draft07Interpreter.h"
std::vector<IndexedSyncedSchema>
interpretSchemas(const std::vector<IdentifiableSchema>& identifiableSchemas) {
  std::vector<IndexedSyncedSchema> indexedSyncedSchemas;
  for (const auto& schema : identifiableSchemas) {
    switch (schema.draft_) {
    case Draft::DRAFT_07:
      indexedSyncedSchemas.emplace_back(
          interpretDraft07IdentifiableSchema(schema));
      break;
    default:
      throw std::runtime_error("Unrecognized schema draft");
    }
  }
  return indexedSyncedSchemas;
}
