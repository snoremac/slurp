
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "slurp/request.h"

static void request_callback(struct slurp_request request);
static uint8_t read_callback(char* dest, uint8_t max_read);

static char input[64];
static uint8_t current_index;

int main(int argc, char **argv) {
  slurp_init_request_parser();
  slurp_on_request(request_callback);
  if (argc > 1) {
    strncpy(input, argv[1], strlen(argv[1]));
    uint8_t result = slurp_parse_request(read_callback);
    if (result > 0) {
      printf("Parse returned error: %u\n", result);
    }
  }
}

static void request_callback(struct slurp_request request) {
  printf("Request: %s.%s\n", request.program, request.request);
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
