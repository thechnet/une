/*
interpreter_state.h - Une
Modified 2023-02-22
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
  une_result this_contestant;
  une_result this;
} une_interpreter_state;

/*
*** Interface.
*/

une_interpreter_state une_interpreter_state_create(void);
void une_interpreter_state_free(une_interpreter_state *is);

#endif /* UNE_INTERPRETER_STATE_H */
