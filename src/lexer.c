/*
lexer.c - Une
Updated 2021-06-04
*/

#include "lexer.h"

#pragma region une_lex
une_token *une_lex(une_error *error, une_lexer_state *ls)
{
  if (ls->read_from_file) {
    ls->get = &une_lexer_fgetwc;
    ls->peek = &une_lexer_fpeekwc;
    ls->file = fopen(ls->path, "r,ccs=UTF-8");
    if (ls->file == NULL) {
      WERR(L"File not found");
    }
  } else {
    ls->get = &une_lexer_sgetwc;
    ls->peek = &une_lexer_speekwc;
  }
  
  size_t tokens_size = UNE_SIZE_MEDIUM; // FIXME: SIZE
  une_token *tokens = rmalloc(tokens_size*sizeof(*tokens));
  size_t tokens_index = 0;
  if (ls->read_from_file) {
    ls->get(ls);
  } else {
    ls->wc = ls->peek(ls);
  }
  
  while (true) {
    if (tokens_index+1 >= tokens_size) {
      tokens_size *= 2;
      une_token *_tokens = rrealloc(tokens, tokens_size*sizeof(*_tokens));
      tokens = _tokens;
      wprintf(L"Warning: Tokens doubled\n");
    }
    
    // Number
    if (ls->wc >= L'0' && ls->wc <= L'9') {
      une_token tk = une_lex_num(error, ls);
      if (tk.type == __UNE_TT_none__) {
        for (size_t i=0; i<tokens_index; i++) une_token_free(tokens[i]);
        free(tokens);
        tokens = NULL;
        break;
      }
      tokens[tokens_index++] = tk;
      continue;
    }
    
    // String
    if (ls->wc == L'"') {
      une_token tk = une_lex_str(error, ls);
      if (tk.type == __UNE_TT_none__) {
        for (size_t i=0; i<tokens_index; i++) une_token_free(tokens[i]);
        free(tokens);
        tokens = NULL;
        break;
      }
      tokens[tokens_index++] = tk;
      continue;
    }

    // Identifier
    if ((ls->wc >= L'a' && ls->wc <= L'z')
    || (ls->wc >= L'A' && ls->wc <= L'Z')
    ||  ls->wc == L'_') {
      une_token tk = une_lex_id(error, ls);
      if (tk.type == __UNE_TT_none__) {
        for (size_t i=0; i<tokens_index; i++) une_token_free(tokens[i]);
        free(tokens);
        tokens = NULL;
        break;
      }
      tokens[tokens_index++] = tk;
      continue;
    }

    #pragma region Grouping and Ordering (- NEW)
    if (ls->wc == L'(') {
      tokens[tokens_index++] = (une_token){UNE_TT_LPAR, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L')') {
      tokens[tokens_index++] = (une_token){UNE_TT_RPAR, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L'{') {
      tokens[tokens_index++] = (une_token){UNE_TT_LBRC, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L'}') {
      tokens[tokens_index++] = (une_token){UNE_TT_RBRC, (une_position){ls->index, ls->index+1}, 0};
      tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L'[') {
      tokens[tokens_index++] = (une_token){UNE_TT_LSQB, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L']') {
      tokens[tokens_index++] = (une_token){UNE_TT_RSQB, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L',') {
      size_t pos_start = ls->index;
      ls->get(ls);
      while (ls->wc == L',') ls->get(ls);
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_SEP,
        .pos = (une_position){pos_start, ls->index+1},
      };
      continue;
    }
    if (ls->wc == WEOF) {
      if (tokens_index == 0 || tokens[tokens_index-1].type != UNE_TT_NEW) {
        tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){ls->index, ls->index+1}, 0};
      }
      tokens[tokens_index] = (une_token){UNE_TT_EOF, (une_position){ls->index, ls->index}, 0};
      break;
    }
    #pragma endregion Grouping and Ordering (- NEW, EOF)
    
    #pragma region Operators (+ Logical Equal)
    if (ls->wc == L'=') {
      ls->get(ls);
      if (ls->wc == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_EQU, (une_position){ls->index-1, ls->index+1}, 0};
        ls->get(ls);
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_SET, (une_position){ls->index, ls->index+1}, 0};
      continue;
    }
    if (ls->wc == L'+') {
      tokens[tokens_index++] = (une_token){UNE_TT_ADD, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L'-') {
      tokens[tokens_index++] = (une_token){UNE_TT_SUB, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    if (ls->wc == L'*') {
      ls->get(ls);
      if (ls->wc == L'*') {
        tokens[tokens_index++] = (une_token){UNE_TT_POW, (une_position){ls->index-1, ls->index+1}, 0};
        ls->get(ls);
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_MUL, (une_position){ls->index, ls->index+1}, 0};
      continue;
    }
    if (ls->wc == L'/') {
      ls->get(ls);
      if (ls->wc == L'/') {
        tokens[tokens_index++] = (une_token){UNE_TT_FDIV, (une_position){ls->index-1, ls->index+1}, 0};
        ls->get(ls);
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_DIV, (une_position){ls->index, ls->index+1}, 0};
      continue;
    }
    if (ls->wc == L'%') {
      tokens[tokens_index++] = (une_token){UNE_TT_MOD, (une_position){ls->index, ls->index+1}, 0};
      ls->get(ls);
      continue;
    }
    #pragma endregion Operators (+ Logical Equal)

    #pragma region Comparisons (- Logical Equal)
    if (ls->wc == L'!') {
      if (ls->peek(ls) == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_NEQ, (une_position){ls->index, ls->index+2}, 0};
        ls->get(ls);
        ls->get(ls);
        continue;
      }
      *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){ls->index, ls->index+1}),
                                   _int=(int)ls->wc, _int=0);
      for (size_t i=0; i<tokens_index; i++) une_token_free(tokens[i]);
      free(tokens);
      tokens = NULL;
      break;
    }
    if (ls->wc == L'>') {
      ls->get(ls);
      if (ls->wc == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_GEQ, (une_position){ls->index-1, ls->index+1}, 0};
        ls->get(ls);
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_GTR, (une_position){ls->index, ls->index+1}, 0};
      continue;
    }
    if (ls->wc == L'<') {
      ls->get(ls);
      if (ls->wc == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_LEQ, (une_position){ls->index-1, ls->index+1}, 0};
        ls->get(ls);
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_LSS, (une_position){ls->index, ls->index+1}, 0};
      continue;
    }
    #pragma endregion Comparisons (- Logical Equal)

    #pragma region Whitespace
    if (ls->wc == L' ' || ls->wc == L'\t') {
      ls->get(ls);
      continue;
    }
    if (ls->wc == L'\r' || ls->wc == L'\n' || ls->wc == L';') {
      size_t idx_left = ls->index;
      ls->get(ls);
      while (
        ls->wc == L' ' || ls->wc == L'\t' || ls->wc == L'\r' || ls->wc == L'\n' || ls->wc == L';'
      ) ls->get(ls);
      if (tokens_index == 0 || tokens[tokens_index-1].type != UNE_TT_NEW) {
        tokens[tokens_index++] = (une_token){
          UNE_TT_NEW, (une_position){idx_left, ls->index}, 0
        };
      }
      continue;
    }
    #pragma endregion Whitespace

    #pragma region Comment
    if (ls->wc == L'#') {
      while (ls->wc != WEOF && ls->wc != L'\n') ls->get(ls);
      continue;
    }
    #pragma endregion Comment

    #pragma region Illegal Character
    *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){ls->index, ls->index+1}),
                                 _int=(int)ls->wc, _int=0);
    for (size_t i=0; i<tokens_index; i++) une_token_free(tokens[i]);
    free(tokens);
    tokens = NULL;
    break;
    #pragma endregion Illegal Character
  }
  
  if (ls->read_from_file) fclose(ls->file);
  return tokens;
}
#pragma endregion une_lex

