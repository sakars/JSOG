#include "UriWrapper.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

std::unique_ptr<UriUriA> UriWrapper::cloneUri() const
{
  if (uri_ == nullptr)
  {
    return nullptr;
  }
  std::string uriString = toString().value();
  // Create a new UriWrapper from the string
  UriWrapper clone(uriString);
  // Yank the UriUriA out of the clone
  auto cloneUri = std::move(clone.uri_);
  return cloneUri;
}

std::unique_ptr<UriUriA> UriWrapper::applyUri(const UriUriA &base,
                                              const UriUriA &relative)
{
  auto dest = std::make_unique<UriUriA>();
  std::memset(dest.get(), 0, sizeof(UriUriA));
  if (const auto code = uriAddBaseUriA(dest.get(), &relative, &base);
      code != URI_SUCCESS)
  {
    uriFreeUriMembersA(dest.get());
    std::cerr << "Failed to apply URI\n";
    if (code == URI_ERROR_NULL)
    {
      std::cerr << "Memory error\n";
    }
    else if (code == URI_ERROR_ADDBASE_REL_BASE)
    {
      std::cerr << "Given base is not absolute\n";
    }
    else
    {
      std::cerr << "Unknown error: " << code << std::endl;
    }
    return nullptr;
  }

  uriMakeOwnerA(dest.get());
  return dest;
}

std::string UriWrapper::escapeString(const std::string &str)
{
  // Allocate enough space for the worst case scenario
  // (space to plus and normalize breaks options are turned on)
  // As per the documentation, we need 6 times the size of the input string
  std::vector<char> uriBuffer(6 * str.size() + 1);
  auto end = uriEscapeA(str.c_str(), uriBuffer.data(), URI_TRUE, URI_TRUE);
  std::string escapedUri(uriBuffer.data(), end);
  return escapedUri;
}

UriWrapper UriWrapper::applyUri(const UriWrapper &base,
                                const UriWrapper &relative)
{
  if (!base.uri_ || !relative.uri_)
  {
    return UriWrapper();
  }
  auto newUri = applyUri(*base.uri_, *relative.uri_);
  if (!newUri)
  {
    std::cerr << "Failed to apply URI\n";
    std::cerr << "Base: " << base.toString().value_or("") << std::endl;
    std::cerr << "Relative: " << relative.toString().value_or("")
              << std::endl;
    return UriWrapper();
  }
  return UriWrapper(std::move(newUri));
}

UriWrapper::UriWrapper(const std::string &str)
{

  uri_ = std::make_unique<UriUriA>();
  const char *errorPos;
  if (uriParseSingleUriA(uri_.get(), str.c_str(), &errorPos) != URI_SUCCESS)
  {
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

UriWrapper &UriWrapper::operator=(UriWrapper &&other)
{
  if (this == &other)
  {
    return *this;
  }
  if (uri_)
  {
    uriFreeUriMembersA(uri_.get());
  }
  uri_ = std::move(other.uri_);
  return *this;
}

UriWrapper &UriWrapper::operator=(const UriWrapper &other)
{
  if (this == &other)
  {
    return *this;
  }
  if (uri_)
  {
    uriFreeUriMembersA(uri_.get());
  }
  uri_ = std::move(other.cloneUri());
  return *this;
}

void UriWrapper::normalize()
{
  if (!uri_)
  {
    return;
  }
  if (uriNormalizeSyntaxA(uri_.get()) != URI_SUCCESS)
  {
    throw std::runtime_error("Failed to normalize URI");
  }
}

std::optional<std::string> UriWrapper::toString() const noexcept
{
  if (!uri_)
  {
    return std::nullopt;
  }
  int charsRequired = 0;
  if (uriToStringCharsRequiredA(uri_.get(), &charsRequired) != URI_SUCCESS)
  {
    return std::nullopt;
  }
  charsRequired += 1;
  std::vector<char> uriString(charsRequired);
  if (uriToStringA(uriString.data(), uri_.get(), charsRequired, NULL) !=
      URI_SUCCESS)
  {
    return std::nullopt;
  }

  return std::string(uriString.data());
}

std::optional<std::string> UriWrapper::toFragmentlessString() const
{
  if (!uri_)
  {
    return std::nullopt;
  }
  auto uriString = toString();
  if (!uriString)
  {
    return std::nullopt;
  }
  auto uriStringView = std::string_view(uriString.value());
  auto fragmentPos = uriStringView.find('#');
  if (fragmentPos == std::string::npos)
  {
    return uriString;
  }
  return std::string(uriStringView.substr(0, fragmentPos));
}

std::optional<std::string> UriWrapper::getFragment() const
{
  if (!uri_)
  {
    return std::nullopt;
  }
  if (uri_->fragment.first == NULL)
  {
    return std::nullopt;
  }
  std::vector<char> fragment(uri_->fragment.afterLast - uri_->fragment.first + 1);
  std::memcpy(fragment.data(), uri_->fragment.first,
              uri_->fragment.afterLast - uri_->fragment.first);
  fragment[uri_->fragment.afterLast - uri_->fragment.first] = '\0';
  uriUnescapeInPlaceA(fragment.data());
  return std::string(fragment.data());
}

void UriWrapper::setFragment(const std::string &fragment,
                             bool hasStartingOctothorpe = false)
{
  auto dest = std::make_unique<UriUriA>();
  auto escapedFragment =
      escapeString(fragment.substr(hasStartingOctothorpe ? 1 : 0));
  if (escapedFragment.empty())
  {
    *this = UriWrapper(toFragmentlessString().value_or(""));
    return;
  }
  escapedFragment = "#" + escapedFragment;
  // perform parsing on the fragment
  const UriWrapper fragmentUri(escapedFragment);
  // clone the current URI
  auto fragmentUriA = fragmentUri.cloneUri();
  if (!uri_)
  {
    uri_ = std::move(fragmentUriA);
    return;
  }
  auto newUri = applyUri(*uri_, *fragmentUriA);
  if (!newUri)
  {
    throw std::runtime_error("Failed to apply fragment to URI");
  }
  uriFreeUriMembersA(uri_.get());
  uriFreeUriMembersA(fragmentUriA.get());
  uri_ = std::move(newUri);
}
