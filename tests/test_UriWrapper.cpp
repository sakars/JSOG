
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

TEST_CASE("UriWrapper escapes fragments correctly", "[UriWrapper]") {

  SECTION("Fragment with a # is escaped correctly") {

    UriWrapper uri("http://example.com:80/path?query=1");
    uri.setFragment("frag#ment");

    SECTION("toString() returns the correct URI") {
      REQUIRE(uri.toString().value_or("") ==
              "http://example.com:80/path?query=1#frag%23ment");
    }

    SECTION("Fragment is parsed correctly") {
      REQUIRE(uri.getFragment() == "frag#ment");
    }
  }

  SECTION("Fragment with a % is not escaped") {

    UriWrapper uri("http://example.com:80/path?query=1");
    uri.setFragment("frag%ment");

    SECTION("toString() returns the correct URI") {
      REQUIRE(uri.toString().value_or("") ==
              "http://example.com:80/path?query=1#frag%25ment");
    }

    SECTION("Fragment is parsed correctly") {
      REQUIRE(uri.getFragment() == "frag%ment");
    }
  }
}

TEST_CASE("'/' in fragments are allowed") {
  UriWrapper uri("http://example.com:80/path?query=1#frag/ment");
  REQUIRE(uri.getFragment() == "frag/ment");
}
