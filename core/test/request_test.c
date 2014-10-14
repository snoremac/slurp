#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "unity.h"

#include "slurp/request.h"

#define MAX_REQUESTS 3
#define MAX_ERRORS 3
#define REQUEST_INPUT_SIZE 64

static void parse(const char* input);
static uint8_t read_callback(char* dest, uint8_t max_read);
static void request_callback(struct slurp_request* request);
static void error_callback(struct slurp_error* error);

static char request_input[REQUEST_INPUT_SIZE];
static int8_t request_input_index;

static struct slurp_request requests[MAX_REQUESTS];
static struct slurp_error errors[MAX_ERRORS];
static uint8_t request_count;
static uint8_t error_count;

void setUp() {
  slurp_init_request_parser();
  slurp_on_request(request_callback);

  request_input_index = 0;

  memset(requests, 0, sizeof(struct slurp_request) * MAX_REQUESTS);
  memset(errors, 0, sizeof(struct slurp_error) * MAX_ERRORS);
  request_count = 0;
  error_count = 0;
}

void tearDown() {
}

void test_request_without_args(){
  parse("~chaser,start~");
  TEST_ASSERT_EQUAL_UINT(1, request_count);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
}

void test_consecutive_requests_no_args(){
  parse("~chaser,start~~chaser,stop~");
  TEST_ASSERT_EQUAL_UINT(2, request_count);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[1].program);
  TEST_ASSERT_EQUAL_STRING("stop", requests[1].request);
}

void test_data_outside_frame(){
  parse("data~");
  TEST_ASSERT_EQUAL_UINT(0, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_FRAMING, errors[0].code);
}

void test_recovery_from_data_outside_frame(){
  parse("data~chaser,start~");
  TEST_ASSERT_EQUAL_UINT(1, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_FRAMING, errors[0].code);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
}

void test_missing_program(){
  parse("~~");
  TEST_ASSERT_EQUAL_UINT(0, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_MISSING_PROGRAM, errors[0].code);
}

void test_recovery_from_missing_program(){
  parse("~~~chaser,start~");
  TEST_ASSERT_EQUAL_UINT(1, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_MISSING_PROGRAM, errors[0].code);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
}

void test_missing_request_with_delim(){
  parse("~chaser,~");
  TEST_ASSERT_EQUAL_UINT(0, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_MISSING_REQUEST, errors[0].code);
}

void test_missing_request_no_delim(){
  parse("~chaser~");
  TEST_ASSERT_EQUAL_UINT(0, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_MISSING_REQUEST, errors[0].code);
}

void test_recovery_from_missing_request(){
  parse("~chaser~~chaser,start~");
  TEST_ASSERT_EQUAL_UINT(1, request_count);
  TEST_ASSERT_EQUAL_UINT(1, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_MISSING_REQUEST, errors[0].code);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
}

void test_consecutive_requests_and_errors(){
  parse("data~chaser,start~~chaser,~~chaser,stop~data");
  TEST_ASSERT_EQUAL_UINT(2, request_count);
  TEST_ASSERT_EQUAL_UINT(3, error_count);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_FRAMING, errors[0].code);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_MISSING_REQUEST, errors[1].code);
  TEST_ASSERT_EQUAL_UINT(SLURP_ERROR_FRAMING, errors[2].code);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[0].program);
  TEST_ASSERT_EQUAL_STRING("start", requests[0].request);
  TEST_ASSERT_EQUAL_STRING("chaser", requests[1].program);
  TEST_ASSERT_EQUAL_STRING("stop", requests[1].request);
}

static void parse(const char* input) {
  strlcpy(request_input, input, fmin(strlen(input) + 1, sizeof(request_input)));
  slurp_parse_request(read_callback, error_callback);
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

static void request_callback(struct slurp_request* request) {
  if (request_count < MAX_REQUESTS) {
    requests[request_count++] = *request;
  }
}

static void error_callback(struct slurp_error* error) {
  if (error_count < MAX_ERRORS) {
    errors[error_count++] = *error;
  }
}
