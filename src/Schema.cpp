
#include "Schema.h"
#include "Draft07.h"

std::unique_ptr<Schema> construct(const BuildableSchema& schema,
                                  const std::string& identifier) {
  if (schema.draft_ == Draft::DRAFT_07) {
    return std::make_unique<Draft07>(schema, identifier);
  }
  throw std::runtime_error("Unrecognized schema draft");
}