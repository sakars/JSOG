#include "UriWrapper.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

std::unique_ptr<UriUriA> UriWrapper::cloneUri() const {
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

std::unique_ptr<UriUriA> UriWrapper::applyUri(const UriUriA& base,
                                              const UriUriA& relative) {
  auto dest = std::make_unique<UriUriA>();
  std::memset(dest.get(), 0, sizeof(UriUriA));
  if (const auto code = uriAddBaseUriA(dest.get(), &relative, &base);
      code != URI_SUCCESS) {
    uriFreeUriMembersA(dest.get());
    std::cerr << "Failed to apply URI\n";
    if (code == URI_ERROR_NULL) {
      std::cerr << "Memory error\n";
    } else if (code == URI_ERROR_ADDBASE_REL_BASE) {
      std::cerr << "Given base is not absolute\n";
    } else {
      std::cerr << "Unknown error: " << code << std::endl;
    }
    return nullptr;
  }

  uriMakeOwnerA(dest.get());
  return dest;
}

std::string UriWrapper::uriToString(const UriUriA& uri) noexcept {
  int charsRequired = 0;
  if (uriToStringCharsRequiredA(&uri, &charsRequired) != URI_SUCCESS) {
    return "";
  }
  charsRequired += 1;
  std::vector<char> uriString(charsRequired);
  if (uriToStringA(uriString.data(), &uri, charsRequired, NULL) !=
      URI_SUCCESS) {
    return "";
  }
  return std::string(uriString.data(), uriString.size());
}

void UriWrapper::normalizeUri(UriUriA& uri) {
  if (uriNormalizeSyntaxA(&uri) != URI_SUCCESS) {
    throw std::runtime_error("Failed to normalize URI");
  }
}

std::string UriWrapper::escapeString(const std::string& str) {
  // Allocate enough space for the worst case scenario
  // (space to plus and normalize breaks options are turned on)
  // As per the documentation, we need 6 times the size of the input string
  std::vector<char> uriBuffer(6 * str.size() + 1);
  auto end = uriEscapeA(str.c_str(), uriBuffer.data(), URI_TRUE, URI_TRUE);
  std::string escapedUri(uriBuffer.data(), end);
  return escapedUri;
}

std::string UriWrapper::unescapeString(const std::string& str) {
  std::vector<char> uriBuffer(str.size() + 1);
  strncpy(uriBuffer.data(), str.c_str(), str.size());
  uriUnescapeInPlaceA(uriBuffer.data());
  return std::string(uriBuffer.data());
}

UriWrapper UriWrapper::applyUri(const UriWrapper& base,
                                const UriWrapper& relative) {
  if (!base.uri_ || !relative.uri_) {
    return UriWrapper();
  }
  auto newUri = applyUri(*base.uri_, *relative.uri_);
  if (!newUri) {
    std::cerr << "Failed to apply URI\n";
    std::cerr << "Base: " << base.toString().value_or("") << std::endl;
    std::cerr << "Relative: " << relative.toString().value_or("") << std::endl;
    return UriWrapper();
  }
  return UriWrapper(std::move(newUri));
}

UriWrapper::UriWrapper(const std::string& str) {
  uri_ = std::make_unique<UriUriA>();
  const char* errorPos;
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

UriWrapper& UriWrapper::operator=(UriWrapper&& other) {
  if (this == &other) {
    return *this;
  }
  if (uri_) {
    uriFreeUriMembersA(uri_.get());
  }
  uri_ = std::move(other.uri_);
  return *this;
}

UriWrapper& UriWrapper::operator=(const UriWrapper& other) {
  if (this == &other) {
    return *this;
  }
  if (uri_) {
    uriFreeUriMembersA(uri_.get());
  }
  uri_ = other.cloneUri();
  return *this;
}

void UriWrapper::normalize() {
  if (!uri_) {
    return;
  }
  normalizeUri(*uri_);
}

std::optional<std::string> UriWrapper::toString() const noexcept {
  if (!uri_) {
    return std::nullopt;
  }
  int charsRequired = 0;
  if (uriToStringCharsRequiredA(uri_.get(), &charsRequired) != URI_SUCCESS) {
    return std::nullopt;
  }
  charsRequired += 1;
  std::vector<char> uriString(charsRequired);
  if (uriToStringA(uriString.data(), uri_.get(), charsRequired, NULL) !=
      URI_SUCCESS) {
    return std::nullopt;
  }

  auto str = std::string(uriString.data());
  auto hashPos = str.find('#');

  std::string strWithEscapedFragmentSlashes =
      hashPos != std::string::npos ? str.substr(0, hashPos + 1) : str;
  if (hashPos != std::string::npos) {
    for (auto& c : str.substr(hashPos + 1)) {
      if (c == '/') {
        strWithEscapedFragmentSlashes += "%2F";
      } else {
        strWithEscapedFragmentSlashes += c;
      }
    }
  }
  return strWithEscapedFragmentSlashes;
}

std::optional<std::string> UriWrapper::toFragmentlessString() const {
  if (!uri_) {
    return std::nullopt;
  }
  auto uriString = toString();
  if (!uriString) {
    return std::nullopt;
  }
  auto uriStringView = std::string_view(uriString.value());
  auto fragmentPos = uriStringView.find('#');
  if (fragmentPos == std::string::npos) {
    return uriString;
  }
  return std::string(uriStringView.substr(0, fragmentPos));
}

bool UriWrapper::hasFragment() const {
  if (!uri_) {
    return false;
  }
  return uri_->fragment.first != NULL;
}

std::optional<std::string> UriWrapper::getFragment() const {
  if (!uri_) {
    return std::nullopt;
  }
  if (uri_->fragment.first == NULL) {
    return std::nullopt;
  }
  return std::string(uri_->fragment.first, uri_->fragment.afterLast);
}

void UriWrapper::setFragment(const std::string& fragment,
                             bool hasStartingOctothorpe) {
  auto dest = std::make_unique<UriUriA>();
  auto escapedFragment = fragment.substr(hasStartingOctothorpe ? 1 : 0);
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

bool UriWrapper::operator<(const UriWrapper& other) const {
  // We treat a null URI as less than any other URI
  if (!uri_) {
    return other.uri_ != nullptr;
  }
  if (!other.uri_) {
    return false;
  }
  if (*this == other) {
    return false;
  }
  auto uriClone = *this;
  auto otherClone = other;
  uriClone.normalize();
  otherClone.normalize();
  const auto uriString = uriClone.toString();
  const auto otherUriString = otherClone.toString();
  if (!uriString || !otherUriString) {
    return false;
  }
  return uriString.value() < otherUriString.value();
}

bool UriWrapper::operator==(const UriWrapper& other) const {
  if (!uri_ && !other.uri_) {
    return true;
  }
  if (!uri_ || !other.uri_) {
    return false;
  }
  // const auto uriString = toString() + hasFragment() ? "" : "#";
  // const auto otherUriString = other.toString() + other.hasFragment() ? "" :
  // "#";
  // // If either URI is invalid, we can't compare them
  // if (!uriString || !otherUriString) {
  //   return false;
  // }
  // return uriString.value() == otherUriString.value();

  {
    auto uriClone = *this;
    auto otherClone = other;
    uriClone.normalize();
    otherClone.normalize();
    auto uri_ = std::move(uriClone.uri_);
    auto otherUri_ = std::move(otherClone.uri_);
    // compare schemes
    bool schemeExists = uri_->scheme.first != NULL &&
                        uri_->scheme.afterLast != NULL &&
                        uri_->scheme.first + 1 != uri_->scheme.afterLast;
    bool otherSchemeExists =
        otherUri_->scheme.first != NULL &&
        otherUri_->scheme.afterLast != NULL &&
        otherUri_->scheme.first + 1 != otherUri_->scheme.afterLast;
    if (schemeExists != otherSchemeExists) {
      return false;
    }
    if (schemeExists && otherSchemeExists) {
      std::string_view scheme(uri_->scheme.first, uri_->scheme.afterLast);
      std::string_view otherScheme(otherUri_->scheme.first,
                                   otherUri_->scheme.afterLast);
      if (scheme.compare(otherScheme) != 0) {
        return false;
      }
    }

    // compare userInfo
    bool userInfo = uri_->userInfo.first != NULL &&
                    uri_->userInfo.afterLast != NULL &&
                    uri_->userInfo.first != uri_->userInfo.afterLast;
    bool otherUserInfoExists =
        otherUri_->userInfo.first != NULL &&
        otherUri_->userInfo.afterLast != NULL &&
        otherUri_->userInfo.first != otherUri_->userInfo.afterLast;
    if (userInfo != otherUserInfoExists) {
      return false;
    }
    if (userInfo && otherUserInfoExists) {
      std::string_view userInfo(uri_->userInfo.first, uri_->userInfo.afterLast);
      std::string_view otherUserInfo(otherUri_->userInfo.first,
                                     otherUri_->userInfo.afterLast);
      if (userInfo.compare(otherUserInfo) != 0) {
        return false;
      }
    }

    // compare host
    bool hostExists = uri_->hostText.first != NULL &&
                      uri_->hostText.afterLast != NULL &&
                      uri_->hostText.first != uri_->hostText.afterLast;
    bool otherHostExists =
        otherUri_->hostText.first != NULL &&
        otherUri_->hostText.afterLast != NULL &&
        otherUri_->hostText.first != otherUri_->hostText.afterLast;
    if (hostExists != otherHostExists) {
      return false;
    }
    if (hostExists && otherHostExists) {
      std::string_view host(uri_->hostText.first, uri_->hostText.afterLast);
      std::string_view otherHost(otherUri_->hostText.first,
                                 otherUri_->hostText.afterLast);
      if (host.compare(otherHost) != 0) {
        return false;
      }
    }

    // compare port
    bool portExists = uri_->portText.first != NULL &&
                      uri_->portText.afterLast != NULL &&
                      uri_->portText.first != uri_->portText.afterLast;
    bool otherPortExists =
        otherUri_->portText.first != NULL &&
        otherUri_->portText.afterLast != NULL &&
        otherUri_->portText.first != otherUri_->portText.afterLast;
    if (portExists != otherPortExists) {
      return false;
    }
    if (portExists && otherPortExists) {
      std::string_view port(uri_->portText.first, uri_->portText.afterLast);
      std::string_view otherPort(otherUri_->portText.first,
                                 otherUri_->portText.afterLast);
      if (port.compare(otherPort) != 0) {
        return false;
      }
    }

    // compare path
    bool pathExists = uri_->pathHead != NULL && uri_->pathTail != NULL;
    bool otherPathExists =
        otherUri_->pathHead != NULL && otherUri_->pathTail != NULL;
    if (pathExists != otherPathExists) {
      return false;
    }

    auto path = uri_->pathHead;
    auto otherPath = otherUri_->pathHead;

    while (path != nullptr && otherPath != nullptr) {
      std::string_view pathSegment(path->text.first, path->text.afterLast);
      std::string_view otherPathSegment(otherPath->text.first,
                                        otherPath->text.afterLast);
      if (pathSegment.compare(otherPathSegment) != 0) {
        return false;
      }
      path = path->next;
      otherPath = otherPath->next;
    }
    if (path != nullptr || otherPath != nullptr) {
      return false;
    }

    // compare query
    bool queryExists =
        uri_->query.first != NULL && uri_->query.afterLast != NULL;
    bool otherQueryExists =
        otherUri_->query.first != NULL && otherUri_->query.afterLast != NULL;
    if (queryExists != otherQueryExists) {
      return false;
    }

    if (queryExists && otherQueryExists) {
      std::string_view query(uri_->query.first, uri_->query.afterLast);
      std::string_view otherQuery(otherUri_->query.first,
                                  otherUri_->query.afterLast);
      if (query.compare(otherQuery) != 0) {
        return false;
      }
    }

    // compare fragment
    bool fragmentExists = uri_->fragment.first != NULL &&
                          uri_->fragment.afterLast != NULL &&
                          uri_->fragment.first != uri_->fragment.afterLast;
    bool otherFragmentExists =
        otherUri_->fragment.first != NULL &&
        otherUri_->fragment.afterLast != NULL &&
        otherUri_->fragment.first != otherUri_->fragment.afterLast;
    if (fragmentExists != otherFragmentExists) {
      return false;
    }

    if (fragmentExists && otherFragmentExists) {
      std::string_view fragment(uri_->fragment.first, uri_->fragment.afterLast);
      std::string_view otherFragment(otherUri_->fragment.first,
                                     otherUri_->fragment.afterLast);
      if (fragment.compare(otherFragment) != 0) {
        return false;
      }
    }

    return true;
  }
}

JSONPointer UriWrapper::getPointer() const {
  if (!uri_) {
    return JSONPointer();
  }
  auto fragment = getFragment();
  if (!fragment) {
    return JSONPointer();
  }
  return JSONPointer::fromURIString(fragment.value());
}

void UriWrapper::setPointer(const JSONPointer& pointer) {
  if (!uri_) {
    return;
  }
  setFragment(pointer.toFragment(false), false);
}

UriWrapper UriWrapper::withPointer(const JSONPointer& pointer) const {
  auto newUri = *this;
  newUri.setPointer(pointer);
  return newUri;
}

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                     const UriWrapper& uri) {
  auto str = uri.toString();
  os << str.value_or("INVALID_URI");
  return os;
}
