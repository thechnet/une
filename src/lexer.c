/*
lexer.c - Une
Updated 2021-04-28
*/

#include "lexer.h"

#pragma region une_lex_wcs
une_token *une_lex_wcs(wchar_t *text, une_error *error)
{
  size_t tokens_size = UNE_SIZE_MEDIUM; // FIXME:
  une_token *tokens = rmalloc(tokens_size*sizeof(*tokens));
  size_t tokens_index = 0;
  size_t idx = 0;
  while (true) {
    if (tokens_index+1 >= tokens_size) {
      tokens_size *= 2;
      une_token *_tokens = rrealloc(tokens, tokens_size *sizeof(*_tokens));
      tokens = _tokens;
      wprintf(L"Warning: Tokens doubled\n");
    }
    
    #pragma region Number
    if (text[idx] >= L'0' && text[idx] <= L'9') {

      size_t buffer_size = UNE_SIZE_SMALL;
      wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));

      size_t idx_start = idx;

      while (text[idx] >= L'0' && text[idx] <= L'9') {
        
        while (idx-idx_start >= buffer_size-2) { // NUL || '0' NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        buffer[idx-idx_start] = text[idx];
        idx++;

      }

      // Integer
      if (text[idx] != L'.') {
        buffer[idx-idx_start] = L'\0';
        une_token tk = (une_token){
          .type = UNE_TT_INT,
          .pos = (une_position){idx_start, idx},
          .value._int = wcs_to_une_int(buffer),
        };
        free(buffer);
        tokens[tokens_index++] = tk;
        continue;
      }

      // Floating Point Number
      buffer[idx-idx_start] = text[idx];
      idx++;

      size_t idx_before_decimals = idx;

      while (text[idx] >= L'0' && text[idx] <= L'9') {
        
        if (idx-idx_start >= buffer_size-2) { // NUL || '0' NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        buffer[idx-idx_start] = text[idx];
        idx++;

      }

      if (idx == idx_before_decimals) {
        *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){idx, idx+1}),
            _int=(une_int)text[idx], _int=0);
        return NULL;
      }

      buffer[idx-idx_start] = L'\0';
      une_token tk = (une_token){
        .type = UNE_TT_FLT,
        .pos = (une_position){idx_start, idx},
        .value._flt = wcs_to_une_flt(buffer),
      };
      free(buffer);
      tokens[tokens_index++] = tk;
      continue;
      
    }
    #pragma endregion Number

    #pragma region String
    if (text[idx] == L'"') {
      size_t buffer_size = UNE_SIZE_MEDIUM;
      wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));

      size_t idx_start = idx;
      bool escape = false;
      size_t buffer_index = 0;
