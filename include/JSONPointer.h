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

class JSONPointer {
  std::vector<std::string> tokens;
  std::string anchor = "";

  /// @brief Escapes a fragment text. This is slightly different from the
  /// escapeString function, as this doesn't escape the '/' character.
  /// @param fragment The fragment string to escape
  /// @return The escaped fragment string
  static std::string escapeURI(const std::string &fragment) {
    std::string escapedFragment = "";
    for (char c : fragment) {
      if (c == '#') {
        escapedFragment += "%23";
      } else if (c == '%') {
        escapedFragment += "%25";
      } else if (c == '?') {
        escapedFragment += "%3F";
      } else if (c == '&') {
        escapedFragment += "%26";
      } else if (c == '=') {
        escapedFragment += "%3D";
      } else if (c == '"') {
        escapedFragment += "%22";
      } else if (c == '\'') {
        escapedFragment += "%27";
      } else if (c == '<') {
        escapedFragment += "%3C";
      } else if (c == '>') {
        escapedFragment += "%3E";
      } else if (c == '\\') {
        escapedFragment += "%5C";
      } else if (c == '{') {
        escapedFragment += "%7B";
      } else if (c == '}') {
        escapedFragment += "%7D";
      } else if (c == '|') {
        escapedFragment += "%7C";
      } else if (c == ' ') {
        escapedFragment += "%20";
      } else {
        escapedFragment += c;
      }
    }

    return escapedFragment;
  }

  static std::string escapePointer(const std::string &fragment) {
    std::string escapedFragment = "";
    for (char c : fragment) {

      if (c == '~') {
        escapedFragment += "~0";
      } else if (c == '/') {
        escapedFragment += "~1";
      } else {
        escapedFragment += c;
      }
    }

    return escapedFragment;
  }

  static std::string unescapeURI(const std::string &fragment) {
    std::string unescapedFragment = "";
    for (size_t i = 0; i < fragment.size(); i++) {
      if (fragment.substr(i).starts_with("%")) {
        if (i + 2 < fragment.size()) {
          std::string hexStr = fragment.substr(i + 1, 2);
          char hexChar = static_cast<char>(std::stoi(hexStr, nullptr, 16));
          unescapedFragment += hexChar;
          i += 2;
        }
      } else {
        unescapedFragment += fragment[i];
      }
    }
    return unescapedFragment;
  }

  static std::string unescapeToken(const std::string &fragment) {
    std::string unescapedToken = "";
    for (size_t i = 0; i < fragment.size(); i++) {
      // slash unescaping (JSON Pointer)
      if (fragment.substr(i).starts_with("~1")) {
        unescapedToken += "/";
        i += 1;
        // tilde unescaping (JSON Pointer)
      } else if (fragment.substr(i).starts_with("~0")) {
        unescapedToken += "~";
        i += 1;

        // percent unescaping (URI)
        // } else if (fragment.starts_with("%")) {
        //   if (i + 2 < fragment.size()) {
        //     std::string hexStr = fragment.substr(i + 1, 2);
        //     char hexChar = static_cast<char>(std::stoi(hexStr, nullptr,
        //     16)); unescapedToken += hexChar; i += 3;
        //   }
      } else {
        unescapedToken += fragment[i];
      }
    }
    return unescapedToken;
  }

public:
  static JSONPointer fromAnchor(const std::string &anchor) {
    JSONPointer pointer;
    pointer.anchor = anchor;
    return pointer;
  }

private:
  static std::vector<std::string_view>
  splitFragment(const std::string &fragment) {
    enum class State { None, Literal, Encoded };
    auto isSlash = [](std::string_view c) {
      bool isLiteral = c.starts_with("/");
      bool isEncoded = c.starts_with("%2F") || c.starts_with("%2f");
      if (isLiteral) {
        return State::Literal;
      } else if (isEncoded) {
        return State::Encoded;
      } else {
        return State::None;
      }
    };
    std::vector<std::string_view> tokens;

    auto start = fragment.begin();
    auto end = fragment.begin();
    for (auto it = fragment.begin(); it != fragment.end(); it++) {
      auto slash = isSlash(std::string_view(it, fragment.end()));
      if (slash == State::Literal) {
        end = it;
        tokens.emplace_back(std::string_view(start, end));
        start = it + 1;
      } else if (slash == State::Encoded) {
        end = it;
        tokens.emplace_back(std::string_view(start, end));
        start = it + 3;
      }
    }
    tokens.emplace_back(std::string_view(start, fragment.end()));

    return tokens;
  }

public:
  /// @brief Parses a JSONPointer from a string. This parses the input string
  /// according to RFC6901 Section 5
  static JSONPointer fromJSONString(const std::string &pointer) {
    JSONPointer outPointer;
    std::string_view fragmentView = pointer;
    const auto tokens = splitFragment(pointer);
    // for (const auto &token : tokens) {
    //   std::cout << std::quoted(token) << std::endl;
    // }
    // std::cout << "Tokens size: " << tokens.size() << std::endl;
    if (tokens.empty()) {
      return outPointer;
    }
    // std::cout << "Addables: " << std::endl;

    outPointer.anchor = unescapeToken(std::string(tokens[0]));
    for (const auto &token : tokens | std::views::drop(1)) {
      // std::cout << "Adding token: "
      //           << std::quoted(unescapeToken(std::string(token))) <<
      //           std::endl;
      outPointer.add(unescapeToken(std::string(token)));
    }
    return outPointer;
  }

