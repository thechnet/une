/*
lexer.c - Une
Updated 2021-07-05
*/

/* Header-specific includes. */
#include "lexer.h"

/* Implementation-specific includes. */
#include <string.h>
#include "tools.h"
#include "stream.h"

/*
Two-character token characters.
*/
const wchar_t *une_2c_tokens_wc = L"==!=>=<=//**&&||";

/*
Two-character token types.
*/
const une_token_type une_2c_tokens_tt[] = {
  UNE_TT_EQU,
  UNE_TT_NEQ,
  UNE_TT_GEQ,
  UNE_TT_LEQ,
  UNE_TT_FDIV,
  UNE_TT_POW,
  UNE_TT_AND,
  UNE_TT_OR,
};

/*
One-character token characters.
*/
const wchar_t *une_1c_tokens_wc = L"(){}[],=><+-*/%!";

/*
One-character token types.
*/
const une_token_type une_1c_tokens_tt[] = {
  UNE_TT_LPAR,
  UNE_TT_RPAR,
  UNE_TT_LBRC,
  UNE_TT_RBRC,
  UNE_TT_LSQB,
  UNE_TT_RSQB,
  UNE_TT_SEP,
  UNE_TT_SET,
  UNE_TT_GTR,
  UNE_TT_LSS,
  UNE_TT_ADD,
  UNE_TT_SUB,
  UNE_TT_MUL,
  UNE_TT_DIV,
  UNE_TT_MOD,
  UNE_TT_NOT,
};

/*
Public lexer interface.
*/

UNE_ISTREAM_WFILE_PULLER(__une_lex_wfile_pull);
UNE_ISTREAM_WFILE_PEEKER(__une_lex_wfile_peek)
UNE_ISTREAM_WFILE_ACCESS(__une_lex_wfile_now);
UNE_ISTREAM_ARRAY_PULLER_VAL(__une_lex_array_pull, wint_t, WEOF, true);
UNE_ISTREAM_ARRAY_PEEKER_VAL(__une_lex_array_peek, wint_t, WEOF, true);
UNE_ISTREAM_ARRAY_ACCESS_VAL(__une_lex_array_now, wint_t, WEOF, true);
UNE_OSTREAM_PUSHER(__une_lex_push, une_token);
UNE_OSTREAM_PEEKER_REF(__une_lex_out_peek, une_token, NULL);

une_token *une_lex(une_error *error, une_lexer_state *ls)
{
  /* Initialize une_lexer_state. */
  if (ls->read_from_file) {
    ls->pull = &__une_lex_wfile_pull;
    ls->peek = &__une_lex_wfile_peek;
    ls->now = &__une_lex_wfile_now;
    FILE *file = fopen(ls->path, "r,ccs=UTF-8");
    if (file == NULL) {
      *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, ((une_position){ .start=0, .end=0 }));
      return NULL;
    }
    fclose(file);
    ls->in = une_istream_wfile_create(ls->path);
  } else {
    ls->pull = &__une_lex_array_pull;
    ls->peek = &__une_lex_array_peek;
    ls->now = &__une_lex_array_now;
    ls->in = une_istream_array_create((void*)ls->text, wcslen(ls->text));
  }
  ls->pull(&ls->in);
  
  /* Initialize token buffer. */
  une_token *tokens = une_malloc(UNE_SIZE_TOKEN_BUF*sizeof(*tokens));
  une_ostream out = une_ostream_create((void*)tokens, UNE_SIZE_TOKEN_BUF, sizeof(*tokens), true);
  void (*push)(une_ostream*, une_token) = &__une_lex_push;
  une_token *(*tkpeek)(une_ostream*, ptrdiff_t) = &__une_lex_out_peek;
  
  // LOGS((wchar_t*)ls->in.data.array.array);
  // wint_t ch = ls->now(&ls->in);
  // ch = ((wchar_t*)ls->in.data.array.array)[0];
  // wchar_t ch = (wint_t)L'r';
  // assert(ch == L'r');
  // exit(0);
  
  while (ls->now(&ls->in) != WEOF) {
    /* Check for error. */
    if (error->type != __UNE_ET_none__) {
      for (ptrdiff_t i=0; i<out.index-1 /* Don't free __UNE_TT_none__ */; i++)
        une_token_free(tokens[i]);
      une_free(tokens);
      tokens = NULL;
      break;
    }
    
    /* Skip whitespace. */
    while (UNE_LEXER_WC_IS_SOFT_WHITESPACE(ls->now(&ls->in)))
      ls->pull(&ls->in);
    
    /* Number. */
    if (UNE_LEXER_WC_IS_DIGIT(ls->now(&ls->in))) {
      push(&out, une_lex_num(error, ls));
      continue;
    }
    
    /* String. */
    if (ls->now(&ls->in) == L'"') {
      push(&out, une_lex_str(error, ls));
      continue;
    }

    /* Identifier. */
    if (UNE_LEXER_WC_CAN_BEGIN_ID(ls->now(&ls->in))) {
      push(&out, une_lex_id(error, ls));
      continue;
    }

    /* Two-character tokens. */
    une_token tk = une_lex_2c_token(error, ls);
    if (tk.type != __UNE_TT_none__) {
      push(&out, tk);
      continue;
    }
    
    /* One-character tokens. */
    tk = une_lex_1c_token(error, ls);
    if (tk.type != __UNE_TT_none__) {
      push(&out, tk);
      if (tk.type == UNE_TT_RBRC)
        push(&out, (une_token){
          .type = UNE_TT_NEW,
          .pos = tk.pos,
        });
      continue;
    }
    
    /* NEW. */
    if (UNE_LEXER_WC_IS_HARD_WHITESPACE(ls->now(&ls->in))) {
      size_t idx_left = ls->in.index;
      while (UNE_LEXER_WC_IS_WHITESPACE(ls->now(&ls->in)))
        ls->pull(&ls->in);
      if (out.index >= 0 && tkpeek(&out, -1)->type != UNE_TT_NEW)
        push(&out, (une_token){
          .type = UNE_TT_NEW,
          .pos = (une_position){ .start = idx_left, .end = ls->in.index },
          .value._vp = 0
        });
      continue;
    }
    
    /* Comment. */
    if (ls->now(&ls->in) == L'#') {
      do
        ls->pull(&ls->in);
      while (ls->now(&ls->in) != WEOF && ls->now(&ls->in) != L'\n');
      continue;
    }
    
    /* Illegal character. */
    *error = UNE_ERROR_SET(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){ls->in.index, ls->in.index+1}));
    push(&out, une_token_create(__UNE_TT_none__));
    continue;
  }
  if (out.index > 0 && tkpeek(&out, -1)->type != UNE_TT_NEW)
    push(&out, (une_token){
      .type = UNE_TT_NEW,
      .pos = (une_position){ .start = ls->in.index, .end = ls->in.index+1 },
      .value._vp = NULL
    });
  push(&out, (une_token){
    .type = UNE_TT_EOF,
    .pos = (une_position){ .start = ls->in.index, .end = ls->in.index+1 },
    .value._vp = NULL
  });
  
  /* Wrap up. */
  if (ls->read_from_file)
    une_istream_wfile_free(ls->in);
  
  return tokens;
}

