#include <catch2/catch_all.hpp>

Catch::Generators::GeneratorWrapper<std::string> randomString();

Catch::Generators::GeneratorWrapper<std::string>
randomString(size_t maxLength, size_t minLength = 0);