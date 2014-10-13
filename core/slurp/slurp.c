
#include <stdint.h>

#include "avrlaunch/buffer/buffer.h"
#include "avrlaunch/scheduler.h"
#include "avrlaunch/hal/hal_slurp.h"
#include "slurp/slurp.h"

// #define PARSE_BUFFER_SIZE = 64
//
// static uint8_t _slurp_task_id;
// static struct slurp_parse_context _parse_context
// static struct buffer _parse_buffer;
// static uint8_t _parse_buffer_data[PARSE_BUFFER_SIZE];
//
// static void slurp_task(struct task* task);
//
// void slurp_init_server(slurp_request_callback request_callback) {
//   slurp_request_init(request_callback);
//
//   _parse_buffer = buffer_init(_parse_buffer_data, PARSE_BUFFER_SIZE, sizeof(uint8_t));
//   _slurp_context = {
//     .input_buffer = get_uart_buffer(),
//     .parse_buffer = &parse_buffer,
//     .state = SLURP_STATE_IDLE
//   }
//   _slurp_task_id = scheduler_add_task(&(struct task_config){"slurp", TASK_FOREVER, TASK_ASAP}, slurp_task, &_parse_context);
// }
//
// static void slurp_task(struct task* task) {
//   struct slurp_parse_context* parse_context = (slurp_parse_context*) task->data;
// }
