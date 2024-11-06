#ifndef SCHEMA_H
#define SCHEMA_H

#include "CodeBlock.h"
#include "LinkedSchema.h"
#include <map>
#include <memory>

class Schema {
  std::unique_ptr<LinkedSchema> buildableSchema_;
  std::string identifier_;

public:
  Schema(std::unique_ptr<LinkedSchema>&& schema, const std::string& identifier)
      : buildableSchema_(std::move(schema)), identifier_(identifier) {}
};

std::unique_ptr<Schema> construct(std::unique_ptr<LinkedSchema> schema,
                                  const std::string& identifier);

std::map<std::string, std::unique_ptr<Schema>> generateIdentifiableSchemas(
    std::vector<std::unique_ptr<LinkedSchema>>& buildableSchemas) {
  std::map<std::string, std::unique_ptr<Schema>> identifiableSchemas;
  std::set<std::string> identifiers;
  std::vector<std::unique_ptr<Schema>> schemas;

  for (auto& schema : buildableSchemas) {
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
    identifiableSchemas[identifier] = construct(std::move(schema), identifier);
  }
  return identifiableSchemas;
}

#endif // SCHEMA_H
