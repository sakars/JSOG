#ifndef DRAFT07_H
#define DRAFT07_H

#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"
#include "JSONPointer.h"
#include "LinkedSchema.h"
#include "UriWrapper.h"
#include <nlohmann/json.hpp>
#include <set>

std::set<UriWrapper> getDraft07Dependencies(const nlohmann::json&,
                                            const UriWrapper&,
                                            const JSONPointer&);

std::vector<std::string> issuesWithDraft07Schema(const LinkedSchema&);

IndexedSyncedSchema
interpretDraft07IdentifiableSchema(const IdentifiableSchema&);

#endif // DRAFT07_H
