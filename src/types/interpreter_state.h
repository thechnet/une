/*
interpreter_state.h - Une
Modified 2022-09-26
*/

#ifndef UNE_INTERPRETER_STATE_H
#define UNE_INTERPRETER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "context.h"

/*
Holds the state of the interpreter.
*/
typedef struct une_interpreter_state_ {
  une_context *context;
  bool should_return;
  bool should_exit;
  une_function *functions;
  size_t functions_size;
  size_t functions_count;
} une_interpreter_state;

/*
*** Interface.
*/

une_interpreter_state une_interpreter_state_create(void);
void une_interpreter_state_free(une_interpreter_state *is);

size_t une_function_create(une_interpreter_state *is, char *definition_file, une_position definition_point);

#endif /* UNE_INTERPRETER_STATE_H */
