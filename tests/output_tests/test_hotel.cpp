
#include "Hotel.h"
#include <catch2/catch_all.hpp>
TEST_CASE("Test hotel") {
  nlohmann::json json = R"({
    "id": "00000000-0000-0000-0000-000000000000",
    "name": "Hotel California",
    "address": {
      "street": "1234 Main St",
      "city": "Santa Monica",
      "state": "CA",
      "zip": "90405"
    },
    "rooms": [
      {
        "id": "00000000-0000-0000-0000-000000000001",
        "name": "Room 1",
        "price": 200,
        "beds": 2,
        "baths": 1,
        "sqft": 500
      },
      {
        "id": "00000000-0000-0000-0000-000000000002",
        "name": "Room 2",
        "price": 100,
        "beds": 1,
        "baths": 1,
        "sqft": 300
      }
    ]

})"_json;
  auto hotel = *JSOG::Hotel::construct(json);
  REQUIRE(*hotel.id_ == "00000000-0000-0000-0000-000000000000");
  REQUIRE(hotel.name_ == "Hotel California");
}