#ifndef DRAFT07_H
#define DRAFT07_H

#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"
#include "JSONPointer.h"
#include "LinkedSchema.h"
#include "UriWrapper.h"
#include <nlohmann/json.hpp>
#include <set>

/// @brief Gets the dependencies of a Draft 7 schema
std::set<UriWrapper> getDraft07Dependencies(const nlohmann::json&,
                                            const UriWrapper&,
                                            const JSONPointer&);

/// @brief Gets the issues with a Draft 7 schema document
std::vector<std::string> issuesWithDraft07Schema(const nlohmann::json&,
                                                 const UriWrapper&,
                                                 const JSONPointer&);

/// @brief Interprets a Draft 7 schema, converting it to an IndexedSyncedSchema
/// @param schema The schema to interpret
/// @return The interpreted schema
IndexedSyncedSchema
interpretDraft07IdentifiableSchema(const IdentifiableSchema& schema);

#endif // DRAFT07_H