#pragma region une_lex_num
une_token une_lex_num(une_error *error, une_lexer_state *ls)
{
  size_t buffer_size = UNE_SIZE_SMALL;
  wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));
  
  size_t idx_start = ls->index;
  
  while (ls->wc >= L'0' && ls->wc <= L'9') {
    
    while (ls->index-idx_start >= buffer_size-2) { // NUL || '0' NUL
      buffer_size *= 2;
      wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
      buffer = _buffer;
    }
  
    buffer[ls->index-idx_start] = ls->wc;
    ls->get(ls);
  
  }
  
  // Integer
  if (ls->wc != L'.') {
    buffer[ls->index-idx_start] = L'\0';
    une_token tk = (une_token){
      .type = UNE_TT_INT,
      .pos = (une_position){idx_start, ls->index},
      .value._int = wcs_to_une_int(buffer),
    };
    free(buffer);
    return tk;
  }
  
  // Floating Point Number
  buffer[ls->index-idx_start] = ls->wc;
  ls->get(ls);
  
  size_t idx_before_decimals = ls->index;
  
  while (ls->wc >= L'0' && ls->wc <= L'9') {
    
    if (ls->index-idx_start >= buffer_size-2) { // NUL || '0' NUL
      buffer_size *= 2;
      wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
      buffer = _buffer;
    }
  
    buffer[ls->index-idx_start] = ls->wc;
    ls->get(ls);
  
  }
  
  if (ls->index == idx_before_decimals) {
    *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_CHARACTER,
                            ((une_position){ls->index, ls->index+1}),
                            _int=(une_int)ls->wc, _int=0);
    return (une_token){.type = __UNE_TT_none__};
  }
  
  buffer[ls->index-idx_start] = L'\0';
  une_token tk = (une_token){
    .type = UNE_TT_FLT,
    .pos = (une_position){idx_start, ls->index},
    .value._flt = wcs_to_une_flt(buffer),
  };
  free(buffer);
  return tk;
}
#pragma endregion une_lex_num

