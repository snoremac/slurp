#ifndef SLURP_SLURP_H
#define SLURP_SLURP_H

#include <stdint.h>

#define SLURP_REQUEST_PROGRAM_LENGTH 8
#define SLURP_REQUEST_REQUEST_LENGTH 8
#define SLURP_REQUEST_ARG_NAME_LENGTH 8
#define SLURP_REQUEST_ARG_VALUE_LENGTH 4
#define SLURP_REQUEST_MAX_ARGS 4

#define SLURP_ERROR_FRAMING 1
#define SLURP_ERROR_MISSING_PROGRAM 2
#define SLURP_ERROR_MISSING_REQUEST 3
#define SLURP_ERROR_MISSING_ARG_VALUE 4

struct slurp_request {
  char program[SLURP_REQUEST_PROGRAM_LENGTH];
  char request[SLURP_REQUEST_REQUEST_LENGTH];
  char arg_names[SLURP_REQUEST_MAX_ARGS][SLURP_REQUEST_ARG_NAME_LENGTH];
  char arg_values[SLURP_REQUEST_MAX_ARGS][SLURP_REQUEST_ARG_VALUE_LENGTH];
  uint8_t args_length;
};

struct slurp_error {
  uint8_t code;
};

typedef uint8_t(*slurp_read_callback)(char* dest, uint8_t max_read);

typedef void(*slurp_request_callback)(struct slurp_request* request);

typedef void(*slurp_error_callback)(struct slurp_error* error);

#endif
