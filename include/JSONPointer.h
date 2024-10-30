#ifndef JSONPOINTER_H
#define JSONPOINTER_H

#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <map>

class JSONPointer
{
  std::vector<std::string> tokens;
  std::string anchor = "";

  /// @brief Escapes a fragment text. This is slightly different from the
  /// escapeString function, as this doesn't escape the '/' character.
  /// @param fragment The fragment string to escape
  /// @return The escaped fragment string
  static std::string escapeURI(const std::string &fragment);

  /// @brief Escapes a pointer token according to RFC6901
  /// @param token The reference token to escape.
  /// @return
  static std::string escapePointer(const std::string &token);

  /// @brief Unescapes a URI fragment string
  /// @param fragment The fragment string to unescape
  /// @return The unescaped fragment string
  static std::string unescapeURI(const std::string &fragment);

  /// @brief Unescapes a reference token of a JSONPointer according to RFC6901
  /// @param token The token to unescape
  /// @return The unescaped token
  static std::string unescapeToken(const std::string &token);

private:
  /// @brief Splits a fragment string into its reference tokens.
  /// @param fragment The fragment string to split
  /// @return A vector of string views that represent the tokens
  /// @warning This function does not unescape the tokens. The returned string_views
  /// are views into the original string and will be invalidated if the original string
  /// is destroyed.
  static std::vector<std::string_view>
  splitFragment(const std::string &fragment);

public:
  /// @brief Generates an empty JSONPointer with an anchor.
  static JSONPointer fromAnchor(const std::string &anchor);

  /// @brief Parses a JSONPointer from a string. This parses the input string
  /// according to RFC6901 Section 5
  static JSONPointer fromJSONString(const std::string &pointer);

  /// @brief Parses a JSONPointer from a URI fragment string.
  /// @details This function treats it's input as if the string was escaped
  /// according to RFC3986, then unescapes it and parses it as a JSONPointer.
  static JSONPointer fromURIString(const std::string &uri);

  JSONPointer() : tokens() {}

  /// @brief Adds a reference token to the JSONPointer
  /// @param token The unescaped token to add.
  void add(const std::string &token);

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
  /// @details This will clear the path as well
  /// @param anchor
  void setAnchor(const std::string &anchor);

  /// @brief Returns the unescaped path of the JSONPointer
  /// @deprecated This function is deprecated. Why would you want to get the
  /// unescaped path?
  [[deprecated("Why would you want to get the unescaped path?")]]
  std::vector<std::string> getPath() const
  {
    return tokens;
  }

  /// @brief Converts the JSONPointer to a fragment string. It escapes the
  /// tokens and the anchor.
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
  nlohmann::json &navigate(::nlohmann::json &anchoredJson) const;

  JSONPointer operator/(const std::string &token);
};

#endif // JSONPOINTER_H