  static JSONPointer fromURIString(const std::string &uri) {
    return fromJSONString(std::string(unescapeURI(uri)));
  }

  JSONPointer() : tokens() {}

  void add(const std::string &token) {
    // std::cout << "Adding token: " << std::quoted(token) << std::endl;
    tokens.push_back(token);
  }

  void add(int index) { return add(std::to_string(index)); }

  void up() {
    if (tokens.empty()) {
      return;
    }
    tokens.pop_back();
  }

  std::string getAnchor() const { return anchor; }
  /// @brief Overwrites the anchor of the JSONPointer
  /// @details This will clear the path as well
  /// @param anchor
  void setAnchor(const std::string &anchor) {
    this->anchor = anchor;
    tokens.clear();
  }
  std::vector<std::string> getPath() const { return tokens; }

  /// @brief Converts the JSONPointer to a fragment string. It escapes the
  /// tokens and the anchor.
  /// @return The escaped fragment string WITH the octothorpe
  std::string toFragment() const {
    std::string fragment = "";
    fragment += "#" + escapePointer(escapeURI(anchor));
    for (const auto &token : tokens) {
      fragment += "/" + escapePointer(escapeURI(token));
    }
    return fragment;
  }

  /// @brief Converts the JSONPointer to a string. Does not escape any
  /// characters
  std::string toString() const {
    std::string fragment = "";
    fragment += escapeURI(anchor);
    for (const auto &token : tokens) {
      fragment += "/" + escapeURI(token);
    }
    return fragment;
  }

  // static std::optional<std::reference_wrapper<::nlohmann::json>>
  // findAnchor(const std::string &anchor, ::nlohmann::json &json) {
  //   if (anchor == "") {
  //     return json;
  //   }
  //   if (json.is_object()) {
  //     if (json.contains("anchor") &&
  //         json["anchor"].get<std::string>() == anchor) {
  //       return json;
  //     }
  //     for (auto &[key, value] : json.items()) {
  //       const auto x = findAnchor(anchor, value);
  //       if (x.has_value()) {
  //         return x;
  //       }
  //     }
  //   } else if (json.is_array()) {
  //     for (auto &value : json) {
  //       const auto x = findAnchor(anchor, value);
  //       if (x.has_value()) {
  //         return x;
  //       }
  //     }
  //   }
  //   return std::nullopt;
  // }

  ::nlohmann::json &navigate(::nlohmann::json &anchoredJson) const {
    // nlohmann::json *anchorRoot = &json;
    // if (anchor != "") {
    //   auto anchorNode = findAnchor(anchor, json);
    //   if (anchorNode.has_value()) {
    //     anchorRoot = &anchorNode.value().get();
    //   }
    // }
    nlohmann::json *currentNode = &anchoredJson;
    for (const auto &token : tokens) {
      if (currentNode->is_object()) {
        if (currentNode->contains(token)) {
          currentNode = &(*currentNode)[token];
        } else {
          throw std::runtime_error("Token not found");
        }
      } else if (currentNode->is_array()) {
        if (token == "-") {
          throw std::runtime_error("Array index '-' not supported");
        }
        int index = std::stoi(token);
        if (index < 0 || index >= currentNode->size()) {
          throw std::runtime_error("Index out of bounds");
        }
        currentNode = &(*currentNode)[index];
      } else {
        throw std::runtime_error("Invalid JSON type");
      }
    }
    return *currentNode;
  }

  JSONPointer operator/(const std::string &token) {
    JSONPointer newPointer = *this;
    newPointer.add(token);
    return newPointer;
  }
};

#endif // JSONPOINTER_H
