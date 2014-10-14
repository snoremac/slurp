#ifndef SLURP_SLURP_H
#define SLURP_SLURP_H

#include <stdint.h>

#define SLURP_ERROR_FRAMING 1
#define SLURP_ERROR_MISSING_PROGRAM 2
#define SLURP_ERROR_MISSING_REQUEST 3

struct slurp_request {
  char program[8];
  char request[8];
  // char arg_names[4][8];
  // char arg_values[4][4];
  // uint8_t args_length;
};

struct slurp_error {
  uint8_t code;
};

typedef uint8_t(*slurp_read_callback)(char* dest, uint8_t max_read);

typedef void(*slurp_request_callback)(struct slurp_request* request);

typedef void(*slurp_error_callback)(struct slurp_error* error);

#endif
