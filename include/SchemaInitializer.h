#ifndef SCHEMAINITIALIZER_H
#define SCHEMAINITIALIZER_H

#include "Schema.h"
#include "DraftRecogniser.h"
#include <memory>
// NOTE: Don't include specific Drafts in the header. That might lead to
// circular refs.

std::unique_ptr<Schema> initializeSchema(const nlohmann::json &json,
                                         std::string baseUri,
                                         JSONPointer jsonPointer,
                                         std::optional<Draft> draft = std::nullopt);

#endif // SCHEMAINITIALIZER_H