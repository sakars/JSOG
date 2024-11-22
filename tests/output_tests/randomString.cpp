#include "randomString.h"
#include <random>

struct StringGenerator : Catch::Generators::IGenerator<std::string> {
  std::string currentValue;
  std::minstd_rand m_rand;
  std::uniform_int_distribution<> m_char;
  std::uniform_int_distribution<> m_length;

  StringGenerator()
      : m_rand(std::random_device()()), m_char(0, 255), m_length(0, 200) {
    next();
  }

  StringGenerator(size_t maxLength, size_t minLength = 0)
      : m_rand(std::random_device()()), m_char(0, 255),
        m_length(minLength, maxLength) {
    next();
  }

  std::string const& get() const override { return currentValue; }
  bool next() override {
    const auto strLength = m_length(m_rand);
    std::string str;
    str.reserve(strLength);
    for (size_t i = 0; i < strLength; ++i) {
      str.push_back(static_cast<char>(m_char(m_rand)));
    }
    currentValue = str;
    return true;
  }
};

Catch::Generators::GeneratorWrapper<std::string> randomString() {
  return Catch::Generators::GeneratorWrapper<std::string>(
      Catch::Detail::make_unique<StringGenerator>());
}

Catch::Generators::GeneratorWrapper<std::string>
randomString(size_t maxLength, size_t minLength) {
  return Catch::Generators::GeneratorWrapper<std::string>(
      Catch::Detail::make_unique<StringGenerator>(maxLength, minLength));
}
