/*
lexer.c - Une
Modified 2023-05-08
*/

/* Header-specific includes. */
#include "lexer.h"

/* Implementation-specific includes. */
#include <string.h>
#include "tools.h"
#include "stream.h"
#include "builtin_functions.h"

/*
Public lexer interface.
*/

UNE_ISTREAM_WFILE_PULLER(une_lex_wfile_pull__)
UNE_ISTREAM_WFILE_PEEKER(une_lex_wfile_peek__)
UNE_ISTREAM_WFILE_ACCESS(une_lex_wfile_now__)
UNE_ISTREAM_ARRAY_PULLER_VAL(une_lex_array_pull__, wint_t, wint_t, WEOF, true)
UNE_ISTREAM_ARRAY_PEEKER_VAL(une_lex_array_peek__, wint_t, wint_t, WEOF, true)
UNE_ISTREAM_ARRAY_ACCESS_VAL(une_lex_array_now__, wint_t, wint_t, WEOF, true)
UNE_OSTREAM_PUSHER(une_lex_push__, une_token)
UNE_OSTREAM_PEEKER_REF(une_lex_out_peek__, une_token, NULL)

une_token *une_lex(une_error *error, une_lexer_state *ls)
{
  /* Initialize une_lexer_state. */
  if (ls->read_from_file) {
    ls->pull = &une_lex_wfile_pull__;
    ls->peek = &une_lex_wfile_peek__;
    ls->now = &une_lex_wfile_now__;
    if (!une_file_exists(ls->path)) {
      *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, ((une_position){ .start=0, .end=0 }));
      return NULL;
    }
    ls->in = une_istream_wfile_create(ls->path);
  } else {
    ls->pull = &une_lex_array_pull__;
    ls->peek = &une_lex_array_peek__;
    ls->now = &une_lex_array_now__;
    ls->in = une_istream_array_create((void*)ls->text, wcslen(ls->text));
  }
  ls->pull(&ls->in);
  
  /* Initialize token buffer. */
  une_token *tokens = malloc(UNE_SIZE_TOKEN_BUF*sizeof(*tokens));
  verify(tokens);
  une_ostream out = une_ostream_create((void*)tokens, UNE_SIZE_TOKEN_BUF, sizeof(*tokens), true);
  tokens = NULL; /* This pointer can turn stale after pushing. */
  void (*push)(une_ostream*, une_token) = &une_lex_push__;
  une_token *(*tkpeek)(une_ostream*, ptrdiff_t) = &une_lex_out_peek__;
  
  while (true) {
    /* Check for error. */
    if (error->type != UNE_ET_none__) {
      tokens = (une_token*)out.array; /* Reobtain up-to-date pointer. */
      for (ptrdiff_t i=0; i<out.index; i++) /* This doesn't try to free UNE_TT_none__ because out.index points to the index of the last item added. */
        une_token_free(tokens[i]);
      free(tokens);
      out.array = NULL; /* The ostream pointer is always up-to-date. */
      break;
    }
    
    /* Begin string expression. */
    if (ls->begin_str_expression) {
      ls->begin_str_expression = false;
      ls->in_str_expression = true;
      push(&out, (une_token){
        .type = UNE_TT_STR_EXPRESSION_BEGIN,
        .pos = (une_position){ .start = (size_t)ls->in.index-1, .end = (size_t)ls->in.index },
        .value._vp = 0
      });
    }
    
    /* Expected end of file. */
    if (ls->now(&ls->in) == WEOF) {
      if (out.index > -1 && tkpeek(&out, -1)->type != UNE_TT_NEW)
        push(&out, (une_token){
          .type = UNE_TT_NEW,
          .pos = (une_position){ .start = (size_t)ls->in.index, .end = (size_t)ls->in.index+1 },
          .value._vp = NULL
        });
      push(&out, (une_token){
        .type = UNE_TT_EOF,
        .pos = (une_position){ .start = (size_t)ls->in.index, .end = (size_t)ls->in.index+1 },
        .value._vp = NULL
      });
      break;
    }
    
    /* Skip whitespace. */
    if (UNE_LEXER_WC_IS_SOFT_WHITESPACE(ls->now(&ls->in))) {
      while (UNE_LEXER_WC_IS_SOFT_WHITESPACE(ls->now(&ls->in)))
        ls->pull(&ls->in);
      continue;
    }
    
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
    if (ls->in_str_expression && ls->now(&ls->in) == L'}') {
      push(&out, (une_token){
        .type = UNE_TT_STR_EXPRESSION_END,
        .pos = (une_position){ .start = (size_t)ls->in.index, .end = (size_t)ls->in.index+1 },
        .value._vp = 0
      });
      ls->in_str_expression = false;
      push(&out, une_lex_str(error, ls));
      continue;
    }
    
    /* Operator. */
    une_token tk = une_lex_operator(error, ls);
    if (tk.type != UNE_TT_none__) {
      push(&out, tk);
      continue;
    }

    /* Keyword or identifier. */
    if (UNE_LEXER_WC_CAN_BEGIN_ID(ls->now(&ls->in))) {
      push(&out, une_lex_keyword_or_identifier(error, ls));
      continue;
    }
    
    /* NEW. */
    if (UNE_LEXER_WC_IS_HARD_WHITESPACE(ls->now(&ls->in))) {
      size_t idx_left = (size_t)ls->in.index;
      while (UNE_LEXER_WC_IS_WHITESPACE(ls->now(&ls->in)))
        ls->pull(&ls->in);
      if (out.index >= 0 && tkpeek(&out, -1)->type != UNE_TT_NEW)
        push(&out, (une_token){
          .type = UNE_TT_NEW,
          .pos = (une_position){ .start = idx_left, .end = (size_t)ls->in.index },
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
    
    /* Unexpected character. */
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){(size_t)ls->in.index, (size_t)ls->in.index+1}));
    push(&out, une_token_create(UNE_TT_none__));
    continue;
  }
  
  /* Wrap up. */
  if (ls->read_from_file)
    une_istream_wfile_free(ls->in);
  
  tokens = (une_token*)out.array; /* Reobtain up-to-date pointer. */
  return tokens;
}

