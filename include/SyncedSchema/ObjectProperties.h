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

/// @brief Represents the schema version synchronized
/// properties of object constraints.
struct ObjectProperties {
  /// @brief Equivalent to the "maxProperties" keyword in JSON Schema Draft 7
  std::optional<size_t> maxProperties_;
  /// @brief Equivalent to the "minProperties" keyword in JSON Schema Draft 7
  std::optional<size_t> minProperties_;
  /// @brief Equivalent to the "required" keyword in JSON Schema Draft 7
  std::optional<std::set<std::string>> required_;
  /// @brief Equivalent to the "properties" keyword in JSON Schema Draft 7
  std::map<std::string, std::reference_wrapper<const SyncedSchema>> properties_;
  /// @brief Equivalent to the "patternProperties" keyword in JSON Schema Draft
  /// 7
  std::optional<
      std::map<std::string, std::reference_wrapper<const SyncedSchema>>>
      patternProperties_;
  /// @brief Equivalent to the "additionalProperties" keyword in JSON Schema
  std::reference_wrapper<const SyncedSchema> additionalProperties_;

  /// @brief Equivalent to the "dependencies" keyword in JSON Schema Draft 7
  /// where the values are strings
  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  /// @brief Equivalent to the "dependencies" keyword in JSON Schema Draft 7
  /// where the values are schemas
  std::optional<
      std::map<std::string, std::reference_wrapper<const SyncedSchema>>>
      schemaDependencies_;

  /// @brief Equivalent to the "propertyNames" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<const SyncedSchema>> propertyNames_;

  ObjectProperties(const SyncedSchema& additionalProperties)
      : additionalProperties_(additionalProperties) {}

  /// @brief Gets the C++ type of the object
  /// @param namespaceLocation The namespace location of the object
  /// @return The C++ type of the object
  std::string getObjectType(std::string namespaceLocation) const;

  /// @brief Generates the code for the object constructor
  /// The returned value is intended to be inserted into a function body with
  /// both input and output variables in scope.
  /// @param codeProperties The properties for the code generation
  /// @param inputJsonVariableName The name of the variable that holds the JSON
  /// @param outSchemaVariableName The name of the variable that will hold the
  /// new value
  /// @return The code block that constructs the object value
  CodeBlock objectConstructor(const CodeProperties& codeProperties,
                              const std::string& inputJsonVariableName,
                              const std::string& outSchemaVariableName) const;

  /// @brief Generates the code for the object class definition
  /// The returned value is intended to be inserted into the namespace scope.
  /// @param codeProperties The properties for the code generation
  /// @param schemaType The type of the schema
  /// @return The code block that defines the object class
  CodeBlock objectClassDefinition(const CodeProperties& codeProperties,
                                  std::string schemaType) const;
};

#endif // OBJECTPROPERTIES_H