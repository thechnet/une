/*
lexer_state.h - Une
Modified 2021-06-04
*/

#ifndef UNE_LEXER_STATE_H
#define UNE_LEXER_STATE_H

#include "../primitive.h"

#pragma region une_lexer_state
typedef struct _une_lexer_state {
  bool read_from_file;
  char *path;
  FILE *file;
  wchar_t *text;
  size_t index;
  wint_t wc;
  void (*get)(struct _une_lexer_state*);
  wint_t (*peek)(struct _une_lexer_state*);
} une_lexer_state;
#pragma region une_lexer_state

une_lexer_state une_lexer_state_create(bool read_from_file, char *path, wchar_t* text);
void une_lexer_state_free(une_lexer_state ls);

#endif /* UNE_LEXER_STATE_H */
