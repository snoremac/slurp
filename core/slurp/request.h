#ifndef SLURP_REQUEST_H
#define SLURP_REQUEST_H

#include <stdint.h>

#include "slurp/slurp.h"

void slurp_init_request_parser(void);

void slurp_parse_request(slurp_read_callback on_read, slurp_error_callback on_error);

void slurp_on_request(slurp_request_callback on_request);

#endif