int i=-1;
      while (true) {
        if (buffer_index >= buffer_size-1) { // NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        idx++;

        if (text[idx] == L'\0') {
          *error = UNE_ERROR_SET(UNE_ET_UNTERMINATED_STRING, ((une_position){idx, idx+1}));
          free(buffer);
          return NULL;
        }

        if (text[idx] == L'\r') continue;

        if (escape) {
          escape = false;
          switch (text[idx]) {
            case L'\\':
            case L'"':
              buffer[buffer_index++] = text[idx];
              continue;
            case L'n':
              buffer[buffer_index++] = L'\n';
              continue;
            default:
              *error = UNE_ERROR_SET(UNE_ET_CANT_ESCAPE_CHAR, ((une_position){idx, idx+1}));
              free(buffer);
              return NULL;
          }
        }

        if (text[idx] == L'\\') {
          escape = true;
          continue;
        }

        if (text[idx] == L'"') {
          idx++;
          break;
        }

        buffer[buffer_index++] = text[idx];
      }

      buffer[buffer_index] = L'\0';
      une_token tk = (une_token){
        .type = UNE_TT_STR,
        .pos = (une_position){idx_start, idx},
        // DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed.
        .value._wcs = buffer,
      };
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion String

    #pragma region Identifier
    if ((text[idx] >= L'a' && text[idx] <= L'z')
    || (text[idx] >= L'A' && text[idx] <= L'Z')
    ||  text[idx] == L'_') {
      size_t buffer_size = UNE_SIZE_MEDIUM;
      wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));

      size_t idx_start = idx;
      bool escape = false;
      size_t buffer_index = 0;

      while ((text[idx] >= L'a' && text[idx] <= L'z')
      || (text[idx] >= L'A' && text[idx] <= L'Z')
      || (text[idx] >= L'0' && text[idx] <= L'9')
      ||  text[idx] == L'_') {
        
        if (buffer_index >= buffer_size-1) { // NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        buffer[buffer_index++] = text[idx];
        
        idx++;
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

      tk.pos = (une_position){idx_start, idx};
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion Identifier

    #pragma region Grouping and Ordering (- NEW)
    if (text[idx] == L'(') {
      tokens[tokens_index++] = (une_token){UNE_TT_LPAR, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L')') {
      tokens[tokens_index++] = (une_token){UNE_TT_RPAR, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L'{') {
      tokens[tokens_index++] = (une_token){UNE_TT_LBRC, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L'}') {
      tokens[tokens_index++] = (une_token){UNE_TT_RBRC, (une_position){idx, idx+1}, 0};
      tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L'[') {
      tokens[tokens_index++] = (une_token){UNE_TT_LSQB, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L']') {
      tokens[tokens_index++] = (une_token){UNE_TT_RSQB, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L',') {
      size_t pos_start = idx;
      idx++;
      while (text[idx] == L',') idx++;
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_SEP,
        .pos = (une_position){pos_start, idx+1},
      };
      continue;
    }
    if (text[idx] == L'\0') {
      if (tokens_index == 0 || tokens[tokens_index-1].type != UNE_TT_NEW) {
        tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){idx, idx+1}, 0};
      }
      tokens[tokens_index] = (une_token){UNE_TT_EOF, (une_position){idx, idx}, 0};
      break;
    }
    #pragma endregion Grouping and Ordering (- NEW, EOF)
    
    #pragma region Operators (+ Logical Equal)
    if (text[idx] == L'=') {
      idx++;
      if (text[idx] == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_EQU, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_SET, (une_position){idx, idx+1}, 0};
      continue;
    }
    if (text[idx] == L'+') {
      tokens[tokens_index++] = (une_token){UNE_TT_ADD, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L'-') {
      tokens[tokens_index++] = (une_token){UNE_TT_SUB, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if (text[idx] == L'*') {
      idx++;
      if (text[idx] == L'*') {
        tokens[tokens_index++] = (une_token){UNE_TT_POW, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_MUL, (une_position){idx, idx+1}, 0};
      continue;
    }
    if (text[idx] == L'/') {
      idx++;
      if (text[idx] == L'/') {
        tokens[tokens_index++] = (une_token){UNE_TT_FDIV, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_DIV, (une_position){idx, idx+1}, 0};
      continue;
    }
    if (text[idx] == L'%') {
      tokens[tokens_index++] = (une_token){UNE_TT_MOD, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    #pragma endregion Operators (+ Logical Equal)

    #pragma region Comparisons (- Logical Equal)
    if (text[idx] == L'!') {
      if (text[idx+1] == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_NEQ, (une_position){idx, idx+2}, 0};
        idx += 2;
        continue;
      }
      // FIXME:
      *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){idx, idx+1}),
          _int=(int)text[idx], _int=0);
      return NULL;
    }
    if (text[idx] == L'>') {
      idx++;
      if (text[idx] == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_GEQ, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_GTR, (une_position){idx, idx+1}, 0};
      continue;
    }
    if (text[idx] == L'<') {
      idx++;
      if (text[idx] == L'=') {
        tokens[tokens_index++] = (une_token){UNE_TT_LEQ, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_LSS, (une_position){idx, idx+1}, 0};
      continue;
    }
    #pragma endregion Comparisons (- Logical Equal)

    #pragma region Whitespace
    if (text[idx] == L' ' || text[idx] == L'\t') {
      idx++;
      continue;
    }
    if (text[idx] == L'\r'
    || text[idx] == L'\n'
    || text[idx] == L';')
    {
      size_t idx_left = idx;
      idx++;
      while (text[idx] == L' '
         || text[idx] == L'\t'
         || text[idx] == L'\r'
         || text[idx] == L'\n'
         || text[idx] == L';') idx++;
      if (tokens_index == 0
      || tokens[tokens_index-1].type != UNE_TT_NEW)
      {
        tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){idx_left, idx}, 0};
      }
      continue;
    }
    #pragma endregion Whitespace

    #pragma region Comment
    if (text[idx] == L'#') {
      while (text[idx] != L'\0' && text[idx] != L'\n') idx++;
      continue;
    }
    #pragma endregion Comment

    #pragma region Illegal Character
    *error = UNE_ERROR_SETX(UNE_ET_UNEXPECTED_CHARACTER, ((une_position){idx, idx+1}),
        _int=(int)text[idx], _int=0);
    return NULL;
    #pragma endregion Illegal Character
  }
  return tokens;
}
#pragma endregion une_lex_wcs

#pragma region une_lex_file
// FIXME: Unfinished!
une_token *une_lex_file(char *path, une_error *error)
{
  size_t tokens_size = UNE_SIZE_BIG;
  une_token *tokens = rmalloc(tokens_size*sizeof(*tokens));
  size_t tokens_index = 0;

  wint_t wchar; /* Can represent any Unicode character +(!) WEOF.
                   This is important when using fgetwc(), as otherwise,
                   WEOF will overflow and be indistinguishable from an
                   actual character.*/
  size_t pos = 0;
  FILE *file = fopen(path, "r,ccs=UTF-8");
  if (file == NULL) WERR(L"File not found");

  while (true) {
    wchar = fgetwc(file);
    
    if (wchar == L'\r') {
      pos++;
      continue;
    }
    if (wchar == WEOF) {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_EOF,
        .pos = (une_position){
          .start = pos,
          .end = pos+1,
        },
      };
      break;
    }

    #pragma region Number
    if (wchar >= L'0' && wchar <= L'9') {

      size_t buffer_size = UNE_SIZE_SMALL;
      wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));

      size_t pos_start = pos;

      while (wchar >= L'0' && wchar <= L'9') {
        
        if (pos-pos_start >= buffer_size-2) { // NUL || '0' NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        buffer[pos-pos_start] = wchar;
        pos++;
        wchar = fgetwc(file);

      }

      // Integer
      if (wchar != L'.') {
        une_token tk = (une_token){
          .type = UNE_TT_INT,
          .pos = (une_position){pos_start, pos},
          .value._int = wcs_to_une_int(buffer),
        };
        free(buffer);
        tokens[tokens_index++] = tk;
        continue;
      }

      // Floating Point Number
      buffer[pos-pos_start] = wchar;
      pos++;
      wchar = fgetwc(file);

      size_t pos_before_decimals = pos;

      while (wchar >= L'0' && wchar <= L'9') {
        
        if (pos-pos_start >= buffer_size-2) { // NUL || '0' NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        buffer[pos-pos_start] = wchar;
        pos++;
        wchar = fgetwc(file);

      }

      if (pos == pos_before_decimals) {
        buffer[pos-pos_start] = L'0';
        pos++;
      }

      buffer[pos-pos_start] = L'\0';
      une_token tk = (une_token){
        .type = UNE_TT_FLT,
        .pos = (une_position){pos_start, pos},
        .value._flt = wcs_to_une_flt(buffer),
      };
      free(buffer);
      tokens[tokens_index++] = tk;
      continue;
      
    }
    #pragma endregion Number

    #pragma region String
    if (wchar == L'"') {
      size_t buffer_size = UNE_SIZE_MEDIUM;
      wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));

      size_t pos_start = pos;
      bool escape = false;
      size_t buffer_index = 0;

      while (true) {
        
        if (buffer_index >= buffer_size-1) { // NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        pos++;
        wchar = fgetwc(file);

        if (wchar == L'\0') {
          *error = UNE_ERROR_SET(UNE_ET_UNTERMINATED_STRING, ((une_position){pos, pos+1}));
          free(buffer);
          return NULL;
        }

        if (wchar == L'\r') continue;

        if (escape) {
          escape = false;
          switch (wchar) {
            case L'\\':
            case L'"':
              buffer[buffer_index++] = wchar;
              continue;
            case L'n':
              buffer[buffer_index++] = L'\n';
              continue;
            default:
              *error = UNE_ERROR_SET(UNE_ET_CANT_ESCAPE_CHAR, ((une_position){pos, pos+1}));
              free(buffer);
              return NULL;
          }
        }

        if (wchar == L'\\') {
          escape = true;
          pos++;
          continue;
        }

        if (wchar == L'"') {
          pos++;
          break;
        }

        buffer[buffer_index++] = wchar;
      }

      buffer[buffer_index] = L'\0';
      une_token tk = (une_token){
        .type = UNE_TT_STR,
        .pos = (une_position){pos_start, pos},
        // DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructed.
        .value._wcs = buffer,
      };
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion String

    #pragma region Identifier
    if ((wchar >= L'a' && wchar <= L'z')
    || (wchar >= L'A' && wchar <= L'Z')
    ||  wchar == L'_') {
      size_t buffer_size = UNE_SIZE_MEDIUM;
      wchar_t *buffer = rmalloc(buffer_size*sizeof(*buffer));

      size_t pos_start = pos;
      bool escape = false;
      size_t buffer_index = 0;

      while ((wchar >= L'a' && wchar <= L'z')
      || (wchar >= L'A' && wchar <= L'Z')
      || (wchar >= L'0' && wchar <= L'9')
      ||  wchar == L'_') {
        
        if (buffer_index >= buffer_size-1) { // NUL
          buffer_size *= 2;
          wchar_t *_buffer = rrealloc(buffer, buffer_size*sizeof(*_buffer));
          buffer = _buffer;
        }

        buffer[buffer_index++] = wchar;
        
        pos++;
        wchar = fgetwc(file);
      }

      buffer[buffer_index] = L'\0';

      une_token tk;

           if (!wcscmp(buffer, L"if"))       tk.type = UNE_TT_IF;
      else if (!wcscmp(buffer, L"elif"))     tk.type = UNE_TT_ELIF;
      else if (!wcscmp(buffer, L"else"))     tk.type = UNE_TT_ELSE;
      else if (!wcscmp(buffer, L"for"))      tk.type = UNE_TT_FOR;
      else if (!wcscmp(buffer, L"from"))     tk.type = UNE_TT_FROM;
      else if (!wcscmp(buffer, L"to"))       tk.type = UNE_TT_TILL;
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

      tk.pos = (une_position){pos_start, pos};
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion Identifier

    #pragma region Grouping and Ordering
    if (wchar == L'(') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_LPAR,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L')') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_RPAR,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L'{') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_LBRC,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L'}') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_RBRC,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L'[') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_LSQB,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L']') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_RSQB,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L',') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_SEP,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    if (wchar == L'\0') {
      tokens[tokens_index++] = (une_token){
        .type = UNE_TT_NEW,
        .pos = (une_position){pos, pos+1},
      };
      continue;
    }
    #pragma endregion Grouping and Ordering
  
  }

  fclose(file);
  return tokens;
}
#pragma endregion une_lex_file
