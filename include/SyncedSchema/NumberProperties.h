#ifndef SYNCEDSCHEMA_NUMBERPROPERTIES_H
#define SYNCEDSCHEMA_NUMBERPROPERTIES_H
#include <optional>
#include <string>

struct NumberProperties {
  std::optional<double> multipleOf_;
  std::optional<double> maximum_;
  std::optional<double> exclusiveMaximum_;
  std::optional<double> minimum_;
  std::optional<double> exclusiveMinimum_;

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

  std::string getNumberType() const;
  std::string getIntegerType() const;
  IntegerType getIntegerEnum() const;
};

#endif // SYNCEDSCHEMA_NUMBERPROPERTIES_H