#ifndef SYNCEDSCHEMA_STRINGPROPERTIES_H
#define SYNCEDSCHEMA_STRINGPROPERTIES_H

#include <optional>
#include <string>

struct StringProperties {
  std::optional<size_t> maxLength_;
  std::optional<size_t> minLength_;
  std::optional<std::string> pattern_;

  std::string getStringType() const;
};

#endif // SYNCEDSCHEMA_STRINGPROPERTIES_H