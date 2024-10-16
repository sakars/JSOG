#include "UriWrapper.h"
// #include <iostream>
// #include <nlohmann/json.hpp>
// #include "ResourceMap.h"
#include <iostream>

int main() {
  UriWrapper uri("http://example.com");
  auto uri2 = uri;
  auto uri3 = uri2;
  return 0;
}