/*
Lex a numeric token.
*/
__une_lexer(une_lex_num)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_NUM_LEN;
  wchar_t *buffer = une_malloc(buffer_size*sizeof(*buffer));
  size_t idx_start = ls->in.index;
  
  do {
    /* Ensure sufficient space in buffer. */
    while (ls->in.index-idx_start+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = une_realloc(buffer, buffer_size*sizeof(*buffer));
    }
  
    /* Add character to buffer. */
    buffer[ls->in.index-idx_start] = ls->now(&ls->in);
    ls->pull(&ls->in);
  } while (UNE_LEXER_WC_IS_DIGIT(ls->now(&ls->in)));
  
  if (ls->now(&ls->in) != L'.') {
    /* Return integer. */
    buffer[ls->in.index-idx_start] = L'\0';
    une_token tk = (une_token){
      .type = UNE_TT_INT,
      .pos = (une_position){idx_start, ls->in.index},
      .value._int = une_wcs_to_une_int(buffer),
    };
    une_free(buffer);
    return tk;
  }
  
  /* Try to lex floating point number. */
  
  buffer[ls->in.index-idx_start] = ls->now(&ls->in);
  ls->pull(&ls->in);
  
  size_t idx_before_decimals = ls->in.index;
  
  while (ls->now(&ls->in) >= L'0' && ls->now(&ls->in) <= L'9') {
    /* Ensure sufficient space in buffer. */
    if (ls->in.index-idx_start+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = une_realloc(buffer, buffer_size*sizeof(*buffer));
    }
  
    /* Add character to buffer. */
    buffer[ls->in.index-idx_start] = ls->now(&ls->in);
    ls->pull(&ls->in);
  }
  
  /* No digits after decimal point. */
  if (ls->in.index == idx_before_decimals) {
    *error = UNE_ERROR_SET(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){ls->in.index, ls->in.index+1}));
    une_free(buffer);
    return une_token_create(__UNE_TT_none__);
  }
  
  /* Return floating point number. */
  buffer[ls->in.index-idx_start] = L'\0';
  une_token tk = (une_token){
    .type = UNE_TT_FLT,
    .pos = (une_position){idx_start, ls->in.index},
    .value._flt = une_wcs_to_une_flt(buffer),
  };
  une_free(buffer);
  return tk;
}

