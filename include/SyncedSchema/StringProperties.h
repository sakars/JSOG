#ifndef SYNCEDSCHEMA_STRINGPROPERTIES_H
#define SYNCEDSCHEMA_STRINGPROPERTIES_H

#include "CodeBlock.h"
#include "CodeProperties.h"
#include <optional>
#include <string>

/// @brief Represents the schema version synchronized
/// properties of string constraints.
struct StringProperties {
  /// @brief Equivalent to the "maxLength" keyword in JSON Schema Draft 7
  std::optional<size_t> maxLength_;
  /// @brief Equivalent to the "minLength" keyword in JSON Schema Draft 7
  std::optional<size_t> minLength_;
  /// @brief Equivalent to the "pattern" keyword in JSON Schema Draft 7
  std::optional<std::string> pattern_;

  /// @brief Gets the C++ type of the string
  std::string getStringType() const;

  /// @brief Generates the code for the string constructor
  /// The returned value is intended to be inserted into a function body with
  /// both input and output variables in scope.
  /// @param codeProperties The properties for the code generation
  /// @param inputJsonVariableName The name of the variable that holds the JSON
  /// @param outSchemaVariableName The name of the variable that will hold the
  /// new value
  /// @return The code block that constructs the string value
  CodeBlock stringConstructor(const CodeProperties& codeProperties,
                              const std::string& inputJsonVariableName,
                              const std::string& outSchemaVariableName) const;
};

#endif // SYNCEDSCHEMA_STRINGPROPERTIES_H