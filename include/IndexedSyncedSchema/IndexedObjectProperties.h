#ifndef INDEXED_OBJECT_PROPERTIES_H
#define INDEXED_OBJECT_PROPERTIES_H

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

/// @brief Represents the schema version synchronized
/// properties of object constraints.
struct IndexedObjectProperties {
  /// @brief Equivalent to the "maxProperties" keyword in JSON Schema Draft 7
  std::optional<size_t> maxProperties_;
  /// @brief Equivalent to the "minProperties" keyword in JSON Schema Draft 7
  std::optional<size_t> minProperties_;
  /// @brief Equivalent to the "required" keyword in JSON Schema Draft 7
  std::optional<std::set<std::string>> required_;
  /// @brief Equivalent to the "properties" keyword in JSON Schema Draft 7
  std::map<std::string, size_t> properties_;
  /// @brief Equivalent to the "patternProperties" keyword in JSON Schema Draft
  /// 7
  std::optional<std::map<std::string, size_t>> patternProperties_;
  /// @brief Equivalent to the "additionalProperties" keyword in JSON Schema
  /// Draft 7
  std::optional<size_t> additionalProperties_;
  /// @brief Equivalent to the "dependencies" keyword in JSON Schema Draft 7
  /// where the values are strings
  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  /// @brief Equivalent to the "dependencies" keyword in JSON Schema Draft 7
  /// where the values are schemas
  std::optional<std::map<std::string, size_t>> schemaDependencies_;
  /// @brief Equivalent to the "propertyNames" keyword in JSON Schema Draft 7
  std::optional<size_t> propertyNames_;
};

#endif // INDEXED_OBJECT_PROPERTIES_H