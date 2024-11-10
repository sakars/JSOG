#include <catch2/catch_all.hpp>

#include "CodeBlock.h"

TEST_CASE("CodeBlock")
{
  CodeBlock block;
  block << "Hello, World!"
        << CodeBlock::inc
        << "Goodbye, World!"
        << CodeBlock::dec
        << "Hello, World!";
  REQUIRE(block.str() == "Hello, World!\n  Goodbye, World!\nHello, World!\n");
}

TEST_CASE("CodeBlock with custom indent")
{
  CodeBlock block("    ");
  block << "Hello, World!"
        << CodeBlock::inc
        << "Goodbye, World!"
        << CodeBlock::inc
        << "Goodbye, World!"
        << CodeBlock::dec
        << "Hello, World!";
  REQUIRE(block.str() == "Hello, World!\n    Goodbye, World!\n        Goodbye, World!\n    Hello, World!\n");
}
