/*
interpreter_state.h - Une
Modified 2021-06-04
*/

#ifndef UNE_INTERPRETER_STATE_H
#define UNE_INTERPRETER_STATE_H

#include "../primitive.h"
#include "context.h"

#pragma region une_interpreter_state
typedef struct _une_interpreter_state {
  une_context *context;
  bool should_return;
} une_interpreter_state;
#pragma endregion une_interpreter_state

une_interpreter_state une_interpreter_state_create(une_context *context);
void une_interpreter_state_free(une_interpreter_state is);

#endif /* UNE_INTERPRETER_STATE_H */
