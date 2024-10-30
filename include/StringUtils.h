#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

std::string normalizeString(std::string s);
bool iscxxTypeJSONPrimitive(std::string s);
std::string cxxTypeToJSONType(std::string s);

#endif // STRINGUTILS_H