#pragma region une_lex_str
une_token une_lex_str(une_error *error, une_lexer_state *ls)
{
  size_t buffer_size = UNE_SIZE_MEDIUM;
  wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));
  
  size_t idx_start = ls->index;
  bool escape = false;
  size_t buffer_index = 0;
  
  while (true) {
    if (buffer_index >= buffer_size-1) { // NUL
      buffer_size *= 2;
      wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
      buffer = _buffer;
    }
    
    ls->get(ls);
    
    if (ls->wc == WEOF) {
      *error = UNE_ERROR_SET(UNE_ET_UNTERMINATED_STRING, ((une_position){ls->index, ls->index+1}));
      return (une_token){.type = __UNE_TT_none__};
    }
    
    if (ls->wc == L'\r') continue;
    
    if (escape) {
      escape = false;
      switch (ls->wc) {
        case L'\\':
        case L'"':
          buffer[buffer_index++] = ls->wc;
          continue;
        case L'n':
          buffer[buffer_index++] = L'\n';
          continue;
        case L'\n': continue;
        default: {
          *error = UNE_ERROR_SET(UNE_ET_CANT_ESCAPE_CHAR, ((une_position){ls->index, ls->index+1}));
          return (une_token){.type = __UNE_TT_none__};
        }
      }
    }
    
    if (ls->wc == L'\\') {
      escape = true;
      continue;
    }
    
    if (ls->wc == L'"') {
      ls->get(ls);
      break;
    }
    
    buffer[buffer_index++] = ls->wc;
  }
  
  buffer[buffer_index] = L'\0';
  return (une_token){
    .type = UNE_TT_STR,
    .pos = (une_position){idx_start, ls->index},
    // DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed.
    .value._wcs = buffer,
  };
}
#pragma endregion une_lex_str

#pragma region une_lex_id
une_token une_lex_id (une_error *error, une_lexer_state *ls)
{
  size_t buffer_size = UNE_SIZE_MEDIUM;
  wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));
  
  size_t idx_start = ls->index;
  bool escape = false;
  size_t buffer_index = 0;
  
  while ((ls->wc >= L'a' && ls->wc <= L'z')
  || (ls->wc >= L'A' && ls->wc <= L'Z')
  || (ls->wc >= L'0' && ls->wc <= L'9')
  ||  ls->wc == L'_') {
    
    if (buffer_index >= buffer_size-1) { // NUL
      buffer_size *= 2;
      wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
      buffer = _buffer;
    }
    
    buffer[buffer_index++] = ls->wc;
    
    ls->get(ls);
  }
  
  buffer[buffer_index] = L'\0';
  
  une_token tk;
  
       if (!wcscmp(buffer, L"if"))       tk.type = UNE_TT_IF;
  else if (!wcscmp(buffer, L"elif"))     tk.type = UNE_TT_ELIF;
  else if (!wcscmp(buffer, L"else"))     tk.type = UNE_TT_ELSE;
  else if (!wcscmp(buffer, L"for"))      tk.type = UNE_TT_FOR;
  else if (!wcscmp(buffer, L"from"))     tk.type = UNE_TT_FROM;
  else if (!wcscmp(buffer, L"till"))       tk.type = UNE_TT_TILL;
  else if (!wcscmp(buffer, L"while"))    tk.type = UNE_TT_WHILE;
  else if (!wcscmp(buffer, L"def"))      tk.type = UNE_TT_DEF;
  else if (!wcscmp(buffer, L"return"))   tk.type = UNE_TT_RETURN;
  else if (!wcscmp(buffer, L"break"))    tk.type = UNE_TT_BREAK;
  else if (!wcscmp(buffer, L"continue")) tk.type = UNE_TT_CONTINUE;
  else if (!wcscmp(buffer, L"and"))      tk.type = UNE_TT_AND;
  else if (!wcscmp(buffer, L"or"))       tk.type = UNE_TT_OR;
  else if (!wcscmp(buffer, L"not"))      tk.type = UNE_TT_NOT;
  else {
    tk.type = UNE_TT_ID;
    tk.value._wcs = buffer;
  }
  
  if (tk.type != UNE_TT_ID) free(buffer);
  
  tk.pos = (une_position){idx_start, ls->index};
  return tk;
}
#pragma endregion une_lex_id

#pragma region une_lexer_fgetwc
void une_lexer_fgetwc(une_lexer_state *ls)
{
  ls->index++;
  ls->wc = fgetwc(ls->file);
}
#pragma endregion une_lexer_fgetwc

#pragma region une_lexer_sgetwc
void une_lexer_sgetwc(une_lexer_state *ls)
{
  ls->wc = ls->text[++ls->index];
  if (ls->wc == L'\0') ls->wc = WEOF;
}
#pragma endregion une_lexer_sgetwc

#pragma region une_lexer_fpeekwc
wint_t une_lexer_fpeekwc(une_lexer_state *ls)
{
  return ungetwc(fgetwc(ls->file), ls->file);
}
#pragma endregion une_lexer_fpeekwc

#pragma region une_lexer_speekwc
wint_t une_lexer_speekwc(une_lexer_state *ls)
{
  wint_t wc = ls->text[ls->index+1];
  if (wc == L'\0') return WEOF;
  return wc;
}
#pragma endregion une_lexer_speekwc
