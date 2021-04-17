/*
lexer.c - Une
Updated 2021-04-17
*/

#include "lexer.h"

une_token *une_lex_wcs(wchar_t *text, une_error *error)
{
  size_t tokens_size = UNE_SIZE_MEDIUM; // FIXME:
  une_token *tokens = malloc(tokens_size *sizeof(*tokens));
  if(tokens == NULL) WERR(L"Out of memory.");
  size_t tokens_index = 0;
  size_t idx = 0;
  while(true)
  {
    if(tokens_index+1 >= tokens_size)
    {
      tokens_size *= 2;
      une_token *_tokens = realloc(tokens, tokens_size *sizeof(*_tokens));
      if(_tokens == NULL) WERR(L"Out of memory.");
      tokens = _tokens;
      wprintf(L"Warning: Tokens doubled\n");
    }
    
    #pragma region Number
    if(text[idx] >= L'0' && text[idx] <= L'9')
    {
      size_t size = UNE_SIZE_SMALL;
      wchar_t *buffer = malloc(size *sizeof(*buffer));
      if(buffer == NULL) WERR(L"Out of memory.");
      size_t idx_left = idx;
      buffer[0] = text[idx];
      idx++;
      // FIXME: NOT DYNAMIC
      while(text[idx] >= L'0' && text[idx] <= L'9') // Implies [idx] != 0
      {
        buffer[idx-idx_left] = text[idx];
        idx++;
      }
      buffer[idx-idx_left] = L'\0';
      if(text[idx] != L'.')
      {
        une_token tk = (une_token){UNE_TT_INT, (une_position){idx_left, idx}};
        tk.value._int = wcs_to_une_int(buffer);
        free(buffer);
        tokens[tokens_index++] = tk;
        continue;
      }
      buffer[idx-idx_left] = L'.';
      idx++;
      size_t idx_before_decimals = idx;
      // FIXME: NOT DYNAMIC
      while(text[idx] >= L'0' && text[idx] <= L'9') // Implies [idx] != 0
      {
        buffer[idx-idx_left] = text[idx];
        idx++;
      }
      if(idx == idx_before_decimals)
      {
        buffer[idx-idx_left] = L'0';
        idx++;
      }
      buffer[idx-idx_left] = L'\0';
      une_token tk = {UNE_TT_FLT, (une_position){idx_left, idx}};
      tk.value._flt = wcs_to_une_flt(buffer);
      free(buffer);
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion Number
    
    #pragma region String
    if(text[idx] == L'"')
    {
      size_t idx_left = idx;
      idx++;
      size_t string_size = UNE_SIZE_MEDIUM; // FIXME:
      wchar_t *string = malloc(string_size *sizeof(*string));
      if(string == NULL) WERR(L"Out of memory.");
      size_t string_index = 0;
      char escape = false;
      while(true)
      {
        if(text[idx] == L'\0') WERR(L"expected string termination");
        if(string_index >= string_size)
        {
          string_size *= 2;
          wchar_t *_string = realloc(string, string_size *sizeof(*_string));
          if(_string == NULL) WERR(L"Out of memory.");
          string = _string;
        }
        if(escape)
        {
          escape = false;
          switch(text[idx])
          {
            case L'\\':
            case L'"':
              string[string_index] = text[idx];
              break;
            case L'n':
              string[string_index] = L'\n';
              break;
            case L'\r':
            case L'\n':
              idx++;
              continue;
            default:
              WERR(L"can't escape character");
              break;
          }
        }
        else
        {
          if(text[idx] == L'\\')
          {
            escape = true;
            idx++;
            continue;
          }
          if(text[idx] == L'"')
          {
            idx++;
            break;
          }
          string[string_index] = text[idx];
        }
        idx++;
        string_index++;
      }
      string[string_index] = L'\0';
      une_token tk = {UNE_TT_STR, (une_position){idx_left, idx}};
      // DOC: Memory Management: This is where strings referenced during tokenization and parsing are constructued.
      tk.value._wcs = string;
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion String
    
    #pragma region Identifier (+ Clause Keywords)
    if((text[idx] >= L'a' && text[idx] <= L'z')
    || (text[idx] >= L'A' && text[idx] <= L'Z')
    ||  text[idx] == L'_')
    {
      size_t idx_left = idx;
      size_t id_size = UNE_SIZE_MEDIUM; // FIXME:
      wchar_t *id = malloc(id_size *sizeof(*id));
      if(id == NULL) WERR(L"Out of memory.");
      id[0] = text[idx];
      idx++;
      size_t id_index = 1;
      while((text[idx] >= L'a' && text[idx] <= L'z')
         || (text[idx] >= L'A' && text[idx] <= L'Z')
         || (text[idx] >= L'0' && text[idx] <= L'9')
         ||  text[idx] == L'_')
      {
        id[id_index] = text[idx];
        idx++;
        id_index++;
        if(id_index >= id_size)
        {
          id_size *= 2;
          wchar_t *_id = realloc(id, id_size *sizeof(*_id));
          if(_id == NULL) WERR(L"Out of memory.");
          id = _id;
        }
      }
      id[id_index] = L'\0';
      une_token tk = {0};
           if(!wcscmp(id, L"if"))       tk.type = UNE_TT_IF;
      else if(!wcscmp(id, L"elif"))     tk.type = UNE_TT_ELIF;
      else if(!wcscmp(id, L"else"))     tk.type = UNE_TT_ELSE;
      else if(!wcscmp(id, L"for"))      tk.type = UNE_TT_FOR;
      else if(!wcscmp(id, L"from"))     tk.type = UNE_TT_FROM;
      else if(!wcscmp(id, L"to"))       tk.type = UNE_TT_TO;
      else if(!wcscmp(id, L"while"))    tk.type = UNE_TT_WHILE;
      else if(!wcscmp(id, L"def"))      tk.type = UNE_TT_DEF;
      else if(!wcscmp(id, L"return"))   tk.type = UNE_TT_RETURN;
      else if(!wcscmp(id, L"break"))    tk.type = UNE_TT_BREAK;
      else if(!wcscmp(id, L"continue")) tk.type = UNE_TT_CONTINUE;
      else if(!wcscmp(id, L"and"))      tk.type = UNE_TT_AND;
      else if(!wcscmp(id, L"or"))       tk.type = UNE_TT_OR;
      else if(!wcscmp(id, L"not"))      tk.type = UNE_TT_NOT;
      else
      {
        tk.type = UNE_TT_ID;
        tk.value._wcs = id;
      }
      if(tk.type != UNE_TT_ID) free(id);
      tk.pos = (une_position){idx_left, idx};
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion Identifier (+ Clause Keywords)
    
    #pragma region Grouping and Ordering (- NEW)
    if(text[idx] == L'(')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_LPAR, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L')')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_RPAR, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'{')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_LBRC, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'}')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_RBRC, (une_position){idx, idx+1}, 0};
      tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'[')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_LSQB, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L']')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_RSQB, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L',')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_SEP, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'\0')
    {
      if(tokens_index == 0 || tokens[tokens_index-1].type != UNE_TT_NEW)
      {
        tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){idx, idx+1}, 0};
      }
      tokens[tokens_index] = (une_token){UNE_TT_EOF, (une_position){idx, idx}, 0};
      break;
    }
    #pragma endregion Grouping and Ordering (- NEW, EOF)
    
    #pragma region Operators (+ Logical Equal)
    if(text[idx] == L'=')
    {
      idx++;
      if(text[idx] == L'=')
      {
        tokens[tokens_index++] = (une_token){UNE_TT_EQU, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_SET, (une_position){idx, idx+1}, 0};
      continue;
    }
    if(text[idx] == L'+')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_ADD, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'-')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_SUB, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'*')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_MUL, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'/')
    {
      idx++;
      if(text[idx] == L'/')
      {
        tokens[tokens_index++] = (une_token){UNE_TT_FDIV, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_DIV, (une_position){idx, idx+1}, 0};
      continue;
    }
    if(text[idx] == L'%')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_MOD, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'^')
    {
      tokens[tokens_index++] = (une_token){UNE_TT_POW, (une_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    #pragma endregion Operators (+ Logical Equal)

    #pragma region Comparisons (- Logical Equal)
    if(text[idx] == L'!')
    {
      if(text[idx+1] == L'=')
      {
        tokens[tokens_index++] = (une_token){UNE_TT_NEQ, (une_position){idx, idx+2}, 0};
        idx += 2;
        continue;
      }
      // FIXME:
      error->type = UNE_ET_ILLEGAL_CHARACTER;
      error->pos = (une_position){idx, idx+1};
      error->values[0]._int = (int)text[idx];
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      return NULL;
    }
    if(text[idx] == L'>')
    {
      idx++;
      if(text[idx] == L'=')
      {
        tokens[tokens_index++] = (une_token){UNE_TT_GEQ, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_GTR, (une_position){idx, idx+1}, 0};
      continue;
    }
    if(text[idx] == L'<')
    {
      idx++;
      if(text[idx] == L'=')
      {
        tokens[tokens_index++] = (une_token){UNE_TT_LEQ, (une_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (une_token){UNE_TT_LSS, (une_position){idx, idx+1}, 0};
      continue;
    }
    #pragma endregion Comparisons (- Logical Equal)

    #pragma region Whitespace
    if(text[idx] == L' ' || text[idx] == L'\t')
    {
      idx++;
      continue;
    }
    if(text[idx] == L'\r'
    || text[idx] == L'\n'
    || text[idx] == L';')
    {
      size_t idx_left = idx;
      idx++;
      while(text[idx] == L' '
         || text[idx] == L'\t'
         || text[idx] == L'\r'
         || text[idx] == L'\n'
         || text[idx] == L';') idx++;
      if(tokens_index == 0
      || tokens[tokens_index-1].type != UNE_TT_NEW)
      {
        tokens[tokens_index++] = (une_token){UNE_TT_NEW, (une_position){idx_left, idx}, 0};
      }
      continue;
    }
    #pragma endregion Whitespace

    #pragma region Illegal Character
    error->type = UNE_ET_ILLEGAL_CHARACTER;
    error->pos = (une_position){idx, idx+1};
    error->values[0]._int = (int)text[idx];
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    return NULL;
    #pragma endregion Illegal Character
  }
  return tokens;
}