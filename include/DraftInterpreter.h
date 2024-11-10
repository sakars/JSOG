#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"

std::vector<IndexedSyncedSchema>
interpretSchemas(const std::vector<IdentifiableSchema>& identifiableSchemas);

void dumpIndexedSyncedSchemas(
    const std::vector<IndexedSyncedSchema>& indexedSyncedSchemas);