#include <catch2/catch_all.hpp>

#include "CodeBlock.h"

TEST_CASE("CodeBlock base test", "[CodeBlock]") {
  CodeBlock block;
  block << "Hello, World!" << CodeBlock::inc << "Goodbye, World!"
        << CodeBlock::dec << "Hello, World!";
  REQUIRE(block.str() == "Hello, World!\n  Goodbye, World!\nHello, World!\n");
}

TEST_CASE("CodeBlock with custom indent", "[CodeBlock]") {
  CodeBlock block("    ");
  block << "Hello, World!" << CodeBlock::inc << "Goodbye, World!"
        << CodeBlock::inc << "Goodbye, World!" << CodeBlock::dec
        << "Hello, World!";
  REQUIRE(block.str() == "Hello, World!\n    Goodbye, World!\n        Goodbye, "
                         "World!\n    Hello, World!\n");
}

TEST_CASE("CodeBlock with discard", "[CodeBlock]") {
  CodeBlock block;
  block.indent = "x";
  block << "Hello, World!" << CodeBlock::inc << "Goodbye, World!"
        << CodeBlock::dis << "Hello, World!";
  REQUIRE(block.str() == "Hello, World!\nxGoodbye, World!Hello, World!\n");
}

TEST_CASE("CodeBlock with multiple discards", "[CodeBlock]") {
  CodeBlock block;
  block.indent = "x";
  block << "Hello, World!" << CodeBlock::inc << "Goodbye, World!"
        << CodeBlock::dis << "Hello, World!" << CodeBlock::dis
        << "Goodbye World!";
  REQUIRE(block.str() == "Hello, World!\nxGoodbye, World!Hello, World!Goodbye "
                         "World!\n");
}

TEST_CASE("Indent blocks function as expected", "[CodeBlock]") {
  CodeBlock block;
  block.indent = "  ";
  block << "Hello, World!";
  {
    Indent indent(block);
    block << "Goodbye, World!";
    {
      Indent indent(block);
      block << "Goodbye, World!";
    }
    block << "Hello, World!";
  }
  REQUIRE(block.str() == "Hello, World!\n  Goodbye, World!\n    Goodbye, "
                         "World!\n  Hello, World!\n");
}
