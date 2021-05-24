/*
parser_state.h - Une
Modified 2021-05-24
*/

#ifndef UNE_PARSER_STATE_H
#define UNE_PARSER_STATE_H

#include "../primitive.h"
#include "token.h"

#pragma region une_parser_state
typedef struct _une_parser_state {
  une_token *tokens;
  size_t index;
  bool inside_loop;
} une_parser_state;
#pragma endregion une_parser_state

une_parser_state une_parser_state_create(void);
void une_parser_state_free(une_parser_state ps);

#endif /* UNE_PARSER_STATE_H */
