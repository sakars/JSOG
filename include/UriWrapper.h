#ifndef URIWRAPPER_H
#define URIWRAPPER_H

#include "JSONPointer.h"
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

  /// @brief Clones the internal C URI object
  /// @return A new URI instance that is a clone of the internal URI object
  std::unique_ptr<UriUriA> cloneUri() const;

  /// @brief Applies a relative URI to an absolute base URI
  /// @param base The base URI instance
  /// @param relative The relative URI instance
  /// @return A new URI instance that is the result of applying the relative URI
  static std::unique_ptr<UriUriA> applyUri(const UriUriA& base,
                                           const UriUriA& relative);

  UriWrapper(std::unique_ptr<UriUriA> uri) : uri_(std::move(uri)) {}

public:
  /// @brief Escapes a string according to RFC3986 (URI encoding)
  /// @param str The string to escape
  /// @return The escaped string
  /// @note This function percent-encodes all unreserved characters.
  static std::string escapeString(const std::string& str);

  /// @brief Applies a relative URI to an absolute base URI
  /// @param base Base URI that must be absolute as defined by RFC3986
  /// @param relative Relative URI to apply to the base URI as defined by
  /// RFC3986
  /// @return A new URI instance that is the result of applying the relative URI
  static UriWrapper applyUri(const UriWrapper& base,
                             const UriWrapper& relative);

  UriWrapper() {}
  UriWrapper(const std::string& str);
  UriWrapper(const char* str) : UriWrapper(std::string(str)) {}
  UriWrapper(const UriWrapper& other) { uri_ = other.cloneUri(); }

  UriWrapper(UriWrapper&& other) : uri_(std::move(other.uri_)) {}
  UriWrapper& operator=(UriWrapper&& other);

  UriWrapper& operator=(const UriWrapper& other);

  ~UriWrapper() {
    if (uri_) {
      uriFreeUriMembersA(uri_.get());
    }
    uri_ = nullptr;
  }

  /// @brief Performs Syntax-Based Normalization on the URI according to RFC3986
  /// Section 6.2.2
  /// @details This function normalizes the URI according to the rules defined
  /// in RFC3986 Section 6.2.2
  /// @note This function does not perform scheme-specific normalization such as
  /// retaining case sensitivity should the scheme be case-sensitive
  void normalize();

  /// @brief Attempts to convert the URI to a string representation.
  /// @return The string representation of the URI, or std::nullopt if the URI
  /// is invalid
  std::optional<std::string> toString() const noexcept;

  /// @brief Attempts to convert the URI to a string representation without the
  /// fragment.
  /// @return The string representation of the URI without the fragment, or
  /// std::nullopt if the URI is invalid
  /// @note Not only does this function remove the fragment, but it also removes
  /// the octothorpe (#) character that precedes the fragment
  std::optional<std::string> toFragmentlessString() const;

  /// @brief Attempts to retrieve the fragment of the URI
  /// @return The fragment of the URI if it exists, or std::nullopt if the URI
  /// is invalid or has no fragment
  /// @note The fragment is the part of the URI that follows the octothorpe (#)
  /// character This function does not return the string with the octothorpe
  /// character
  std::optional<std::string> getFragment() const;

  /// @brief Sets the fragment of the URI, replacing the current fragment.
  /// @param fragment The new fragment to set
  /// @param hasStartingOctothorpe Whether the provided fragment has a starting
  /// octothorpe (#) character
  /// @note The fragment is escaped according to RFC3986 before being set
  void setFragment(const std::string& fragment,
                   bool hasStartingOctothorpe = false);

  /// @brief Compares two URIs. Meant to be used in sorting algorithms and
  /// containers like map.
  /// @param other The URI to compare to
  /// @return True if this URI is less than the other URI
  bool operator<(const UriWrapper& other) const;

  /// @brief Compares two URIs for equality
  /// @param other The URI to compare to
  /// @return True if the URIs are equal
  bool operator==(const UriWrapper& other) const;

  JSONPointer getPointer() const;

  void setPointer(const JSONPointer& pointer);

  UriWrapper withPointer(const JSONPointer& pointer) const;

  /// @brief Checks if the URI is absolute
  /// @warning This function does not check if the URI is valid nor does it
  /// check if the path is absolute. This distinction is important as a URI can
  /// be valid and have an absolute path, but not be absolute itself.
  bool isAbsolute() const { return uri_ && uri_->scheme.first; }

  /// @brief Checks if the URI is relative
  /// @ref isAbsolute
  bool isRelative() const { return !isAbsolute(); }
};

/// @brief Outputs the URI to an output stream
/// @param os The output stream to write to
/// @param uri The URI to write
/// @return The output stream reference
/// @note If the URI is invalid, this function will output INVALID_URI
std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                     const UriWrapper& uri);

#endif // URIWRAPPER_H