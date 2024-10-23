#ifndef SCHEMAINITIALIZER_H
#define SCHEMAINITIALIZER_H

#include "Schema.h"
#include <memory>
// NOTE: Don't include specific Drafts in the header. That might lead to
// circular refs.

std::unique_ptr<Schema> initializeSchema(const nlohmann::json &json,
                                         std::string baseUri,
                                         Draft draft = Draft::UNKNOWN);

#endif // SCHEMAINITIALIZER_H