/*
instance.h - Une
Modified 2021-05-22
*/

#ifndef UNE_INSTANCE_H
#define UNE_INSTANCE_H

#include "error.h"
#include "lexer_state.h"
#include "parser_state.h"
#include "interpreter_state.h"

#pragma region une_instance
typedef struct _une_instance {
  une_error error;
  une_lexer_state ls;
  une_parser_state ps;
  une_interpreter_state is;
} une_instance;
#pragma endregion une_instance

une_instance une_instance_create(void);
void une_instance_free(une_instance instance);

#endif /* UNE_INSTANCE_H */