une_lexer__(une_lex_operator)
{
  for (une_token_type tt=UNE_R_BGN_OPERATOR_TOKENS; tt<=UNE_R_END_OPERATOR_TOKENS; tt++) {
    ptrdiff_t i=0;
    while (ls->peek(&ls->in, i) == (wint_t)une_token_table[tt-1][i]) {
      if ((wint_t)une_token_table[tt-1][++i] == L'\0') {
        ptrdiff_t starting_index = ls->in.index;
        for (; i>0; i--)
          ls->pull(&ls->in);
        return (une_token){
          .type = tt,
          .pos.start = (size_t)starting_index,
          .pos.end = (size_t)ls->in.index
        };
      }
    }
  }
  return une_token_create(UNE_TT_none__);
}

une_lexer__(une_lex_num)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_NUM_LEN;
  wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
  verify(buffer);
  size_t idx_start = (size_t)ls->in.index;
  
  do {
    /* Ensure sufficient space in buffer. */
    while ((size_t)ls->in.index-idx_start+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
  
    /* Add character to buffer. */
    buffer[(size_t)ls->in.index-idx_start] = (wchar_t)ls->now(&ls->in);
    ls->pull(&ls->in);
  } while (UNE_LEXER_WC_IS_DIGIT(ls->now(&ls->in)));
  
  if (ls->now(&ls->in) != L'.' || ls->peek(&ls->in, 1) == L'.') { /* This alternative case catches situations where an integer is followed by two dots - this is not a decimal point, but instead the beginning of a DOTDOT token. */
    /* Return integer. */
    buffer[(size_t)ls->in.index-idx_start] = L'\0';
    une_int int_;
    if(!une_wcs_to_une_int(buffer, &int_))
      assert(false);
    une_token tk = (une_token){
      .type = UNE_TT_INT,
      .pos = (une_position){idx_start, (size_t)ls->in.index},
      .value._int = int_
    };
    free(buffer);
    return tk;
  }
  
  /* Try to lex floating point number. */
  
  buffer[(size_t)ls->in.index-idx_start] = (wchar_t)ls->now(&ls->in); /* '.'. */
  ls->pull(&ls->in);
  
  size_t idx_before_decimals = (size_t)ls->in.index;
  
  while (ls->now(&ls->in) >= L'0' && ls->now(&ls->in) <= L'9') {
    /* Ensure sufficient space in buffer. */
    if ((size_t)ls->in.index-idx_start+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
  
    /* Add character to buffer. */
    buffer[(size_t)ls->in.index-idx_start] = (wchar_t)ls->now(&ls->in);
    ls->pull(&ls->in);
  }
  
  /* No digits after decimal point. */
  if (ls->in.index == (ptrdiff_t)idx_before_decimals) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){(size_t)ls->in.index, (size_t)ls->in.index+1}));
    free(buffer);
    return une_token_create(UNE_TT_none__);
  }
  
  /* Return floating point number. */
  buffer[(size_t)ls->in.index-idx_start] = L'\0';
  une_flt flt;
  if (!une_wcs_to_une_flt(buffer, &flt))
    assert(false);
  une_token tk = (une_token){
    .type = UNE_TT_FLT,
    .pos = (une_position){idx_start, (size_t)ls->in.index},
    .value._flt = flt
  };
  free(buffer);
  return tk;
}

