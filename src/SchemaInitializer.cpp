#include "SchemaInitializer.h"
#include "Draft07.h"
#include "JSONPointer.h"
#include <stdexcept>

std::unique_ptr<Schema> initializeSchema(const nlohmann::json &json,
                                         std::string baseUri,
                                         JSONPointer jsonPointer, Draft draft) {
  // TODO: Implement schema version detection. For MVP only Draft07 will do.
  if (draft == Draft::DRAFT_07) {
    return std::make_unique<Draft07>(json, baseUri, jsonPointer);
  }
  std::cerr << "Attempted Schema initialization with an unrecognized draft";
  return std::make_unique<Draft07>(json, baseUri, jsonPointer);
}