/*
lexer.c - Une
Modified 2023-06-29
*/

/* Header-specific includes. */
#include "lexer.h"

/* Implementation-specific includes. */
#include <string.h>
#include "tools.h"
#include "builtin_functions.h"

/*
Public lexer interface.
*/

void une_lex(une_error *error, une_lexer_state *ls)
{
  assert(ls->text);
  
  while (true) {
    /* Check for error. */
    if (error->type != UNE_ET_none__) {
      for (size_t i=0; i<ls->tokens_count - 1 /* Don't free UNE_TT_none__. */ ; i++)
        une_token_free(ls->tokens[i]);
      free(ls->tokens);
      ls->tokens = NULL;
      break;
    }
    
    /* Begin string expression. */
    if (ls->begin_str_expression) {
      ls->begin_str_expression = false;
      ls->in_str_expression = true;
      une_lexer_commit(ls, (une_token){
        .type = UNE_TT_STR_EXPRESSION_BEGIN,
        .pos = (une_position){
          .start = ls->text_index-1,
          .end = ls->text_index,
          .line = ls->line
        },
        .value._vp = 0
      });
    }
    
    /* Expected end of file. */
    if (une_lexer_now(ls) == L'\0') {
      if (ls->tokens_count > 0 && ls->tokens[ls->tokens_count-1].type != UNE_TT_NEW)
        une_lexer_commit(ls, (une_token){
          .type = UNE_TT_NEW,
          .pos = (une_position){
            .start = ls->text_index,
            .end = ls->text_index+1,
            .line = ls->line
          },
          .value._vp = NULL
        });
      une_lexer_commit(ls, (une_token){
        .type = UNE_TT_EOF,
        .pos = (une_position){
          .start = ls->text_index,
          .end = ls->text_index+1,
          .line = ls->line
        },
        .value._vp = NULL
      });
      break;
    }
    
    /* Skip whitespace. */
    if (UNE_LEXER_WC_IS_SOFT_WHITESPACE(une_lexer_now(ls))) {
      while (UNE_LEXER_WC_IS_SOFT_WHITESPACE(une_lexer_now(ls)))
        une_lexer_advance(ls);
      continue;
    }
    
    /* Number. */
    if (UNE_LEXER_WC_IS_DIGIT(une_lexer_now(ls))) {
      une_lexer_commit(ls, une_lex_number(error, ls));
      continue;
    }
    
    /* String. */
    if (une_lexer_now(ls) == L'"') {
      une_lexer_commit(ls, une_lex_string(error, ls));
      continue;
    }
    if (ls->in_str_expression && une_lexer_now(ls) == L'}') {
      une_lexer_commit(ls, (une_token){
        .type = UNE_TT_STR_EXPRESSION_END,
        .pos = (une_position){
          .start = ls->text_index,
          .end = ls->text_index+1,
          .line = ls->line
        },
        .value._vp = 0
      });
      ls->in_str_expression = false;
      une_lexer_commit(ls, une_lex_string(error, ls));
      continue;
    }
    
    /* Operator. */
    une_token tk = une_lex_operator(error, ls);
    if (tk.type != UNE_TT_none__) {
      une_lexer_commit(ls, tk);
      continue;
    }

    /* Keyword or identifier. */
    if (UNE_LEXER_WC_CAN_BEGIN_ID(une_lexer_now(ls))) {
      une_lexer_commit(ls, une_lex_keyword_or_identifier(error, ls));
      continue;
    }
    
    /* NEW. */
    if (UNE_LEXER_WC_IS_HARD_WHITESPACE(une_lexer_now(ls))) {
      size_t idx_left = ls->text_index;
      size_t lines = 0;
      while (UNE_LEXER_WC_IS_WHITESPACE(une_lexer_now(ls))) {
        if (une_lexer_now(ls) == '\n')
          lines++;
        une_lexer_advance(ls);
      }
      if (ls->tokens_count > 0 && ls->tokens[ls->tokens_count-1].type != UNE_TT_NEW)
        une_lexer_commit(ls, (une_token){
          .type = UNE_TT_NEW,
          .pos = (une_position){
            .start = idx_left,
            .end = ls->text_index,
            .line = ls->line
          },
          .value._vp = 0
        });
      ls->line += lines;
      continue;
    }
    
    /* Comment. */
    if (une_lexer_now(ls) == L'#') {
      do
        une_lexer_advance(ls);
      while (une_lexer_now(ls) != L'\0' && une_lexer_now(ls) != L'\n');
      continue;
    }
    
    /* Unexpected character. */
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
      .start = ls->text_index,
      .end = ls->text_index+1,
      .line = ls->line
    }));
    une_lexer_commit(ls, une_token_create(UNE_TT_none__));
    continue;
  }
}

