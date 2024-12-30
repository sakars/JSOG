#ifndef INDEXED_REINTERPRETABLES_H
#define INDEXED_REINTERPRETABLES_H

#include <optional>
#include <vector>

/// @brief Represents the schema version synchronized
/// properties of reinterpretables constraints.
struct IndexedReinterpretables {
  /// @brief Equivalent to the "$ref" keyword in JSON Schema Draft 7
  std::optional<size_t> ref_;

  /// @brief Equivalent to the "if" keyword in JSON Schema Draft 7
  std::optional<size_t> if_;
  /// @brief Equivalent to the "then" keyword in JSON Schema Draft 7
  std::optional<size_t> then_;
  /// @brief Equivalent to the "else" keyword in JSON Schema Draft 7
  std::optional<size_t> else_;
  /// @brief Equivalent to the "allOf" keyword in JSON Schema Draft 7
  std::optional<std::vector<size_t>> allOf_;
  /// @brief Equivalent to the "anyOf" keyword in JSON Schema Draft 7
  std::optional<std::vector<size_t>> anyOf_;
  /// @brief Equivalent to the "oneOf" keyword in JSON Schema Draft 7
  std::optional<std::vector<size_t>> oneOf_;
  /// @brief Equivalent to the "not" keyword in JSON Schema Draft 7
  std::optional<size_t> not_;
};

#endif // INDEXED_REINTERPRETABLES_H