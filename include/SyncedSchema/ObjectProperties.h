#ifndef OBJECTPROPERTIES_H
#define OBJECTPROPERTIES_H

#include "CodeBlock.h"
#include "CodeProperties.h"
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

struct SyncedSchema;

struct ObjectProperties {
  std::optional<size_t> maxProperties_;
  std::optional<size_t> minProperties_;
  std::optional<std::set<std::string>> required_;
  std::map<std::string, std::reference_wrapper<const SyncedSchema>> properties_;
  std::optional<
      std::map<std::string, std::reference_wrapper<const SyncedSchema>>>
      patternProperties_;
  std::reference_wrapper<const SyncedSchema> additionalProperties_;

  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  std::optional<
      std::map<std::string, std::reference_wrapper<const SyncedSchema>>>
      schemaDependencies_;

  std::optional<std::reference_wrapper<const SyncedSchema>> propertyNames_;

  ObjectProperties(const SyncedSchema& additionalProperties)
      : additionalProperties_(additionalProperties) {}

  std::string getObjectType(std::string namespaceLocation) const;

  CodeBlock objectConstructor(const CodeProperties& codeProperties,
                              const std::string& inputJsonVariableName,
                              const std::string& outSchemaVariableName) const;

  CodeBlock objectClassDefinition(const CodeProperties& codeProperties,
                                  std::string schemaType) const;
};

#endif // OBJECTPROPERTIES_H