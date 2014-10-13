#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "unity.h"

#include "slurp/request.h"

#define MAX_REQUESTS 3
#define REQUEST_INPUT_SIZE 64

static void parse(const char* input);
static uint8_t read_callback(char* dest, uint8_t max_read);
static void request_callback(struct slurp_request request);

static char request_input[REQUEST_INPUT_SIZE];
static int8_t request_input_index;

static struct slurp_request requests[MAX_REQUESTS];
static uint8_t request_count;

void setUp() {
  slurp_init_request_parser();
  slurp_on_request(request_callback);

  request_input_index = 0;

  memset(requests, 0, sizeof(struct slurp_request) * MAX_REQUESTS);
  request_count = 0;
}

void tearDown() {
}

void test_empty_command(){
  parse("~~");
  TEST_ASSERT_EQUAL_UINT(0, request_count);
}

void test_program_only(){
  parse("~chaser~");
  TEST_ASSERT_EQUAL_UINT(0, request_count);
}

void test_basic_command(){
  parse("~chaser,start~");
  TEST_ASSERT_EQUAL_UINT(1, request_count);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
}

void test_consecutive_basic_commands(){
  parse("~chaser,start~~chaser,stop~");
  TEST_ASSERT_EQUAL_UINT(2, request_count);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[1].program);
  TEST_ASSERT_EQUAL_STRING("stop", requests[1].request);
}

static void parse(const char* input) {
  strlcpy(request_input, input, fmin(strlen(input) + 1, sizeof(request_input)));
  uint8_t result = slurp_parse_request(read_callback);
  if (result > 0) {
    TEST_FAIL_MESSAGE("Parser returned error");
  }
}

static uint8_t read_callback(char* dest, uint8_t max_read) {
  uint8_t i = 0;
  while (i + request_input_index < strlen(request_input) && i < max_read) {
    dest[i] = request_input[i + request_input_index];
    i++;
  }
  request_input_index += i;
  return i;
}

static void request_callback(struct slurp_request request) {
  if (request_count < MAX_REQUESTS) {
    requests[request_count++] = request;
  }
}
