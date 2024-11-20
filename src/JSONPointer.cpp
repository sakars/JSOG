#include "JSONPointer.h"
#include "UriWrapper.h"

JSONPointer JSONPointer::fromURIString(const std::string& uri,
                                       bool hasOctothorpe) {
  std::string processedUri = uri;
  if (hasOctothorpe) {
    if (processedUri.empty() || processedUri[0] != '#') {
      throw std::runtime_error(
          "URI does not start with octothorpe, but configured to remove it.");
    }
    processedUri = processedUri.substr(1);
  }
  std::string unescapedUri = UriWrapper::unescapeString(processedUri);
  JSONPointer pointer;
  auto tokens = splitFragment(unescapedUri);
  if (tokens.empty()) {
    return pointer;
  }
  // anchors aren't a part of the pointer as per standard and thus isn't
  // required to be unescaped
  pointer.setAnchor(std::string(tokens[0]));
  for (size_t i = 1; i < tokens.size(); i++) {
    pointer.add(unescapeToken(std::string(tokens[i])));
  }
  return pointer;
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
  std::string fragment = ""; //(withOctothorpe ? "#" : "");
  fragment += UriWrapper::escapeString(anchor);
  for (const auto& token : tokens) {
    fragment += UriWrapper::escapeString("/" + escapePointer(token));
  }
  return (withOctothorpe ? "#" : "") + fragment;
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
JSONPointer::splitFragment(const std::string& ptrString) {
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

  auto start = ptrString.begin();
  auto end = ptrString.begin();
  for (auto it = ptrString.begin(); it != ptrString.end(); it++) {
    auto slash = isSlash(std::string_view(it, ptrString.end()));
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
  tokens.emplace_back(std::string_view(start, ptrString.end()));

  return tokens;
}

std::string JSONPointer::escapePointer(const std::string& unescapedPtr) {
  std::string escapedPtr = "";
  for (char c : unescapedPtr) {
    if (c == '~') {
      escapedPtr += "~0";
    } else if (c == '/') {
      escapedPtr += "~1";
    } else {
      escapedPtr += c;
    }
  }
  return escapedPtr;
}

std::string JSONPointer::unescapePointer(const std::string& escapedPtr) {
  std::string unescapedPtr = "";
  for (size_t i = 0; i < escapedPtr.size(); i++) {
    // Try to unescape slashes before tildes to avoid double unescaping
    // slash unescaping (JSON Pointer)
    if (escapedPtr.substr(i).starts_with("~1")) {
      unescapedPtr += "/";
      i += 1;
    }
    // tilde unescaping (JSON Pointer)
    else if (escapedPtr.substr(i).starts_with("~0")) {
      unescapedPtr += "~";
      i += 1;
    } else {
      unescapedPtr += escapedPtr[i];
    }
  }
  return unescapedPtr;
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
