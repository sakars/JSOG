#include "StringUtils.h"
#include <stdexcept>
#include <unordered_map>

std::string normalizeString(std::string s)
{
  std::string outStr = "_";
  for (const auto &c : s)
  {
    if (c == ' ')
    {
      outStr += "_s_";
    }
    else if (c == '_')
    {
      outStr += "__";
    }
    else if (!((c <= '9' && c >= '0') || (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z')))
    {
      outStr += "_d" + std::to_string((unsigned char)c) + "_";
    }
    else
    {
      outStr += c;
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
