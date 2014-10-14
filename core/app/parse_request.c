
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "slurp/request.h"

static void request_callback(struct slurp_request* request);
static uint8_t read_callback(char* dest, uint8_t max_read);
static void error_callback(struct slurp_error* error);

static char input[64];
static uint8_t current_index;

int main(int argc, char **argv) {
  slurp_init_request_parser();
  slurp_on_request(request_callback);
  for (uint8_t i = 1; i < argc; i++) {
    strncpy(input, argv[i], sizeof(input));
    current_index = 0;
    slurp_parse_request(read_callback, error_callback);
  }
}

static void request_callback(struct slurp_request* request) {
  printf("Request: %s.%s\n", request->program, request->request);
}

static uint8_t read_callback(char* dest, uint8_t max_read) {
  uint8_t i = 0;
  while (i + current_index < strlen(input) && i < max_read) {
    dest[i] = input[i + current_index];
    i++;
  }
  current_index += i;
  return i;
}

static void error_callback(struct slurp_error* error) {
  printf("Parser returned error: %u\n", error->code);
}
