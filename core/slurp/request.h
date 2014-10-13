#ifndef SLURP_REQUEST_H
#define SLURP_REQUEST_H

#include <stdint.h>

#include "slurp/slurp.h"

void slurp_init_request_parser(void);

uint8_t slurp_parse_request(slurp_read_callback read_callback);

void slurp_on_request(slurp_request_callback request_callback);

#endif
