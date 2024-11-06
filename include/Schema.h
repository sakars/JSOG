#ifndef SCHEMA_H
#define SCHEMA_H

#include "CodeBlock.h"
#include "IdentifiableSchema.h"
#include <format>
#include <map>
#include <memory>

class Schema {

  std::string identifier;

public:
  Schema(const std::string& identifier) : identifier(identifier) {}
  virtual CodeBlock generateDeclaration() const = 0;

  virtual CodeBlock generateDefinition() const = 0;
};

#endif // SCHEMA_H
