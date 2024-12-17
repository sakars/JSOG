#ifndef SYNCEDSCHEMA_NUMBERPROPERTIES_H
#define SYNCEDSCHEMA_NUMBERPROPERTIES_H
#include "CodeBlock.h"
#include "CodeProperties.h"
#include "IndexedSyncedSchema.h"
#include <optional>
#include <string>

class SyncedSchema;

struct NumberProperties {
  std::optional<double> multipleOf_ = std::nullopt;
  std::optional<double> maximum_ = std::nullopt;
  std::optional<double> exclusiveMaximum_ = std::nullopt;
  std::optional<double> minimum_ = std::nullopt;
  std::optional<double> exclusiveMinimum_ = std::nullopt;

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
  NumberProperties(
      const IndexedSyncedSchema& schema,
      const std::vector<std::unique_ptr<SyncedSchema>>& syncedSchemas);

  std::string getNumberType() const;
  std::string getIntegerType() const;
  IntegerType getIntegerEnum() const;
  CodeBlock numberConstructor(const CodeProperties& codeProperties,
                              const std::string& inputJsonVariableName,
                              const std::string& outSchemaVariableName) const;

  CodeBlock integerConstructor(const CodeProperties& codeProperties,
                               const std::string& inputJsonVariableName,
                               const std::string& outSchemaVariableName) const;
};

#endif // SYNCEDSCHEMA_NUMBERPROPERTIES_H