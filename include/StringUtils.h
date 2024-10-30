#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

inline std::string normalizeString(std::string s)
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

inline bool iscxxTypeJSONPrimitive(std::string s)
{
  return s == "std::string" || s == "double" || s == "int" || s == "unsigned int" ||
         s == "long" || s == "unsigned long" || s == "bool";
}

inline std::string cxxTypeToJSONType(std::string s)
{
  if (!iscxxTypeJSONPrimitive(s))
  {
    return "object";
  }
  if (s == "std::string")
  {
    return "string";
  }
  else if (s == "double")
  {
    return "number";
  }
  else if (s == "int")
  {
    return "integer";
  }
  else if (s == "unsigned int")
  {
    return "integer";
  }
  else if (s == "long")
  {
    return "integer";
  }
  else if (s == "unsigned long")
  {
    return "integer";
  }
  else if (s == "bool")
  {
    return "boolean";
  }
  else if (s == "std::monostate")
  {
    return "null";
  }
  throw std::runtime_error("Unknown type: " + s);
}

#endif // STRINGUTILS_H
