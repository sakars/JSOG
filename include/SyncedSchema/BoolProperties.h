
#include "CodeBlock.h"
#include "CodeProperties.h"
#include <string>

/// @brief Represents the schema version synchronized
/// properties of boolean constraints. For now, there's none
/// but this class is here for future extensibility and method grouping.
class BoolProperties {
public:
  /// @brief Gets the C++ type of the boolean
  /// @return The C++ type of the boolean
  std::string getBooleanType() const { return "bool"; }
  /// @brief Generates the code for the boolean constructor
  /// The returned value is intended to be inserted into a function body with
  /// both input and output variables in scope.
  /// @param properties Code properties for the code generation
  /// @param inputJsonVariableName The name of the variable that holds the JSON
  /// @param outSchemaVariableName The name of the variable that will hold the
  /// new value
  /// @return The code block that constructs the boolean value
  CodeBlock booleanConstructor(const CodeProperties& properties,
                               const std::string& inputJsonVariableName,
                               const std::string& outSchemaVariableName) const;
};
