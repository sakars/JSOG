#ifndef DRAFT07_H
#define DRAFT07_H

#include "JSONPointer.h"
#include "UriWrapper.h"
#include <nlohmann/json.hpp>
#include <set>

std::set<UriWrapper> getDraft07Dependencies(const nlohmann::json&,
                                            const UriWrapper&,
                                            const JSONPointer&);

#endif // DRAFT07_H
