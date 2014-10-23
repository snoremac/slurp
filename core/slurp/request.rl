
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "slurp/request.h"

#define PARSE_BUFFER_SIZE 64
#define FRAME_BOUNDARY "~"
#define FIELD_DELIM ","

// 0x7d: }
// 0x7e: ~
// 0x5d: ]
// 0x5e: ^
// 0x2c: ,

%%{
  machine slurp;

  action start_frame {
    current_request = (struct slurp_request) { 0 };
  }

  action end_frame {
    request_callback(&current_request);
    fgoto main;
  }

  action start_field {
    field_start = p;
  }

  action end_escape {
    memmove(p - 1, p, pe - p);
    fhold;
    *pe = 0;
    *p |= 0x20;
    pe--;
  }

  action end_program {
    strlcpy(current_request.program, field_start, fmin((p - field_start) + 1, SLURP_REQUEST_PROGRAM_LENGTH));
    field_start = 0;
  }

  action end_request {
    strlcpy(current_request.request, field_start, fmin((p - field_start) + 1, SLURP_REQUEST_REQUEST_LENGTH));
    field_start = 0;
  }

  action end_arg_name {
    if (current_request.args_length < SLURP_REQUEST_MAX_ARGS) {
      strlcpy(
        current_request.arg_names[current_request.args_length],
        field_start,
        fmin((p - field_start) + 1, SLURP_REQUEST_ARG_NAME_LENGTH)
      );
    }
    field_start = 0;
  }

  action end_arg_value {
    if (current_request.args_length < SLURP_REQUEST_MAX_ARGS) {
      strlcpy(
        current_request.arg_values[current_request.args_length],
        field_start,
        fmin((p - field_start) + 1, SLURP_REQUEST_ARG_VALUE_LENGTH)
      );
      current_request.args_length++;
    }
    field_start = 0;
  }

  action start_frame_error {
    struct slurp_error error = (struct slurp_error) { .code = SLURP_ERROR_FRAMING };
    error_callback(&error);
    fgoto frame_error;
  }

  action end_frame_error {
    fhold;
    fgoto main;
  }

  action start_program_error {
    struct slurp_error error = (struct slurp_error) { .code = SLURP_ERROR_MISSING_PROGRAM };
    error_callback(&error);
    fgoto payload_error;
  }

  action start_request_error {
    struct slurp_error error = (struct slurp_error) { .code = SLURP_ERROR_MISSING_REQUEST };
    error_callback(&error);
    fgoto payload_error;
  }

  action start_arg_value_error {
    struct slurp_error error = (struct slurp_error) { .code = SLURP_ERROR_MISSING_ARG_VALUE };
    error_callback(&error);
    fgoto payload_error;
  }

  action end_payload_error {
    fhold;
    fgoto main;
  }

  frame_boundary = 0x7e;
  escape = 0x7d;
  escaped_boundary = escape 0x5e;
  escaped_escape = escape 0x5d;
  field_delim = 0x2c;

  byte = (any - (frame_boundary | escape | field_delim));
  field = (byte | escaped_boundary @end_escape | escaped_escape @end_escape)+;

  program = field >start_field %end_program;
  request = field >start_field %end_request;
  arg_name = field >start_field %end_arg_name;
  arg_value = field >start_field %end_arg_value;

  frame_error := (^frame_boundary)* frame_boundary @end_frame_error;
  payload_error := (^frame_boundary)* frame_boundary @end_payload_error;

  main :=
    frame_boundary @!start_frame_error >start_frame
    program @!start_program_error
    (field_delim request) @!start_request_error
    (field_delim (arg_name (field_delim arg_value) @!start_arg_value_error)?)*
    frame_boundary >end_frame
  ;
}%%

%% write data;

static struct slurp_request current_request;
static slurp_request_callback request_callback;
static slurp_error_callback error_callback;
static char* field_start;

static int8_t cs;
static char parse_buffer[PARSE_BUFFER_SIZE];
static uint8_t buffer_held;

static void serialize_payload(const struct slurp_request* request, struct serialize_buffer* buffer);
static void serialize_args(const struct slurp_request* request, struct serialize_buffer* buffer);
static void buffer_append(struct serialize_buffer* buffer, const char* append, uint8_t append_length);

void slurp_init_request_parser() {
  %% write init;
}

void slurp_parse_request(slurp_read_callback on_read, slurp_error_callback on_error) {
  error_callback = on_error;

  uint8_t buffer_remaining = PARSE_BUFFER_SIZE - buffer_held;
  if (buffer_remaining == 0) {
    return;
  }

  memset(parse_buffer + buffer_held, 0, buffer_remaining);
  char *p = parse_buffer + buffer_held;
  uint8_t bytes_read = on_read(p, buffer_remaining);

  // printf("bytes read: %u\n", bytes_read);
  // printf("parse buffer: %s\n", parse_buffer);
  // printf("p: %s\n", p);

  char *pe = p + bytes_read;
  if (bytes_read == 0) {
    return;
  }

  char *eof = NULL;

  %% write exec;

  if (field_start == 0) {
    buffer_held = 0;
  } else {
    buffer_held = pe - field_start;
    memmove(parse_buffer, field_start, buffer_held);
    field_start = parse_buffer;
  }

  return;
}

void slurp_on_request(slurp_request_callback on_request) {
  request_callback = on_request;
}

uint8_t slurp_serialize_request(const struct slurp_request* request, char* request_buffer, uint8_t request_buffer_length) {
  struct serialize_buffer buffer = {
    .buffer = request_buffer,
    .max_length = request_buffer_length,
    .current_length = 0
  };

  buffer_append(&buffer, FRAME_BOUNDARY, strlen(FRAME_BOUNDARY));
  serialize_payload(request, &buffer);
  buffer_append(&buffer, FRAME_BOUNDARY, strlen(FRAME_BOUNDARY));
  return buffer.truncated ? 0 : buffer.current_length;
}

static void serialize_payload(const struct slurp_request* request, struct serialize_buffer* buffer) {
  buffer_append(buffer, request->program, strlen(request->program));
  buffer_append(buffer, FIELD_DELIM, strlen(FIELD_DELIM));
  buffer_append(buffer, request->request, strlen(request->request));
  serialize_args(request, buffer);
}

static void serialize_args(const struct slurp_request* request, struct serialize_buffer* buffer) {
  for (uint8_t i = 0; i < request->args_length; i++) {
    buffer_append(buffer, FIELD_DELIM, strlen(FIELD_DELIM));
    buffer_append(buffer, request->arg_names[i], strlen(request->arg_names[i]));
    buffer_append(buffer, FIELD_DELIM, strlen(FIELD_DELIM));
    buffer_append(buffer, request->arg_values[i], SLURP_REQUEST_ARG_VALUE_LENGTH);
  }
}

static void buffer_append(struct serialize_buffer* buffer, const char* append, uint8_t append_length) {
  uint8_t checked_append_length = fmin(append_length, buffer->max_length - buffer->current_length);
  memcpy(buffer->buffer + buffer->current_length, append, checked_append_length);
  buffer->current_length += checked_append_length;
  buffer->truncated = checked_append_length < append_length;
}
