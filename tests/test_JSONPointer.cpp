#include "JSONPointer.h"
#include <catch2/catch_all.hpp>

TEST_CASE("JSONPointer correctly parses fragments", "[JSONPointer]") {
  SECTION("Empty fragment") {
    JSONPointer pointer = JSONPointer::fromJSONString("");
    REQUIRE(pointer.toString() == "");
    REQUIRE(pointer.toFragment() == "#");
  }

  SECTION("Single token fragment") {
    JSONPointer pointer = JSONPointer::fromJSONString("/token");
    REQUIRE(pointer.toString() == "/token");
    REQUIRE(pointer.toFragment() == "#/token");
  }

  SECTION("Multiple token fragment") {
    JSONPointer pointer = JSONPointer::fromJSONString("token1/token2");
    REQUIRE(pointer.toString() == "token1/token2");
    REQUIRE(pointer.toFragment() == "#token1/token2");
  }
}

TEST_CASE("JSONPointer correctly adds and removes tokens", "[JSONPointer]") {
  JSONPointer pointer;
  SECTION("Adding tokens") {
    pointer.add("token1");
    REQUIRE(pointer.toString() == "/token1");
    REQUIRE(pointer.toFragment() == "#/token1");
    pointer.add("token2");
    REQUIRE(pointer.toString() == "/token1/token2");
    REQUIRE(pointer.toFragment() == "#/token1/token2");
  }

  SECTION("Adding index") {
    pointer.add(1);
    REQUIRE(pointer.toString() == "/1");
    REQUIRE(pointer.toFragment() == "#/1");
  }

  SECTION("Removing tokens") {
    pointer.add("token1");
    pointer.add("token2");
    pointer.up();
    REQUIRE(pointer.toString() == "/token1");
    REQUIRE(pointer.toFragment() == "#/token1");
  }

  SECTION("Removing tokens from empty pointer") {
    pointer.up();
    REQUIRE(pointer.toString() == "");
    REQUIRE(pointer.toFragment() == "#");
  }
}

TEST_CASE("JSONPointer correctly navigates a json", "[JSONPointer]") {
  nlohmann::json json = R"(
    {
      "foo": ["bar", "baz"],
      "": 0,
      "a/b": 1,
      "c%d": 2,
      "e^f": 3,
      "g|h": 4,
      "i\\j": 5,
      "k\"l": 6,
      " ": 7,
      "m~n": 8
   }
  )"_json;
  SECTION("JSON string") {
    REQUIRE(&json == &JSONPointer::fromJSONString("").navigate(json));
    REQUIRE(&json["foo"] ==
            &JSONPointer::fromJSONString("/foo").navigate(json));
    REQUIRE(&json["foo"][0] ==
            &JSONPointer::fromJSONString("/foo/0").navigate(json));
    REQUIRE(&json["foo"][1] ==
            &JSONPointer::fromJSONString("/foo/1").navigate(json));
    REQUIRE(&json[""] == &JSONPointer::fromJSONString("/").navigate(json));
    REQUIRE(&json["a/b"] ==
            &JSONPointer::fromJSONString("/a~1b").navigate(json));
    REQUIRE(&json["c%d"] ==
            &JSONPointer::fromJSONString("/c%d").navigate(json));
    REQUIRE(&json["e^f"] ==
            &JSONPointer::fromJSONString("/e^f").navigate(json));
    REQUIRE(&json["g|h"] ==
            &JSONPointer::fromJSONString("/g|h").navigate(json));
    REQUIRE(&json["i\\j"] ==
            &JSONPointer::fromJSONString("/i\\j").navigate(json));
    REQUIRE(&json["k\"l"] ==
            &JSONPointer::fromJSONString("/k\"l").navigate(json));
    REQUIRE(&json[" "] == &JSONPointer::fromJSONString("/ ").navigate(json));
    REQUIRE(&json["m~n"] ==
            &JSONPointer::fromJSONString("/m~0n").navigate(json));
  }

  SECTION("URI string") {
    REQUIRE(&json == &JSONPointer::fromURIString("").navigate(json));
    REQUIRE(&json["foo"] ==
            &JSONPointer::fromURIString("#/foo").navigate(json));
    REQUIRE(&json["foo"][0] ==
            &JSONPointer::fromURIString("#/foo/0").navigate(json));
    REQUIRE(&json["foo"][1] ==
            &JSONPointer::fromURIString("#/foo/1").navigate(json));
    REQUIRE(&json[""] == &JSONPointer::fromURIString("#/").navigate(json));
    REQUIRE(&json["a/b"] ==
            &JSONPointer::fromURIString("#/a~1b").navigate(json));
    REQUIRE(&json["c%d"] ==
            &JSONPointer::fromURIString("#/c%25d").navigate(json));
    REQUIRE(&json["e^f"] ==
            &JSONPointer::fromURIString("#/e%5Ef").navigate(json));
    REQUIRE(&json["g|h"] ==
            &JSONPointer::fromURIString("#/g%7Ch").navigate(json));
    REQUIRE(&json["i\\j"] ==
            &JSONPointer::fromURIString("#/i%5Cj").navigate(json));
    REQUIRE(&json["k\"l"] ==
            &JSONPointer::fromURIString("#/k%22l").navigate(json));
    REQUIRE(&json[" "] == &JSONPointer::fromURIString("#/%20").navigate(json));
    REQUIRE(&json["m~n"] ==
            &JSONPointer::fromURIString("#/m~0n").navigate(json));
  }
}