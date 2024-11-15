#include "DraftInterpreter.h"
#include "Draft07.h"

std::map<Draft, IndexedSyncedSchema (*)(const IdentifiableSchema&)>
    interpreters{
        {Draft::DRAFT_07, interpretDraft07IdentifiableSchema},
    };

std::vector<IndexedSyncedSchema>
interpretSchemas(const std::vector<IdentifiableSchema>& identifiableSchemas) {
  std::vector<IndexedSyncedSchema> indexedSyncedSchemas;
  for (const auto& schema : identifiableSchemas) {
    if (interpreters.contains(schema.draft_)) {
      indexedSyncedSchemas.emplace_back(interpreters.at(schema.draft_)(schema));
    } else {
      throw std::runtime_error("Unrecognized schema draft");
    }
  }
  return indexedSyncedSchemas;
}
