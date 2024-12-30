#ifndef INDEXED_ARRAY_PROPERTIES_H
#define INDEXED_ARRAY_PROPERTIES_H

#include <optional>
#include <vector>

/// @brief Represents the schema version synchronized
/// properties of array constraints.
struct IndexedArrayProperties {
  /// @brief Equivalent to the "items" keyword in JSON Schema Draft 7
  /// if the value is an array. The value is an identifier
  /// to another schema.
  std::optional<std::vector<size_t>> tupleableItems_;
  /// @brief Equivalent to "items" keyword in JSON Schema Draft 7
  /// if the value is an object or "additionalItems" if "items" is an array.
  /// The value is an identifier to another schema.
  std::optional<size_t> items_;
  /// @brief Equivalent to the "maxItems" keyword in JSON Schema Draft 7
  std::optional<size_t> maxItems_;
  /// @brief Equivalent to the "minItems" keyword in JSON Schema Draft 7
  std::optional<size_t> minItems_;
  /// @brief Equivalent to the "uniqueItems" keyword in JSON Schema Draft 7
  std::optional<bool> uniqueItems_;
  /// @brief Equivalent to the "contains" keyword in JSON Schema Draft 7
  std::optional<size_t> contains_;
};

#endif // INDEXED_ARRAY_PROPERTIES_H