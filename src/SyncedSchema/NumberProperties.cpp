#include "SyncedSchema/NumberProperties.h"
#include "CodeBlock.h"
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>

std::string NumberProperties::getNumberType() const { return "double"; }

NumberProperties::IntegerType
smallestIntegerType(std::optional<double> minimum,
                    std::optional<double> maximum,
                    std::optional<double> exclusiveMinimum,
                    std::optional<double> exclusiveMaximum) {
  using IntegerType = NumberProperties::IntegerType;
  // Initialize effective bounds with provided values or defaults
  double adjustedMin = minimum.value_or(std::numeric_limits<int64_t>::min());
  double adjustedMax = maximum.value_or(std::numeric_limits<int64_t>::max());

  // Adjust bounds based on exclusive values if they are set
  if (exclusiveMinimum && *exclusiveMinimum > adjustedMin) {
    adjustedMin = *exclusiveMinimum;
  }
  if (exclusiveMaximum && *exclusiveMaximum < adjustedMax) {
    adjustedMax = *exclusiveMaximum;
  }

  // Convert adjusted bounds to integers
  int64_t minInt = static_cast<int64_t>(
      std::ceil(adjustedMin)); // Round up for inclusive lower bound
  int64_t maxInt = static_cast<int64_t>(
      std::floor(adjustedMax)); // Round down for inclusive upper bound

  // Determine if signed or unsigned types are needed
  bool needsSigned = minInt < 0;

  // Check each type from smallest to largest
  if (needsSigned) {
    if (minInt >= std::numeric_limits<int8_t>::min() &&
        maxInt <= std::numeric_limits<int8_t>::max())
      return IntegerType::INT8;
    if (minInt >= std::numeric_limits<int16_t>::min() &&
        maxInt <= std::numeric_limits<int16_t>::max())
      return IntegerType::INT16;
    if (minInt >= std::numeric_limits<int32_t>::min() &&
        maxInt <= std::numeric_limits<int32_t>::max())
      return IntegerType::INT32;
    return IntegerType::INT64;
  } else {
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint8_t>::max())
      return IntegerType::UINT8;
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint16_t>::max())
      return IntegerType::UINT16;
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint32_t>::max())
      return IntegerType::UINT32;
    return IntegerType::UINT64;
  }
}

std::string NumberProperties::getIntegerType() const {
  IntegerType type = getIntegerEnum();

  switch (type) {
  case IntegerType::INT8:
    return "int8_t";
  case IntegerType::UINT8:
    return "uint8_t";
  case IntegerType::INT16:
    return "int16_t";
  case IntegerType::UINT16:
    return "uint16_t";
  case IntegerType::INT32:
    return "int32_t";
  case IntegerType::UINT32:
    return "uint32_t";
  case IntegerType::INT64:
    return "int64_t";
  case IntegerType::UINT64:
    return "uint64_t";
  }
  return "int64_t";
}

NumberProperties::IntegerType NumberProperties::getIntegerEnum() const {
  return smallestIntegerType(minimum_, maximum_, exclusiveMinimum_,
                             exclusiveMaximum_);
}

CodeBlock NumberProperties::integerConstructor(
    const CodeProperties& codeProperties,
    const std::string& inputJsonVariableName,
    const std::string& outSchemaVariableName) const {
  CodeBlock block(codeProperties.indent_);
  BLOCK << std::format("if({}.is_number_integer()) {{", inputJsonVariableName)
        << CodeBlock::inc;
  BLOCK << std::format("{} = {}.get<{}>();", outSchemaVariableName,
                       inputJsonVariableName, getIntegerType());
  BLOCK << CodeBlock::dec << "}";

  return block;
}

CodeBlock NumberProperties::numberConstructor(
    const CodeProperties& codeProperties,
    const std::string& inputJsonVariableName,
    const std::string& outSchemaVariableName) const {
  CodeBlock block(codeProperties.indent_);
  BLOCK << std::format("if({}.is_number()) {{", inputJsonVariableName)
        << CodeBlock::inc;
  BLOCK << std::format("{} = {}.get<{}>();", outSchemaVariableName,
                       inputJsonVariableName, getNumberType());
  BLOCK << CodeBlock::dec << "}";

  return block;
}
