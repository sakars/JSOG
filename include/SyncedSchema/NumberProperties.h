#ifndef SYNCEDSCHEMA_NUMBERPROPERTIES_H
#define SYNCEDSCHEMA_NUMBERPROPERTIES_H
#include "CodeBlock.h"
#include "CodeProperties.h"
#include <optional>
#include <string>

class SyncedSchema;

/// @brief Represents the schema version synchronized
/// properties of number constraints.
struct NumberProperties {
  /// @brief Equivalent to the "multipleOf" keyword in JSON Schema Draft 7
  std::optional<double> multipleOf_ = std::nullopt;
  /// @brief Equivalent to the "maximum" keyword in JSON Schema Draft 7
  std::optional<double> maximum_ = std::nullopt;
  /// @brief Equivalent to the "exclusiveMaximum" keyword in JSON Schema Draft 7
  std::optional<double> exclusiveMaximum_ = std::nullopt;
  /// @brief Equivalent to the "minimum" keyword in JSON Schema Draft 7
  std::optional<double> minimum_ = std::nullopt;
  /// @brief Equivalent to the "exclusiveMinimum" keyword in JSON Schema Draft 7
  std::optional<double> exclusiveMinimum_ = std::nullopt;

  /// @brief The integer type of the number
  enum class IntegerType {
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64
  };

  NumberProperties() = default;
  NumberProperties(const NumberProperties& other) = default;

  /// @brief Gets the C++ type of the number JSON type
  std::string getNumberType() const;
  /// @brief Gets the C++ type of the integer JSON type
  std::string getIntegerType() const;
  /// @brief Gets the integer type of the number
  IntegerType getIntegerEnum() const;
  /// @brief  Generates the code for the number constructor
  /// The returned value is intended to be inserted into a function body with
  /// both input and output variables in scope.
  /// @param codeProperties The properties for the code generation
  /// @param inputJsonVariableName The name of the variable that holds the JSON
  /// @param outSchemaVariableName The name of the variable that will hold the
  /// new value
  /// @return The code block that constructs the number value
  CodeBlock numberConstructor(const CodeProperties& codeProperties,
                              const std::string& inputJsonVariableName,
                              const std::string& outSchemaVariableName) const;

  /// @brief Generates the code for the integer constructor
  /// The returned value is intended to be inserted into a function body with
  /// both input and output variables in scope.
  /// @param codeProperties The properties for the code generation
  /// @param inputJsonVariableName The name of the variable that holds the JSON
  /// @param outSchemaVariableName The name of the variable that will hold the
  /// new value
  /// @return The code block that constructs the integer value
  CodeBlock integerConstructor(const CodeProperties& codeProperties,
                               const std::string& inputJsonVariableName,
                               const std::string& outSchemaVariableName) const;
};

#endif // SYNCEDSCHEMA_NUMBERPROPERTIES_H