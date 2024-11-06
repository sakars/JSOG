#ifndef SCHEMA_H
#define SCHEMA_H

#include "BuildableSchema.h"
#include "CodeBlock.h"
#include <map>
#include <memory>

class Schema {
  std::string identifier;

public:
  Schema(const std::string& identifier) : identifier(identifier) {}
};

std::unique_ptr<Schema> construct(const BuildableSchema& schema,
                                  const std::string& identifier);

std::map<std::string, std::unique_ptr<Schema>> generateIdentifiableSchemas(
    std::vector<std::unique_ptr<BuildableSchema>>& schemas) {
  std::map<std::string, std::unique_ptr<Schema>> identifiableSchemas;
  std::set<std::string> identifiers;
  for (const auto& schema : schemas) {
    const std::string preferred_identifier = schema->getPreferredIdentifier();
    std::string identifier = preferred_identifier;
    size_t i = 0;
    while (identifiers.count(identifier) > 0) {
      bool preferred_identifier_has_number_at_end =
          preferred_identifier.back() >= '0' &&
          preferred_identifier.back() <= '9';
      identifier = preferred_identifier +
                   (preferred_identifier_has_number_at_end ? "_" : "") +
                   std::to_string(i);
      i++;
    }
    identifiers.insert(identifier);
    identifiableSchemas[identifier] = construct(*schema, identifier);
  }
  return identifiableSchemas;
}

#endif // SCHEMA_H