/*
Lexers.
*/

une_lexer__(une_lex_operator)
{
  for (une_token_type tt=UNE_R_BGN_OPERATOR_TOKENS; tt<=UNE_R_END_OPERATOR_TOKENS; tt++) {
    ptrdiff_t i=0;
    while (une_lexer_peek(ls, i) == (wint_t)une_token_table[tt-1][i]) {
      if ((wint_t)une_token_table[tt-1][++i] == L'\0') {
        ptrdiff_t starting_index = (ptrdiff_t)ls->text_index;
        for (; i>0; i--)
          une_lexer_advance(ls);
        return (une_token){
          .type = tt,
          .pos = (une_position){
            .start = (size_t)starting_index,
            .end = ls->text_index,
            .line = ls->line
          }
        };
      }
    }
  }
  return une_token_create(UNE_TT_none__);
}

une_lexer__(une_lex_number)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_NUM_LEN;
  wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
  verify(buffer);
  size_t idx_start = ls->text_index;
  
  do {
    /* Ensure sufficient space in buffer. */
    while (ls->text_index-idx_start+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
  
    /* Add character to buffer. */
    buffer[ls->text_index-idx_start] = (wchar_t)une_lexer_now(ls);
    une_lexer_advance(ls);
  } while (UNE_LEXER_WC_IS_DIGIT(une_lexer_now(ls)));
  
  if (une_lexer_now(ls) != L'.' || une_lexer_peek(ls, 1) == L'.') { /* This alternative case catches situations where an integer is followed by two dots - this is not a decimal point, but instead the beginning of a DOTDOT token. */
    /* Return integer. */
    buffer[ls->text_index-idx_start] = L'\0';
    une_int int_;
    if(!une_wcs_to_une_int(buffer, &int_))
      assert(false);
    une_token tk = (une_token){
      .type = UNE_TT_INT,
      .pos = (une_position){
        .start = idx_start,
        .end = ls->text_index,
        .line = ls->line
      },
      .value._int = int_
    };
    free(buffer);
    return tk;
  }
  
  /* Try to lex floating point number. */
  
  buffer[ls->text_index-idx_start] = (wchar_t)une_lexer_now(ls); /* '.'. */
  une_lexer_advance(ls);
  
  size_t idx_before_decimals = ls->text_index;
  
  while (une_lexer_now(ls) >= L'0' && une_lexer_now(ls) <= L'9') {
    /* Ensure sufficient space in buffer. */
    if (ls->text_index-idx_start+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
  
    /* Add character to buffer. */
    buffer[ls->text_index-idx_start] = (wchar_t)une_lexer_now(ls);
    une_lexer_advance(ls);
  }
  
  /* No digits after decimal point. */
  if (ls->text_index == idx_before_decimals) {
    *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
      .start = ls->text_index,
      .end = ls->text_index+1,
      .line = ls->line
    }));
    free(buffer);
    return une_token_create(UNE_TT_none__);
  }
  
  /* Return floating point number. */
  buffer[ls->text_index-idx_start] = L'\0';
  une_flt flt;
  if (!une_wcs_to_une_flt(buffer, &flt))
    assert(false);
  une_token tk = (une_token){
    .type = UNE_TT_FLT,
    .pos = (une_position){
      .start = idx_start,
      .end = ls->text_index,
      .line = ls->line
    },
    .value._flt = flt
  };
  free(buffer);
  return tk;
}

