#ifndef CODEPROPERTIES_H
#define CODEPROPERTIES_H

#include <optional>
#include <string>

struct CodeProperties {
  enum class HeaderGuard { Pragma, Ifndef };
  HeaderGuard headerGuardType_ = HeaderGuard::Ifndef;
  std::string indent_ = "  ";
  std::optional<std::string> globalNamespace_ = "JSOG";
  std::optional<std::string> definePrefix_ = "JSOG_";

  // Feature flags

  /// @brief If set, whenever a schema is defined as an array,
  /// tupleableItems_ is set, and minItems_ is set to at least 1,
  /// the generated code will make itemN required, not optional.
  /// @todo This feature is not yet tested
  bool minItemsMakeTupleableRequired_ = false;
};

#endif // CODEPROPERTIES_H