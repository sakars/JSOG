#include "StringUtils.h"
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

std::string sanitizeString(std::string s) {
  static const std::string hex = "0123456789ABCDEF";
  static const auto escapeChar = [](unsigned char c) -> std::string {
    return std::string("_") + hex[(c & 0xF0) >> 4] + hex[c & 0x0F];
  };
  std::string outStr = "";

  // Escape the first character into a hex code if it is a number.
  if (s[0] >= '0' && s[0] <= '9') {
    outStr += escapeChar(s[0]);
    s = s.substr(1);
  }
  for (const auto& c : s) {
    if (c == ' ') {
      outStr += "_s";
    } else if (c == '_') {
      outStr += "__";
    } else if ((c <= '9' && c >= '0') || (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z')) {
      outStr += c;
    } else {
      outStr += escapeChar(static_cast<unsigned char>(c));
    }
  }

  // If the string is empty, add a placeholder.
  if (outStr.empty()) {
    outStr = "_";
  }
  // If the string is a C++ keyword, add a placeholder.
  static const std::unordered_set<std::string> cpp_keywords = {
      "alignas",
      "alignof",
      "and",
      "and_eq",
      "asm",
      "atomic_cancel",
      "atomic_commit",
      "atomic_noexcept",
      "auto",
      "bitand",
      "bitor",
      "bool",
      "break",
      "case",
      "catch",
      "char",
      "char8_t",
      "char16_t",
      "char32_t",
      "class",
      "compl",
      "concept",
      "const",
      "consteval",
      "constexpr",
      "constinit",
      "const_cast",
      "continue",
      "co_await",
      "co_return",
      "co_yield",
      "decltype",
      "default",
      "delete",
      "do",
      "double",
      "dynamic_cast",
      "else",
      "enum",
      "explicit",
      "export",
      "extern",
      "false",
      "float",
      "for",
      "friend",
      "goto",
      "if",
      "inline",
      "int",
      "long",
      "mutable",
      "namespace",
      "new",
      "noexcept",
      "not",
      "not_eq",
      "nullptr",
      "operator",
      "or",
      "or_eq",
      "private",
      "protected",
      "public",
      "register",
      "reinterpret_cast",
      "requires",
      "return",
      "short",
      "signed",
      "sizeof",
      "static",
      "static_assert",
      "static_cast",
      "struct",
      "switch",
      "synchronized",
      "template",
      "this",
      "thread_local",
      "throw",
      "true",
      "try",
      "typedef",
      "typeid",
      "typename",
      "union",
      "unsigned",
      "using",
      "virtual",
      "void",
      "volatile",
      "wchar_t",
      "while",
      "xor",
      "xor_eq"};
  for (const auto& keyword : cpp_keywords) {
    if (outStr.starts_with(keyword)) {
      outStr += "_";
      break;
    }
  }

  return outStr;
}

bool iscxxTypeJSONPrimitive(std::string s) {
  static const std::unordered_map<std::string, std::string> typeMap = {
      {"std::string", "string"}, {"double", "number"},
      {"int", "integer"},        {"unsigned int", "integer"},
      {"long", "integer"},       {"unsigned long", "integer"},
      {"bool", "boolean"},       {"std::monostate", "null"}};

  return typeMap.find(s) != typeMap.end();
}

std::string cxxTypeToJSONType(std::string s) {
  static const std::unordered_map<std::string, std::string> typeMap = {
      {"std::string", "string"}, {"double", "number"},
      {"int", "integer"},        {"unsigned int", "integer"},
      {"long", "integer"},       {"unsigned long", "integer"},
      {"bool", "boolean"},       {"std::monostate", "null"}};

  auto it = typeMap.find(s);
  if (it != typeMap.end()) {
    return it->second;
  }
  return "object";
}
