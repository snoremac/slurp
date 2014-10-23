#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "unity.h"

#include "slurp/request.h"

const struct slurp_request basic_request = { .program = "chaser", .request = "start" };
const char* basic_request_str = "~chaser,start~";

const struct slurp_request geo_request = {
  .program = "geo", .request = "map",
  .arg_names = { "lat", "lon", "zoom" },
  .arg_values = { "\x00\x00\x00x", "\xff\xff\xff\xe2", "\x00\x00\x00\x03" },
  .args_length = 3
};
const char* geo_request_str = "~geo,map,lat,\x00\x00\x00x,lon,\xff\xff\xff\xe2,zoom,\x00\x00\x00\x03~";

void setUp() {
}

void tearDown() {
}

void test_serialize_request_no_args() {
  char buffer[SLURP_REQUEST_MAX_LENGTH];
  uint8_t length = slurp_serialize_request(&basic_request, buffer, SLURP_REQUEST_MAX_LENGTH);
  TEST_ASSERT_EQUAL_MEMORY(basic_request_str, buffer, length);
}

void test_serialize_request_with_args() {
  char buffer[SLURP_REQUEST_MAX_LENGTH];
  uint8_t length = slurp_serialize_request(&geo_request, buffer, SLURP_REQUEST_MAX_LENGTH);
  TEST_ASSERT_EQUAL_MEMORY(geo_request_str, buffer, length);
}

void test_detect_truncation() {
  char buffer[SLURP_REQUEST_MAX_LENGTH];
  uint8_t length = slurp_serialize_request(&basic_request, buffer, SLURP_REQUEST_MAX_LENGTH);
  TEST_ASSERT_TRUE(length > 0);

  char short_buffer[8];
  length = slurp_serialize_request(&basic_request, short_buffer, 8);
  TEST_ASSERT_EQUAL_UINT(0, length);
}
