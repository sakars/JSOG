#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

/// @brief Normalizes a string to be used as a C++ identifier, meaning that it
/// only contains alphanumeric characters and underscores and starts with an alpha character or underscore.
/// @param s The string to normalize
/// @return The normalized string
std::string normalizeString(std::string s);

/// @brief Checks if a C++ type is comparabke to a JSON primitive type
/// @param s The C++ type to check
/// @return True if the type is comparable to a JSON primitive type
bool iscxxTypeJSONPrimitive(std::string s);

/// @brief Converts a C++ type to an equivalent JSON type.
/// @param s The C++ type to convert
/// @return The equivalent JSON type
std::string cxxTypeToJSONType(std::string s);

#endif // STRINGUTILS_H
