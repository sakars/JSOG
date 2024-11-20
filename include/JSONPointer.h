#ifndef JSONPOINTER_H
#define JSONPOINTER_H

#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

class JSONPointer {
  /// @brief Fully unescaped reference tokens.
  /// They don't contain neither the leading slash nor the anchor, nor any URI
  /// or JSON Pointer escaping.
  std::vector<std::string> tokens;
  std::string anchor = "";

  /// @brief Escapes a pointer token according to RFC6901 (JSON Pointer)
  /// @param token The reference token to escape.
  /// @return The escaped token string
  /// @note This function does not perform URI escaping.
  static std::string escapePointer(const std::string& token);

  /// @brief Unescapes a pointer token according to RFC6901 (JSON Pointer)
  /// @param token The reference token to unescape.
  /// @return The unescaped token string
  /// @note This function does not perform URI unescaping.
  static std::string unescapePointer(const std::string& token);

  /// @brief Unescapes a reference token of a JSONPointer according to RFC6901
  /// @param token The token to unescape
  /// @return The unescaped token
  static std::string unescapeToken(const std::string& token);

private:
  /// @brief Splits a JSON Pointer string into tokens
  /// @note Requires that fragment has JSON Pointer escaping,
  /// but does not have URI escaping nor the leading octothorpe (#)
  /// @warning Returned output must not outlive the input string
  /// @param fragment The JSON Pointer string to split
  /// @return A vector of string views that point to the tokens in the input
  /// No escaping is done on the tokens
  static std::vector<std::string_view>
  splitFragment(const std::string& fragment);

public:
  /// @brief Parses a JSONPointer from a URI fragment string.
  /// @details This function unescapes input
  /// according to RFC3986, then unescapes it according to RFC6901 and parses it
  static JSONPointer fromURIString(const std::string& uri,
                                   bool hasOctothorpe = true);

  JSONPointer() : tokens() {}

  /// @brief Adds a reference token to the JSONPointer
  /// @param token The unescaped token to add.
  void add(const std::string& token);

  /// @brief Adds an index token to the JSONPointer (for arrays)
  /// @param index The index to add
  void add(int index);

  /// @brief Removes the last token from the JSONPointer
  /// @details If the JSONPointer is empty, this function does nothing
  void up();

  /// @brief Returns the anchor of the JSONPointer
  /// @return The anchor string
  std::string getAnchor() const { return anchor; }

  /// @brief Overwrites the anchor of the JSONPointer
  /// This will clear the path as well
  /// @param anchor
  void setAnchor(const std::string& anchor);

  /// @brief Converts the JSONPointer to a URI fragment string. It escapes the
  /// tokens with JSON Pointer encoding and URI encoding and the anchor with URI
  /// encoding.
  /// @param withOctothorpe If true, the fragment string will start with an
  /// octothorpe (#). True by default.
  /// @return The escaped fragment string
  std::string toFragment(bool withOctothorpe = true) const;

  /// @brief Converts the JSONPointer to a string. Does not escape any
  /// characters
  std::string toString() const;

  /// @brief Navigates a JSON object with the JSONPointer, ignoring the anchor.
  /// @param anchoredJson The JSON object to navigate.
  /// @return The JSON object reference that the JSONPointer points to
  /// @details This function navigates the JSON object with the JSONPointer.
  /// @warning This function throws an exception if the JSONPointer points to a
  /// non-existent location.
  nlohmann::json& navigate(::nlohmann::json& anchoredJson) const;

  /// @brief appends a reference token to the JSONPointer
  /// @param token The unescaped token to add.
  /// @return A new JSONPointer instance with the token appended
  JSONPointer operator/(const std::string& token) const;

  /// @brief Compares two JSONPointers for equality
  /// @param other The JSONPointer to compare to
  /// @return True if the JSONPointers are equal
  bool operator==(const JSONPointer& other) const;
};

#endif // JSONPOINTER_H
