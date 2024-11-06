#ifndef DRAFT07_H
#define DRAFT07_H
#include "Schema.h"
class Draft07 : public Schema {
public:
  Draft07(const BuildableSchema&, const std::string& identifier)
      : Schema(identifier) {}
};

#endif // DRAFT07_H