une_lexer__(une_lex_str)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_STR_LEN;
  wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
  verify(buffer);
  size_t idx_start = (size_t)ls->in.index;
  bool escape = false;
  size_t buffer_index = 0;
  
  while (true) {
    /* Ensure sufficient space in string buffer. */
    if (buffer_index+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
    
    ls->pull(&ls->in);
    
    /* Ignore carriage return. */
    if (ls->now(&ls->in) == L'\r')
      continue;
    
    /* Premature end of string. */
    if (ls->now(&ls->in) == WEOF) {
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){(size_t)ls->in.index, (size_t)ls->in.index+1}));
      free(buffer);
      return une_token_create(UNE_TT_none__);
    }
    
    /* Escaped characters. */
    if (escape) {
      escape = false;
      switch (ls->now(&ls->in)) {
        case L'\\':
        case L'"':
        case L'{':
        case L'}':
          buffer[buffer_index++] = (wchar_t)ls->now(&ls->in);
          continue;
        case L'n':
          buffer[buffer_index++] = L'\n';
          continue;
        case L'e':
          buffer[buffer_index++] = L'\033';
          continue;
        case L'a':
          buffer[buffer_index++] = L'\a';
          continue;
        case L'\n':
          continue;
        default: {
          *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){(size_t)ls->in.index, (size_t)ls->in.index+1}));
          free(buffer);
          return une_token_create(UNE_TT_none__);
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
    
    /* Beginning of string expression (end of this string). */
    if (ls->now(&ls->in) == L'{') {
      if (ls->in_str_expression) {
        *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){(size_t)ls->in.index, (size_t)ls->in.index+1}));
        free(buffer);
        return une_token_create(UNE_TT_none__);
      }
      ls->pull(&ls->in);
      ls->begin_str_expression = true;
      break;
    }
    
    /* Add character to string buffer. */
    buffer[buffer_index++] = (wchar_t)ls->now(&ls->in);
  }
  
  buffer[buffer_index] = L'\0';
  return (une_token){
    .type = UNE_TT_STR,
    .pos = (une_position){idx_start, (size_t)ls->in.index},
    /* DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed. */
    .value._wcs = buffer,
  };
}

une_lexer__(une_lex_keyword_or_identifier)
{
  size_t buffer_size = UNE_SIZE_ID_LEN;
  wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
  verify(buffer);
  size_t idx_start = (size_t)ls->in.index;
  size_t buffer_index = 0;
  
  /* Read keyword or identifier. */
  while (UNE_LEXER_WC_CAN_BE_IN_ID(ls->now(&ls->in))) {
    if (buffer_index >= buffer_size-1) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
    buffer[buffer_index++] = (wchar_t)ls->now(&ls->in);
    ls->pull(&ls->in);
  }
  buffer[buffer_index] = L'\0';
  
  /* Determine token type. */
  une_token tk;
  if (!wcscmp(buffer, une_token_table[UNE_TT_FUNCTION-1])) {
    tk.type = UNE_TT_FUNCTION;
    tk.value._vp = (void*)(ls->read_from_file ? ls->path : UNE_COMMAND_LINE_NAME);
    goto keyword_or_identifier_defined;
  }
  for (une_token_type tt=UNE_R_BGN_KEYWORD_TOKENS; tt<=UNE_R_END_KEYWORD_TOKENS; tt++)
    if (!wcscmp(buffer, une_token_table[tt-1])) {
      tk.type = tt;
      goto keyword_or_identifier_defined;
    }
  une_builtin_function builtin = une_builtin_wcs_to_function(buffer);
  if (builtin != UNE_BUILTIN_none__) {
    tk.type = UNE_TT_BUILTIN;
    tk.value._int = (une_int)builtin;
    goto keyword_or_identifier_defined;
  }
  tk.type = UNE_TT_ID;
  tk.value._wcs = buffer;
  
  /* Finalize token. */
  keyword_or_identifier_defined:
  if (tk.type != UNE_TT_ID)
    free(buffer);
  tk.pos = (une_position){ .start = idx_start, .end = (size_t)ls->in.index };
  
  return tk;
}
