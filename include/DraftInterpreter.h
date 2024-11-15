#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"

extern std::map<Draft, IndexedSyncedSchema (*)(const IdentifiableSchema&)>
    interpreters;

std::vector<IndexedSyncedSchema>
interpretSchemas(const std::vector<IdentifiableSchema>& identifiableSchemas);

void dumpIndexedSyncedSchemas(
    const std::vector<IndexedSyncedSchema>& indexedSyncedSchemas);