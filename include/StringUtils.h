#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

/// @brief Normalizes a string to be used as a C++ identifier, meaning that it
/// only contains alphanumeric characters and underscores and starts with an
/// alpha character or underscore.
/// @param s The string to normalize
/// @return The normalized string
std::string sanitizeString(std::string s);

#endif // STRINGUTILS_H
