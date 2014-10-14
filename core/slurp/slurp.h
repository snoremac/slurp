#ifndef SLURP_SLURP_H
#define SLURP_SLURP_H

struct slurp_request {
  char program[8];
  char request[8];
  // char arg_names[4][8];
  // char arg_values[4][4];
  // uint8_t args_length;
};

typedef uint8_t (*slurp_read_callback)(char* dest, uint8_t max_read);

typedef void(*slurp_request_callback)(struct slurp_request* request);

#endif
