/*
lexer_state.h - Une
Modified 2021-06-13
*/

#ifndef UNE_LEXER_STATE_H
#define UNE_LEXER_STATE_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../stream.h"
#include "token.h"

/*
Holds the state of the lexer.
*/
typedef struct _une_lexer_state {
  bool read_from_file;
  char *path;
  wchar_t *text;
  une_istream in;
  wint_t (*pull)(une_istream*);
  wint_t (*peek)(une_istream*, ptrdiff_t);
  wint_t (*now)(une_istream*);
} une_lexer_state;

/*
*** Interface.
*/

une_lexer_state une_lexer_state_create(bool read_from_file, char *path, wchar_t* text);

#endif /* UNE_LEXER_STATE_H */
