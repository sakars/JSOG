#ifndef DRAFT07_H
#define DRAFT07_H
#include "Schema.h"
class Draft07 : public Schema {
public:
  Draft07(std::unique_ptr<LinkedSchema>&& schema, const std::string& identifier)
      : Schema(std::move(schema), identifier) {}
};

#endif // DRAFT07_H