/*
Lex a string token.
*/
__une_lexer(une_lex_str)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_STR_LEN;
  wchar_t *buffer = une_malloc(buffer_size*sizeof(*buffer));
  size_t idx_start = ls->in.index;
  bool escape = false;
  size_t buffer_index = 0;
  
  while (true) {
    /* Ensure sufficient space in string buffer. */
    if (buffer_index+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = une_realloc(buffer, buffer_size*sizeof(*buffer));
    }
    
    ls->pull(&ls->in);
    
    /* Ignore carriage return. */
    if (ls->now(&ls->in) == L'\r')
      continue;
    
    /* Premature end of stream. */
    if (ls->now(&ls->in) == WEOF) {
      *error = UNE_ERROR_SET(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){ls->in.index, ls->in.index+1}));
      une_free(buffer);
      return une_token_create(__UNE_TT_none__);
    }
    
    /* Escaped characters. */
    if (escape) {
      escape = false;
      switch (ls->now(&ls->in)) {
        case L'\\':
        case L'"':
          buffer[buffer_index++] = ls->now(&ls->in);
          continue;
        case L'n':
          buffer[buffer_index++] = L'\n';
          continue;
        case L'\n':
          continue;
        default: {
          *error = UNE_ERROR_SET(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){ls->in.index, ls->in.index+1}));
          une_free(buffer);
          return une_token_create(__UNE_TT_none__);
        }
      }
    }
    
    /* Schedule escaped character. */
    if (ls->now(&ls->in) == L'\\') {
      escape = true;
      continue;
    }
    
    /* End of string. */
    if (ls->now(&ls->in) == L'"') {
      ls->pull(&ls->in);
      break;
    }
    
    /* Add character to string buffer. */
    buffer[buffer_index++] = ls->now(&ls->in);
  }
  
  buffer[buffer_index] = L'\0';
  return (une_token){
    .type = UNE_TT_STR,
    .pos = (une_position){idx_start, ls->in.index},
    /* DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed. */
    .value._wcs = buffer,
  };
}

/*
Lex an id token.
*/
__une_lexer(une_lex_id)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_ID_LEN;
  wchar_t *buffer = une_malloc(buffer_size*sizeof(*buffer));
  size_t idx_start = ls->in.index;
  size_t buffer_index = 0;
  
  while (UNE_LEXER_WC_CAN_BE_IN_ID(ls->now(&ls->in))) {
    /* Ensure sufficient space in string buffer. */
    if (buffer_index >= buffer_size-1) { /* NUL. */
      buffer_size *= 2;
      buffer = une_realloc(buffer, buffer_size*sizeof(*buffer));
    }
    
    /* Add character to string buffer. */
    buffer[buffer_index++] = ls->now(&ls->in);
    ls->pull(&ls->in);
  }
  
  buffer[buffer_index] = L'\0';
  
  /* Construct keyword or string token. */
  une_token tk;
  if (!wcscmp(buffer, L"if"))
    tk.type = UNE_TT_IF;
  else if (!wcscmp(buffer, L"elif"))
    tk.type = UNE_TT_ELIF;
  else if (!wcscmp(buffer, L"else"))
    tk.type = UNE_TT_ELSE;
  else if (!wcscmp(buffer, L"for"))
    tk.type = UNE_TT_FOR;
  else if (!wcscmp(buffer, L"from"))
    tk.type = UNE_TT_FROM;
  else if (!wcscmp(buffer, L"till"))
    tk.type = UNE_TT_TILL;
  else if (!wcscmp(buffer, L"while"))
    tk.type = UNE_TT_WHILE;
  else if (!wcscmp(buffer, L"def"))
    tk.type = UNE_TT_DEF;
  else if (!wcscmp(buffer, L"return"))
    tk.type = UNE_TT_RETURN;
  else if (!wcscmp(buffer, L"break"))
    tk.type = UNE_TT_BREAK;
  else if (!wcscmp(buffer, L"continue"))
    tk.type = UNE_TT_CONTINUE;
  else if (!wcscmp(buffer, L"not"))
    tk.type = UNE_TT_NOT;
  else if (!wcscmp(buffer, L"global"))
    tk.type = UNE_TT_GLOBAL;
  else {
    tk.type = UNE_TT_ID;
    tk.value._wcs = buffer;
  }
  if (tk.type != UNE_TT_ID)
    une_free(buffer);
  
  tk.pos = (une_position){ .start = idx_start, .end = ls->in.index };
  return tk;
}

/*
Attempt to lex a two-character token.
*/
__une_lexer(une_lex_2c_token)
{
  size_t i = 0;
  while (true) {
    if (une_2c_tokens_wc[i] == L'\0')
      return une_token_create(__UNE_TT_none__);
    if (une_2c_tokens_wc[i] == ls->now(&ls->in) && ls->peek(&ls->in, 1) == une_2c_tokens_wc[i+1]) {
      ls->pull(&ls->in);
      ls->pull(&ls->in);
      return (une_token){
        .type = une_2c_tokens_tt[i/2],
        .pos = (une_position){ .start = ls->in.index-2, .end = ls->in.index },
        .value._vp = 0
      };
    }
    i += 2;
  }
}

/*
Attempt to lex a one-character token.
*/
__une_lexer(une_lex_1c_token)
{
  size_t i = 0;
  while (true) {
    if (une_1c_tokens_wc[i] == L'\0')
      return une_token_create(__UNE_TT_none__);
    if (une_1c_tokens_wc[i] == ls->now(&ls->in)) {
      ls->pull(&ls->in);
      return (une_token){
        .type = une_1c_tokens_tt[i],
        .pos = (une_position){ .start = ls->in.index-1, .end = ls->in.index },
        .value._vp = 0
      };
    }
    i++;
  }
}
