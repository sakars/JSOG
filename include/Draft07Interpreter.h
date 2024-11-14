#ifndef DRAFT07INTERPRETER_H
#define DRAFT07INTERPRETER_H

#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"

IndexedSyncedSchema
interpretDraft07IdentifiableSchema(const IdentifiableSchema&);

std::vector<std::string> issuesWithDraft07Schema(const IdentifiableSchema&);

#endif // DRAFT07INTERPRETER_H
