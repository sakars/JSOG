#include "StringUtils.h"
#include <stdexcept>
#include <unordered_map>

std::string normalizeString(std::string s)
{
  static const std::string hex = "0123456789ABCDEF";
  static const auto escapeChar = [](unsigned char c) -> std::string
  {
    return std::string("_") + hex[(c & 0xF0) >> 4] + hex[c & 0x0F];
  };
  std::string outStr = "";

  // Escape the first character into a hex code if it is a number.
  if (s[0] >= '0' && s[0] <= '9')
  {
    outStr += escapeChar(s[0]);
    s = s.substr(1);
  }
  for (const auto &c : s)
  {
    if (c == ' ')
    {
      outStr += "_";
    }
    else if (c == '_')
    {
      outStr += "__";
    }
    else if ((c <= '9' && c >= '0') || (c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z'))
    {
      outStr += c;
    }
    else
    {
      outStr += escapeChar(static_cast<unsigned char>(c));
    }
  }
  return outStr;
}

bool iscxxTypeJSONPrimitive(std::string s)
{
  static const std::unordered_map<std::string, std::string> typeMap = {
      {"std::string", "string"},
      {"double", "number"},
      {"int", "integer"},
      {"unsigned int", "integer"},
      {"long", "integer"},
      {"unsigned long", "integer"},
      {"bool", "boolean"},
      {"std::monostate", "null"}};

  return typeMap.find(s) != typeMap.end();
}

std::string cxxTypeToJSONType(std::string s)
{
  static const std::unordered_map<std::string, std::string> typeMap = {
      {"std::string", "string"},
      {"double", "number"},
      {"int", "integer"},
      {"unsigned int", "integer"},
      {"long", "integer"},
      {"unsigned long", "integer"},
      {"bool", "boolean"},
      {"std::monostate", "null"}};

  auto it = typeMap.find(s);
  if (it != typeMap.end())
  {
    return it->second;
  }
  return "object";
}
