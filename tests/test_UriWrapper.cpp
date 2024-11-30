
#define CATCH_CONFIG_MAIN
#include "UriWrapper.h"
#include <catch2/catch_all.hpp>

TEST_CASE("UriWrapper parses URI correctly", "[UriWrapper]") {
  UriWrapper uri("http://example.com:80/path?query=1#fragment");

  SECTION("toString() returns the correct URI") {
    REQUIRE(uri.toString().value_or("") ==
            "http://example.com:80/path?query=1#fragment");
  }

  SECTION("Fragment is parsed correctly") {
    REQUIRE(uri.getFragment() == "fragment");
  }
}

TEST_CASE("UriWrapper copies correctly", "[UriWrapper]") {
  std::string uriString = "http://example.com:80/path?query=1#fragment";
  UriWrapper uri(uriString);

  SECTION("Sanity check") { REQUIRE(uri.toString() == uriString); }

  SECTION("Fragment is copy-constructed correctly") {
    UriWrapper uri2(uri);
    REQUIRE(uri2.toString().value_or("") == uriString);
  }

  SECTION("Fragment is copy-assigned correctly") {
    UriWrapper uri2("http://example2.com");
    uri2 = uri;
    REQUIRE(uri2.toString().value_or("") == uriString);
  }

  SECTION("Fragment is moved correctly") {
    UriWrapper uri2(std::move(uri));
    REQUIRE(uri2.toString().value_or("") == uriString);
  }

  SECTION("Fragment is move-assigned correctly") {
    UriWrapper uri2("http://example2.com");
    uri2 = std::move(uri);
    REQUIRE(uri2.toString().value_or("") == uriString);
  }
}

TEST_CASE("UriWrapper successfully copies and moves around", "[UriWrapper]") {
  UriWrapper uri4;
  {
    UriWrapper uri;
    {
      std::string uriString = "http://example.com:8080/path?query=1#fragment";
      UriWrapper uri2(uriString);
      uri = std::move(uri2);
    }
    UriWrapper uri2(uri);
    UriWrapper uri3(std::move(uri2));
    uri4 = std::move(uri3);
  }
  REQUIRE(uri4.toString().value_or("") ==
          "http://example.com:8080/path?query=1#fragment");
}

TEST_CASE("'/' in fragments are allowed") {
  UriWrapper uri("http://example.com:80/path?query=1#frag/ment");
  REQUIRE(uri.getFragment() == "frag/ment");
}

TEST_CASE("UriWrapper outputs correct toFragmentlessString()", "[UriWrapper]") {

  SECTION("Fragment is removed correctly") {
    UriWrapper uri("http://example.com:80/path?query=1#fragment");
    REQUIRE(uri.toFragmentlessString().value_or("") ==
            "http://example.com:80/path?query=1");
  }

  SECTION("URI without fragment is returned as-is") {
    UriWrapper uri("http://example.com:80/path?query=1");
    REQUIRE(uri.toFragmentlessString().value_or("") ==
            "http://example.com:80/path?query=1");
  }

  SECTION("URI with empty fragment is returned as-is") {
    UriWrapper uri("http://example.com:80/path?query=1#");
    REQUIRE(uri.toFragmentlessString().value_or("") ==
            "http://example.com:80/path?query=1");
  }
}

TEST_CASE("Normalization", "[UriWrapper]") {
  SECTION("Case normalization") {
    UriWrapper uri("data%3aTEXT%2Fhtml%3Bcharset%3dutf-8%2c%3chtml%3e");
    UriWrapper uri2("data%3ATEXT%2fhtml%3bcharset%3Dutf-8%2C%3Chtml%3E");
    uri.normalize();
    uri2.normalize();
    REQUIRE(uri.toString().value() == uri2.toString().value());
  }

  SECTION("Syntax normalization") {
    UriWrapper uri("example://a/b/c/%7Bfoo%7D");
    UriWrapper uri2("eXAMPLE://a/./b/../b/%63/%7bfoo%7d");
    uri.normalize();
    uri2.normalize();
    REQUIRE(uri.toString().value() == uri2.toString().value());
  }

  SECTION("Percent-encoding normalization") {
    UriWrapper uri("http://example.com/a%7Eb");
    UriWrapper uri2("http://example.com/a~b");
    uri.normalize();
    uri2.normalize();
    REQUIRE(uri.toString().value() == uri2.toString().value());
  }
}

TEST_CASE("Comparison with std::less", "[UriWrapper]") {
  UriWrapper uri1("http://example.com");
  UriWrapper uri2("http://example.com");
  UriWrapper uri3("http://example.com:80");
  UriWrapper uri4("http://example.com:80");

  REQUIRE_FALSE(std::less<UriWrapper>()(uri1, uri2));
  REQUIRE_FALSE(std::less<UriWrapper>()(uri2, uri1));
  REQUIRE_FALSE(std::less<UriWrapper>()(uri3, uri4));
  REQUIRE_FALSE(std::less<UriWrapper>()(uri4, uri3));

  REQUIRE(std::less<UriWrapper>()(uri1, uri3) !=
          std::less<UriWrapper>()(uri3, uri1));

  UriWrapper uri5("http://test.com");
  UriWrapper uri6("http://test.com#");
  REQUIRE_FALSE(std::less<UriWrapper>()(uri5, uri6));
  REQUIRE_FALSE(std::less<UriWrapper>()(uri6, uri5));
}

TEST_CASE("Host is case-insensitive", "[UriWrapper]") {
  UriWrapper uri1("http://example.com");
  UriWrapper uri2("http://EXAMPLE.com");
  REQUIRE(uri1 == uri2);
}

TEST_CASE("Inequality operator", "[UriWrapper]") {
  UriWrapper uri1("http://example.com/test1");
  UriWrapper uri2("http://example.com/test2");
  REQUIRE(uri1 != uri2);
}