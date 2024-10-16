#ifndef URIWRAPPER_H
#define URIWRAPPER_H

#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <uriparser/Uri.h>

/// @brief Wrapper around the uriparser library, providing a C++ interface
/// @details This class wraps the uriparser library, providing a C++ interface
/// to the C library. It allows for easy parsing and manipulation of URIs.
/// @note This class is not thread-safe
class UriWrapper {
private:
  // std::optional<UriUriA> uri = std::nullopt;
  std::unique_ptr<UriUriA> uri_ = nullptr;

  std::unique_ptr<UriUriA> cloneUri() const {
    if (uri_ == nullptr) {
      return nullptr;
    }
    std::string uriString = toString().value();
    // Create a new UriWrapper from the string
    UriWrapper clone(uriString);
    // Yank the UriUriA out of the clone
    auto cloneUri = std::move(clone.uri_);
    return cloneUri;
  }

  static std::unique_ptr<UriUriA> applyUri(const UriUriA &base,
                                           const UriUriA &relative) {
    auto dest = std::make_unique<UriUriA>();
    std::memset(dest.get(), 0, sizeof(UriUriA));
    if (uriAddBaseUriA(dest.get(), &relative, &base) != URI_SUCCESS) {
      uriFreeUriMembersA(dest.get());
      return nullptr;
    }

    uriMakeOwnerA(dest.get());
    return dest;
  }

public:
  static std::string escapeString(const std::string &str) {
    std::unique_ptr<char[]> uriString(new char[6 * (str.size() + 1)]);
    auto end = uriEscapeA(str.c_str(), uriString.get(), URI_TRUE, URI_TRUE);
    std::string escapedUri(uriString.get(), end);
    return escapedUri;
  }

  UriWrapper() {}
  UriWrapper(const std::string &str) {

    uri_ = std::make_unique<UriUriA>();
    const char *errorPos;
    if (uriParseSingleUriA(uri_.get(), str.c_str(), &errorPos) != URI_SUCCESS) {
      std::string error = "\nFailed to parse URI.\n";
      error += str;
      error += "\n";
      error += std::string(errorPos - str.c_str(), ' ');
      error += "^\n";
      std::cerr << error;
      throw std::invalid_argument(error);
    }

    uriMakeOwnerA(uri_.get());
  }

  UriWrapper(const UriWrapper &other) { uri_ = std::move(other.cloneUri()); }

  UriWrapper(UriWrapper &&other) : uri_(std::move(other.uri_)) {}
  UriWrapper &operator=(UriWrapper &&other) {
    if (this == &other) {
      return *this;
    }
    if (uri_) {
      uriFreeUriMembersA(uri_.get());
    }
    uri_ = std::move(other.uri_);
    return *this;
  }

  UriWrapper &operator=(const UriWrapper &other) {
    if (this == &other) {
      return *this;
    }
    if (uri_) {
      uriFreeUriMembersA(uri_.get());
    }
    uri_ = std::move(other.cloneUri());
    return *this;
  }

  ~UriWrapper() {
    if (uri_) {
      uriFreeUriMembersA(uri_.get());
    }
    uri_ = nullptr;
  }

  std::optional<std::string> toString() const {
    if (!uri_) {
      return std::nullopt;
    }
    int charsRequired = 0;
    if (uriToStringCharsRequiredA(uri_.get(), &charsRequired) != URI_SUCCESS) {
      return std::nullopt;
    }
    charsRequired += 1;
    std::unique_ptr<char[]> uriString(new char[charsRequired]);
    if (uriToStringA(uriString.get(), uri_.get(), charsRequired, NULL) !=
        URI_SUCCESS) {
      return std::nullopt;
    }

    return std::string(uriString.get());
  }

  std::optional<std::string> getFragment() const {
    if (!uri_) {
      return std::nullopt;
    }
    if (uri_->fragment.first == NULL) {
      return std::nullopt;
    }
    std::unique_ptr<char[]> fragment(
        new char[uri_->fragment.afterLast - uri_->fragment.first + 1]);
    std::memcpy(fragment.get(), uri_->fragment.first,
                uri_->fragment.afterLast - uri_->fragment.first);
    fragment[uri_->fragment.afterLast - uri_->fragment.first] = '\0';
    uriUnescapeInPlaceA(fragment.get());
    return std::string(fragment.get());
  }

  void setFragment(const std::string &fragment) {
    auto dest = std::make_unique<UriUriA>();
    auto escapedFragment = escapeString(fragment);
    escapedFragment = "#" + escapedFragment;
    // perform parsing on the fragment
    const UriWrapper fragmentUri(escapedFragment);
    // clone the current URI
    auto fragmentUriA = fragmentUri.cloneUri();
    if (!uri_) {
      uri_ = std::move(fragmentUriA);
      return;
    }
    auto newUri = applyUri(*uri_, *fragmentUriA);
    if (!newUri) {
      throw std::runtime_error("Failed to apply fragment to URI");
    }
    uriFreeUriMembersA(uri_.get());
    uriFreeUriMembersA(fragmentUriA.get());
    uri_ = std::move(newUri);
  }
};

#endif // URIWRAPPER_H