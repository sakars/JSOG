#ifndef SYNCEDSCHEMA_STRINGPROPERTIES_H
#define SYNCEDSCHEMA_STRINGPROPERTIES_H

#include "CodeBlock.h"
#include "CodeProperties.h"
#include <optional>
#include <string>

struct StringProperties {
  std::optional<size_t> maxLength_;
  std::optional<size_t> minLength_;
  std::optional<std::string> pattern_;

  std::string getStringType() const;

  CodeBlock stringConstructor(const CodeProperties& codeProperties,
                              const std::string& inputJsonVariableName,
                              const std::string& outSchemaVariableName) const;
};

#endif // SYNCEDSCHEMA_STRINGPROPERTIES_H