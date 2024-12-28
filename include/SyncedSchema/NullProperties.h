#include <string>

/// @brief Represents the schema version synchronized
/// properties of null constraints. For now, there's none
/// but this class is here for future extensibility and method grouping.
class NullProperties {
public:
  /// @brief Gets the C++ type of the null type
  std::string getNullType() const { return "std::monostate"; }
};