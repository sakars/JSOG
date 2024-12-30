#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"

/// @brief A map to hold the interpreter functions for each schema version
extern std::map<Draft, IndexedSyncedSchema (*)(const IdentifiableSchema&)>
    interpreters;

/// @brief Interprets schemas according to their draft version
/// @param identifiableSchemas The schemas to interpret
/// @return The interpreted schemas
std::vector<IndexedSyncedSchema>
interpretSchemas(const std::vector<IdentifiableSchema>& identifiableSchemas);

/// @brief Creates a data dump of the indexed synced schemas
/// @param indexedSyncedSchemas
void dumpIndexedSyncedSchemas(
    const std::vector<IndexedSyncedSchema>& indexedSyncedSchemas);