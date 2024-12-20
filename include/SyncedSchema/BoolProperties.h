
#include "CodeBlock.h"
#include "CodeProperties.h"
#include <string>

class BoolProperties {
public:
  std::string getBooleanType() const { return "bool"; }
  CodeBlock booleanConstructor(const CodeProperties& schema,
                               const std::string& inputJsonVariableName,
                               const std::string& outSchemaVariableName) const;
};
