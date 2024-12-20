
#include "SyncedSchema/StringProperties.h"
#include "CodeBlock.h"
#include "CodeProperties.h"
#include <format>

std::string StringProperties::getStringType() const { return "std::string"; }

CodeBlock StringProperties::stringConstructor(
    const CodeProperties& codeProperties,
    const std::string& inputJsonVariableName,
    const std::string& outSchemaVariableName) const {
  CodeBlock block(codeProperties.indent_);
  BLOCK << std::format("if({}.is_string()) {{", inputJsonVariableName)
        << CodeBlock::inc;
  BLOCK << std::format("{} = {}.get<{}>();", outSchemaVariableName,
                       inputJsonVariableName, getStringType());
  BLOCK << CodeBlock::dec << "}";

  return block;
}
