#include <string>

class NullProperties {
public:
  std::string getNullType() const { return "std::monostate"; }
};