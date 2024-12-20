#ifndef ARRAY_PROPERTIES_H
#define ARRAY_PROPERTIES_H

#include "CodeBlock.h"
#include "CodeProperties.h"
#include "IndexedSyncedSchema.h"
#include <optional>
#include <string>
#include <vector>

class SyncedSchema;

struct ArrayProperties {
  /// @brief The first items in the array
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>>
      tupleableItems_ = std::nullopt;
  /// @brief The object that matches non-tupleable items in the array
  std::reference_wrapper<const SyncedSchema> items_;
  std::optional<size_t> maxItems_ = std::nullopt;
  std::optional<size_t> minItems_ = std::nullopt;
  std::optional<bool> uniqueItems_ = std::nullopt;
  std::optional<std::reference_wrapper<const SyncedSchema>> contains_ =
      std::nullopt;

  ArrayProperties(const SyncedSchema& items) : items_(items) {}
  ArrayProperties(
      const IndexedArrayProperties& arrayProperties,
      const std::vector<std::unique_ptr<SyncedSchema>>& syncedSchemas);

  std::string getArrayType(std::string namespaceLocation) const;

  CodeBlock arrayConstructor(const CodeProperties& codeProperties,
                             const std::string& inputJsonVariableName,
                             const std::string& outSchemaVariableName) const;

  CodeBlock arrayClassDefinition(const CodeProperties& codeProperties,
                                 std::string schemaType) const;
};

#endif // ARRAY_PROPERTIES_H