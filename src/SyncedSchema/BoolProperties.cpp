#include "SyncedSchema/BoolProperties.h"
#include <format>

CodeBlock BoolProperties::booleanConstructor(
    const CodeProperties& codeProperties,
    const std::string& inputJsonVariableName,
    const std::string& outSchemaVariableName) const {
  CodeBlock block(codeProperties.indent_);
  BLOCK << std::format("if({}.is_boolean()) {{", inputJsonVariableName)
        << CodeBlock::inc;
  BLOCK << std::format("{} = {}.get<{}>();", outSchemaVariableName,
                       inputJsonVariableName, getBooleanType());
  BLOCK << CodeBlock::dec << "}";

  return block;
}