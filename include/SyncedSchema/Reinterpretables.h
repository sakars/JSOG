
#include <optional>
#include <vector>

class SyncedSchema;

/// @brief Represents the schema version synchronized
/// properties of reinterpretables constraints.
struct Reinterpretables {
  /// @brief Equivalent to the "$ref" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<SyncedSchema>> ref_;
  /// @brief Equivalent to the "if" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<const SyncedSchema>> if_;
  /// @brief Equivalent to the "then" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<const SyncedSchema>> then_;
  /// @brief Equivalent to the "else" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<const SyncedSchema>> else_;

  /// @brief Equivalent to the "allOf" keyword in JSON Schema Draft 7
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> allOf_;
  /// @brief Equivalent to the "anyOf" keyword in JSON Schema Draft 7
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> anyOf_;
  /// @brief Equivalent to the "oneOf" keyword in JSON Schema Draft 7
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> oneOf_;
  /// @brief Equivalent to the "not" keyword in JSON Schema Draft 7
  std::optional<std::reference_wrapper<const SyncedSchema>> not_;
};