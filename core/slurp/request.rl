
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "slurp/request.h"

#define PARSE_BUFFER_SIZE 64
#define SCANNER_ERROR -1

// 0x7d: }
// 0x7e: ~
// 0x5d: ]
// 0x5e: ^

%%{
  machine slurp;

  action start_frame {
    current_request = (struct slurp_request) { 0 };
  }

  action end_frame {
    request_callback(current_request);
    fgoto main;
  }

  action start_field {
    field_start = p;
  }

  action end_program {
    strlcpy(current_request.program, field_start, fmin((p - field_start) + 1, sizeof(current_request.program)));
  }

  action end_request {
    strlcpy(current_request.request, field_start, fmin((p - field_start) + 1, sizeof(current_request.request)));
  }

  action start_frame_error {
    field_start = p;
    fgoto frame_error;
  }

  action end_frame_error {
    field_start = p;
    fhold;
    fgoto main;
  }

  frame_boundary = 0x7e;
  escape = 0x7d;
  escaped_boundary = escape 0x5e;
  escaped_escape = escape 0x5d;

  field_delim = ",";
  byte = (any - (frame_boundary | escape | field_delim));
  #field = (byte | escaped_boundary | escaped_escape)+;
  field = (byte)+;

  frame_error := (!frame_boundary)* frame_boundary @end_frame_error;

  main :=
    frame_boundary @!start_frame_error >start_frame
    field >start_field %end_program
    field_delim field >start_field %end_request
    frame_boundary >end_frame
  ;
}%%

%% write data;

    // char match[(te - ts) + 1];
    // strlcpy(match, ts, (te - ts) + 1);



  // payload := |*
  //   (byte)+ => field;
  // *|;

    // field_delim => { fret; };

//   main :=
//     frame_boundary
//     @{ fcall payload; }
//     frame_boundary
//   ;

static struct slurp_request current_request;
static slurp_request_callback request_callback;
static char* field_start;

static int8_t cs;
// static uint8_t stack[16];
// static uint8_t top;
// static uint8_t act;
// static char* ts;
// static char* te;

static char parse_buffer[PARSE_BUFFER_SIZE];
//static uint8_t buffer_used;

void slurp_init_request_parser() {
  %% write init;
}

uint8_t slurp_parse_request(slurp_read_callback read_callback) {
  char *p = parse_buffer;
  uint8_t bytes_read = read_callback(p, PARSE_BUFFER_SIZE);

  // printf("Bytes read: %u\n", bytes_read);
  // printf("p: %s\n", p);

  char *pe = p + bytes_read;
  if (bytes_read == 0) {
    return 0;
  }

  //char *eof = bytes_read == SLURP_EOF ? pe : NULL;
  char *eof = NULL;

  %% write exec;

  return 0;
}

// uint8_t slurp_parse_request(slurp_read_callback read_callback) {
//   uint8_t buffer_remaining = PARSE_BUFFER_SIZE - buffer_used;
//   if (buffer_remaining == 0) {
//     return 1;
//   }
//
//   char *p = parse_buffer + buffer_used;
//
//   // char read_buffer[buffer_remaining];
//   // uint8_t bytes_read = read_callback(read_buffer, buffer_remaining);
//   uint8_t bytes_read = read_callback(p, buffer_remaining);
//
//   printf("Bytes read: %u\n", bytes_read);
//   printf("p: %s\n", p);
//
//   char *pe = p + bytes_read;
//   if (bytes_read == 0 ) {
//     return 0;
//   }
//
//   char *eof = NULL;
//
//   %% write exec;
//
//   if (cs == SCANNER_ERROR) {
//     return 1;
//   }
//
//   if (ts == 0) {
//     buffer_used = 0;
//   } else {
//     buffer_used = pe - ts;
//     memmove(parse_buffer, ts, buffer_used);
//     te = parse_buffer + (te - ts);
//     ts = parse_buffer;
//   }
//
//   return 0;
// }

void slurp_on_request(slurp_request_callback callback) {
  request_callback = callback;
}
