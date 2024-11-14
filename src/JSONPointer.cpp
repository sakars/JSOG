#include "JSONPointer.h"

JSONPointer JSONPointer::fromAnchor(const std::string& anchor) {
  JSONPointer pointer;
  pointer.anchor = anchor;
  return pointer;
}

JSONPointer JSONPointer::fromJSONString(const std::string& pointer) {
  JSONPointer outPointer;
  const auto tokens = splitFragment(pointer);
  if (tokens.empty()) {
    return outPointer;
  }

  outPointer.anchor = unescapeToken(std::string(tokens[0]));
  for (const auto& token : tokens | std::views::drop(1)) {
    outPointer.add(unescapeToken(std::string(token)));
  }
  return outPointer;
}

JSONPointer JSONPointer::fromURIString(const std::string& uri) {
  return fromJSONString(std::string(unescapeURI(uri)));
}

void JSONPointer::add(const std::string& token) { tokens.push_back(token); }

void JSONPointer::add(int index) { return add(std::to_string(index)); }

void JSONPointer::up() {
  if (tokens.empty()) {
    return;
  }
  tokens.pop_back();
}

void JSONPointer::setAnchor(const std::string& anchor) {
  this->anchor = anchor;
  tokens.clear();
}

std::string JSONPointer::toFragment(bool withOctothorpe) const {
  std::string fragment = (withOctothorpe ? "#" : "");
  fragment += escapePointer(escapeURI(anchor));
  for (const auto& token : tokens) {
    fragment += "/" + escapePointer(escapeURI(token));
  }
  return fragment;
}

/// @brief Converts the JSONPointer to a string. Escapes only characters that
/// are required to be escaped by the JSON pointer standard (RFC6901)
std::string JSONPointer::toString() const {
  std::string fragment = "";
  fragment += escapePointer(anchor);
  for (const auto& token : tokens) {
    fragment += "/" + escapePointer(token);
  }
  return fragment;
}

nlohmann::json& JSONPointer::navigate(::nlohmann::json& anchoredJson) const {
  nlohmann::json* currentNode = &anchoredJson;
  for (const auto& token : tokens) {
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
      long index = std::stol(token);
      if (index < 0 || (size_t)index >= currentNode->size()) {
        throw std::runtime_error("Index out of bounds");
      }
      currentNode = &(*currentNode)[index];
    } else {
      throw std::runtime_error("Invalid JSON type");
    }
  }
  return *currentNode;
}

JSONPointer JSONPointer::operator/(const std::string& token) const {
  JSONPointer newPointer = *this;
  newPointer.add(token);
  return newPointer;
}

bool JSONPointer::operator==(const JSONPointer& other) const {
  return anchor == other.anchor && tokens == other.tokens;
}

std::vector<std::string_view>
JSONPointer::splitFragment(const std::string& fragment) {
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

std::string JSONPointer::escapeURI(const std::string& fragment) {
  if (fragment.empty())
    return "";
  static const std::map<char, std::string> uriEscapeMap = {
      {'#', "%23"}, {'%', "%25"},  {'?', "%3F"}, {'&', "%26"}, {'=', "%3D"},
      {'"', "%22"}, {'\'', "%27"}, {'<', "%3C"}, {'>', "%3E"}, {'\\', "%5C"},
      {'{', "%7B"}, {'}', "%7D"},  {'|', "%7C"}, {' ', "%20"}};

  std::string escapedFragment;
  for (char c : fragment) {
    if (uriEscapeMap.count(c)) {
      escapedFragment += uriEscapeMap.at(c);
    } else {
      escapedFragment += c;
    }
  }
  return escapedFragment;
}

std::string JSONPointer::escapePointer(const std::string& fragment) {
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

std::string JSONPointer::unescapeURI(const std::string& token) {
  std::string unescapedFragment = "";
  for (size_t i = 0; i < token.size(); i++) {
    if (token.substr(i).starts_with("%")) {
      if (i + 2 < token.size()) {
        std::string hexStr = token.substr(i + 1, 2);
        char hexChar = static_cast<char>(std::stoi(hexStr, nullptr, 16));
        unescapedFragment += hexChar;
        i += 2;
      }
    } else {
      unescapedFragment += token[i];
    }
  }
  return unescapedFragment;
}

std::string JSONPointer::unescapeToken(const std::string& token) {
  std::string unescapedToken = "";
  for (size_t i = 0; i < token.size(); i++) {
    // slash unescaping (JSON Pointer)
    if (token.substr(i).starts_with("~1")) {
      unescapedToken += "/";
      i += 1;
    }
    // tilde unescaping (JSON Pointer)
    else if (token.substr(i).starts_with("~0")) {
      unescapedToken += "~";
      i += 1;
    } else {
      unescapedToken += token[i];
    }
  }
  return unescapedToken;
}
