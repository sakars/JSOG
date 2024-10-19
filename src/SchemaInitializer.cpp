#include "SchemaInitializer.h"
#include "Draft07.h"

std::unique_ptr<Schema> initializeSchema(const nlohmann::json &json,
                                         std::string baseUri) {
  // TODO: Implement schema version detection. For MVP only Draft07 will do.
  return std::make_unique<Draft07>(json, baseUri);
}