une_lexer__(une_lex_string)
{
  /* Setup. */
  size_t buffer_size = UNE_SIZE_STR_LEN;
  wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
  verify(buffer);
  size_t idx_start = ls->text_index;
  size_t line_start = ls->line; /* Strings can contain newlines. */
  bool escape = false;
  size_t buffer_index = 0;
  
  while (true) {
    /* Ensure sufficient space in string buffer. */
    if (buffer_index+1 /* NUL. */ >= buffer_size) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
    
    une_lexer_advance(ls);
    
    /* Correct lexer state line. */
    if (une_lexer_now(ls) == L'\n')
      ls->line++;
    
    /* Ignore carriage return. */
    if (une_lexer_now(ls) == L'\r')
      continue;
    
    /* Premature end of string. */
    if (une_lexer_now(ls) == L'\0') {
      *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
        .start = ls->text_index,
        .end = ls->text_index+1,
        .line = ls->line
      }));
      free(buffer);
      return une_token_create(UNE_TT_none__);
    }
    
    /* Escaped characters. */
    if (escape) {
      escape = false;
      switch (une_lexer_now(ls)) {
        case L'\\':
        case L'"':
        case L'{':
        case L'}':
          buffer[buffer_index++] = (wchar_t)une_lexer_now(ls);
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
          *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
            .start = ls->text_index,
            .end = ls->text_index+1,
            .line = ls->line
          }));
          free(buffer);
          return une_token_create(UNE_TT_none__);
        }
      }
    }
    
    /* Schedule escaped character. */
    if (une_lexer_now(ls) == L'\\') {
      escape = true;
      continue;
    }
    
    /* End of string. */
    if (une_lexer_now(ls) == L'"') {
      une_lexer_advance(ls);
      break;
    }
    
    /* Beginning of string expression (end of this string). */
    if (une_lexer_now(ls) == L'{') {
      if (ls->in_str_expression) {
        *error = UNE_ERROR_SET(UNE_ET_SYNTAX, ((une_position){
          .start = ls->text_index,
          .end = ls->text_index+1,
          .line = ls->line
        }));
        free(buffer);
        return une_token_create(UNE_TT_none__);
      }
      une_lexer_advance(ls);
      ls->begin_str_expression = true;
      break;
    }
    
    /* Add character to string buffer. */
    buffer[buffer_index++] = (wchar_t)une_lexer_now(ls);
  }
  
  buffer[buffer_index] = L'\0';
  return (une_token){
    .type = UNE_TT_STR,
    .pos = (une_position){
      .start = idx_start,
      .end = ls->text_index,
      .line = line_start
    },
    /* DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed. */
    .value._wcs = buffer,
  };
}

une_lexer__(une_lex_keyword_or_identifier)
{
  size_t buffer_size = UNE_SIZE_ID_LEN;
  wchar_t *buffer = malloc(buffer_size*sizeof(*buffer));
  verify(buffer);
  size_t idx_start = ls->text_index;
  size_t buffer_index = 0;
  
  /* Read keyword or identifier. */
  while (UNE_LEXER_WC_CAN_BE_IN_ID(une_lexer_now(ls))) {
    if (buffer_index >= buffer_size-1) {
      buffer_size *= 2;
      buffer = realloc(buffer, buffer_size*sizeof(*buffer));
      verify(buffer);
    }
    buffer[buffer_index++] = (wchar_t)une_lexer_now(ls);
    une_lexer_advance(ls);
  }
  buffer[buffer_index] = L'\0';
  
  /* Determine token type. */
  une_token tk;
  if (!wcscmp(buffer, une_token_table[UNE_TT_FUNCTION-1])) {
    tk.type = UNE_TT_FUNCTION;
    tk.value._vp = (void*)ls->name;
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
  tk.pos = (une_position){
    .start = idx_start,
    .end = ls->text_index,
    .line = ls->line
  };
  
  return tk;
}

/*
Helpers.
*/

wchar_t une_lexer_now(une_lexer_state *ls)
{
  assert(ls);
  assert(ls->text);
  assert(ls->text_index <= ls->text_length);
  return ls->text[ls->text_index];
}

wchar_t une_lexer_advance(une_lexer_state *ls)
{
  assert(ls);
  assert(ls->text);
  assert(ls->text_index <= ls->text_length);
  if (ls->text_index + 1 <= ls->text_length)
    ls->text_index++;
  return ls->text[ls->text_index];
}

wchar_t une_lexer_peek(une_lexer_state *ls, une_int offset)
{
  assert(ls);
  assert(ls->text);
  assert(ls->text_index <= ls->text_length);
  return ls->text[une_clamp((une_int)ls->text_index+offset, 0, (une_int)ls->text_length)];
}

void une_lexer_commit(une_lexer_state *ls, une_token token)
{
  assert(ls);
  assert(ls->tokens);
  
  if (ls->tokens_count >= ls->tokens_size) {
    ls->tokens_size *= 2;
    ls->tokens = realloc(ls->tokens, ls->tokens_size*sizeof(*ls->tokens));
    verify(ls->tokens);
  }
  
  ls->tokens[ls->tokens_count++] = token;
}
