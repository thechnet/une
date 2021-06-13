/*
interpreter_state.h - Une
Modified 2021-06-09
*/

#ifndef UNE_INTERPRETER_STATE_H
#define UNE_INTERPRETER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "context.h"

/*
Holds the state of the interpreter.
*/
typedef struct _une_interpreter_state {
  une_context *context;
  bool should_return;
} une_interpreter_state;

/*
*** Interface.
*/

une_interpreter_state une_interpreter_state_create(une_context *context);

#endif /* UNE_INTERPRETER_STATE_H */
