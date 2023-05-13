/*
interpreter_state.h - Une
Modified 2023-05-13
*/

#ifndef UNE_INTERPRETER_STATE_H
#define UNE_INTERPRETER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "context.h"

/*
A temporary result buffer.
*/
typedef struct une_holding_ {
  une_result *buffer;
  size_t size;
  size_t count;
} une_holding;

/*
Holds the state of the interpreter.
*/
typedef struct une_interpreter_state_ {
  une_context *context;
  bool should_return;
  bool should_exit;
  une_result this_contestant;
  une_result this;
  une_holding holding;
} une_interpreter_state;

/*
*** Interface.
*/

une_interpreter_state une_interpreter_state_create(void);
void une_interpreter_state_free(une_interpreter_state *is);
une_holding une_interpreter_state_holding_strip(une_interpreter_state *is);
void une_interpreter_state_holding_reinstate(une_interpreter_state *is, une_holding old);
une_result *une_interpreter_state_holding_add(une_interpreter_state *is, une_result result);
void une_interpreter_state_holding_purge(une_interpreter_state *is);

#endif /* UNE_INTERPRETER_STATE_H */
