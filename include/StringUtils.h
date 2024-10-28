#include <string>

std::string normalizeString(std::string s) {
  std::string outStr = "_";
  for (const auto &c : s) {
    if (c == ' ') {
      outStr += "_s_";
    } else if (c == '_') {
      outStr += "__";
    } else if (!((c <= '9' && c >= '0') || (c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z'))) {
      outStr += "_d" + std::to_string((unsigned char)c) + "_";
    } else {
      outStr += c;
    }
  }
  return outStr;
}