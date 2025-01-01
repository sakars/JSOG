#ifndef ARRAY_PROPERTIES_H
#define ARRAY_PROPERTIES_H

#include "CodeBlock.h"
#include "CodeProperties.h"
#include "IndexedSyncedSchema.h"
#include <optional>
#include <string>
#include <vector>

class SyncedSchema;

/// @brief Represents the schema version synchronized
/// properties of array constraints.
struct ArrayProperties {
  /// @brief The first items in the array
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>>
      tupleableItems_ = std::nullopt;
  /// @brief The object that matches non-tupleable items in the array
  std::reference_wrapper<const SyncedSchema> items_;
  /// @brief Equivalent to the "maxItems" keyword in JSON Schema Draft 7
  std::optional<size_t> maxItems_ = std::nullopt;
  /// @brief Equivalent to the "minItems" keyword in JSON Schema Draft 7
  std::optional<size_t> minItems_ = std::nullopt;
  /// @brief Equivalent to the "uniqueItems" keyword in JSON Schema Draft 7
  std::optional<bool> uniqueItems_ = std::nullopt;
  /// @brief Equivalent to the "contains" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<const SyncedSchema>> contains_ =
      std::nullopt;

  ArrayProperties(const SyncedSchema& items) : items_(items) {}
  ArrayProperties(
      const IndexedArrayProperties& arrayProperties,
      const std::vector<std::unique_ptr<SyncedSchema>>& syncedSchemas,
      const SyncedSchema& trueSchema);

  /// @brief Gets the C++ type of the array
  std::string getArrayType(std::string namespaceLocation) const;

  /// @brief Generates the code for the array constructor
  /// The returned value is intended to be inserted into a function body with
  /// both input and output variables in scope.
  /// @param codeProperties The properties for the code generation
  /// @param inputJsonVariableName The name of the variable that holds the JSON
  /// @param outSchemaVariableName The name of the variable that will hold the
  /// new value
  /// @return The code block that constructs the array
  CodeBlock arrayConstructor(const CodeProperties& codeProperties,
                             const std::string& inputJsonVariableName,
                             const std::string& outSchemaVariableName) const;

  /// @brief Generates the code for the array class definition
  /// The returned value is intended to be inserted into the namespace scope.
  CodeBlock arrayClassDefinition(const CodeProperties& codeProperties,
                                 std::string schemaType) const;
};

#endif // ARRAY_PROPERTIES_H