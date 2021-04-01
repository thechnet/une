/*
test-1.c – CMDRScript C Interpreter Test 1
Modified 2021-03-31

macOS gcc:
clear && gcc test-1.c -o test-1 && ./test-1

TODO:
- read tokens directly from file (cs_lex_file)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define CS_USE_PORTABLE_SWPRINTF

#ifdef CS_USE_PORTABLE_SWPRINTF
  #define SWPRINTF(dest, size, format, ...) swprintf((dest), (size), (format), ##__VA_ARGS__)
#else
  #define SWPRINTF(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

#define SIZE (size_t)4096

#define ERR(s) { printf("\33[91mln %d: %s\33[0m\n", __LINE__, s); exit(1); }

#pragma region wcs_to_int
int wcs_to_int(wchar_t * str)
{
  int out = 0;
  for(int i = 0; str[i] != L'\0'; i++)
  {
    if(str[i] < 48 || str[i] > 57) return 0;
    out = 10 * out + str[i] - 48;
  }
  return out;
}
#pragma endregion wcs_to_int

#pragma region wcs_to_float
// DEBUG: UNSAFE
float wcs_to_float(wchar_t * str)
{
  float out = 0;
  for(int div = 1, i = 0; str[i] != L'\0'; i++)
  {
    if((str[i] < 48 || str[i] > 57) && (str[i] != L'.' || div != 1)) return 0;
    if((str[i] == L'.' && i++) || div > 1) div *= 10;
    out = ((div>1)?1:10) * out + ((float)(str[i]-48))/div;
  }
  return out;
}
#pragma endregion wcs_to_float

#pragma region File
typedef struct _cs_file {
  char * name;
  wchar_t * text;
} cs_file;

void cs_file_read(cs_file * out, char * path)
{
  out->name = malloc((strlen(path) + 1) * sizeof(*(out->name)));
  if(out->name == NULL) ERR("malloc");
  strcpy(out->name, path);
  size_t textChars = 64 + 1; // trailing NUL
  FILE * f = fopen(path, "r,ccs=UTF-8");
  if(f == NULL) ERR("file not found");
  out->text = malloc(textChars * sizeof(*(out->text)));
  if(out->text == NULL) ERR("malloc");
  size_t cursor = 0;
  wchar_t c;
  while(1)
  {
    c = fgetwc(f);
    if(c == L'\r') continue;
    if(cursor >= textChars-1) // NUL
    {
      textChars *= 2;
      wchar_t * _text = realloc(out->text, textChars * sizeof(*_text));
      if(_text == NULL) ERR("realloc");
      out->text = _text;
    }
    if(c == WEOF) break;
    out->text[cursor] = c;
    cursor++;
  }
  fclose(f);
  out->text[cursor] = L'\0';
}
#pragma endregion File

#pragma region Struct: Position
typedef struct _cs_position {
  size_t start;
  size_t end;
} cs_position;
#pragma endregion Position

#pragma region Union: Value, Enum: Value Type
typedef union _cs_value {
    int _int;
    float _float;
    wchar_t * _wcs;
    void * * _p;
} cs_value;

typedef enum _cs_value_type {
  CS_VT_INT, // _int
  CS_VT_FLOAT, // _float
  CS_VT_WCS, // _wcs
  CS_VT_VOIDPP // _p
} cs_value_type;
#pragma endregion Union: Value, Enum: Value Type

#pragma region Token

#pragma region Enum: Token Type
typedef enum _cs_token_type {
  // Data
  CS_TT_INT,  // _int
  CS_TT_FLT,  // _float
  CS_TT_ID,   // _wcs
  CS_TT_STR,  // _wcs

  // Grouping and Ordering
  CS_TT_LPAR,
  CS_TT_RPAR,
  CS_TT_LBRC,
  CS_TT_RBRC,
  CS_TT_LSQB,
  CS_TT_RSQB,
  CS_TT_SEP,
  CS_TT_NEW,
  CS_TT_EOF,

  // Operators
  CS_TT_SET,
  CS_TT_ADD,
  CS_TT_SUB,
  CS_TT_MUL,
  CS_TT_DIV,
  CS_TT_FDIV, // Floor Division
  CS_TT_MOD,
  CS_TT_POW,

  // Comparisons
  CS_TT_EQU,
  CS_TT_NEQ,
  CS_TT_GTR,
  CS_TT_GEQ,
  CS_TT_LSS,
  CS_TT_LEQ,
  CS_TT_NOT,
  CS_TT_AND,
  CS_TT_OR,

  // Clause Keywords
  CS_TT_IF,
  CS_TT_ELIF,
  CS_TT_ELSE,
  CS_TT_FOR,
  CS_TT_FROM,
  CS_TT_TO,
  CS_TT_WHILE,
  CS_TT_DEF,
  CS_TT_RETURN,
  CS_TT_BREAK,
  CS_TT_CONTINUE
} cs_token_type;
#pragma endregion Enum: Token Type

#pragma region Struct: Token
typedef struct _cs_token {
  cs_token_type type;
  cs_position pos;
  cs_value value;
} cs_token;
#pragma endregion Struct: Token

#pragma region Token Type to WCS
const wchar_t * cs_token_type_to_wcs(cs_token_type type)
{
  switch(type)
  {
    // Data
    case CS_TT_INT: return L"INT";
    case CS_TT_FLT: return L"FLT";
    case CS_TT_ID: return L"ID";
    case CS_TT_STR: return L"STR";

    // Grouping and Ordering
    case CS_TT_LPAR: return L"LPAR";
    case CS_TT_RPAR: return L"RPAR";
    case CS_TT_LBRC: return L"LBRC";
    case CS_TT_RBRC: return L"RBRC";
    case CS_TT_LSQB: return L"LSQB";
    case CS_TT_RSQB: return L"RSQB";
    case CS_TT_SEP: return L"SEP";
    case CS_TT_NEW: return L"NEW";
    case CS_TT_EOF: return L"EOF";

    // Operators
    case CS_TT_SET: return L"SET";
    case CS_TT_ADD: return L"ADD";
    case CS_TT_SUB: return L"SUB";
    case CS_TT_MUL: return L"MUL";
    case CS_TT_DIV: return L"DIV";
    case CS_TT_FDIV: return L"FDIV";
    case CS_TT_MOD: return L"MOD";
    case CS_TT_POW: return L"POW";

    // Comparisons
    case CS_TT_EQU: return L"EQU";
    case CS_TT_NEQ: return L"NEQ";
    case CS_TT_GTR: return L"GTR";
    case CS_TT_GEQ: return L"GEQ";
    case CS_TT_LSS: return L"LSS";
    case CS_TT_LEQ: return L"LEQ";
    case CS_TT_NOT: return L"NOT";
    case CS_TT_AND: return L"AND";
    case CS_TT_OR: return L"OR";

    // Clause Keywords
    case CS_TT_IF: return L"IF";
    case CS_TT_ELIF: return L"ELIF";
    case CS_TT_ELSE: return L"ELSE";
    case CS_TT_FOR: return L"FOR";
    case CS_TT_FROM: return L"FROM";
    case CS_TT_TO: return L"TO";
    case CS_TT_WHILE: return L"WHILE";
    case CS_TT_DEF: return L"DEF";
    case CS_TT_RETURN: return L"RETURN";
    case CS_TT_BREAK: return L"BREAK";
    case CS_TT_CONTINUE: return L"CONTINUE";

    default: return L"?";
  }
}
#pragma endregion Token Type to WCS

#pragma region Token to WCS
wchar_t * cs_token_to_wcs(cs_token token)
{
  // DEBUG: NOT DYNAMIC
  wchar_t * str = malloc(512 * sizeof(*str));
  if(str == NULL) ERR("malloc");
  wcscpy(str, L"\33[93m");
  wcscat(str, cs_token_type_to_wcs(token.type));
  wcscat(str, L"\33[0m");
  switch(token.type)
  {
    case CS_TT_INT:
      SWPRINTF(str+wcslen(str), SIZE, L":\33[92m%d\33[0m", token.value._int);
      break;
    
    case CS_TT_FLT:
      SWPRINTF(str+wcslen(str), SIZE, L":\33[92m%.2f\33[0m", token.value._float);
      break;
    
    case CS_TT_STR:
      SWPRINTF(str+wcslen(str), SIZE, L":\33[92m\"%ls\"\33[0m", token.value._wcs);
      break;
    
    case CS_TT_ID:
      SWPRINTF(str+wcslen(str), SIZE, L":\33[92m%ls\33[0m", token.value._wcs);
      break;
    
    default:
      break;
  }
  return str;
}
#pragma endregion Token to WCS

#pragma region Display Tokens
void cs_tokens_display(cs_token * tokens)
{
  size_t i = 0;
  while(1)
  {
    wprintf(L"%ls ", cs_token_to_wcs(tokens[i]));
    if(tokens[i].type == CS_TT_EOF) break;
    i++;
  }
  wprintf(L"\n");
}
#pragma endregion Display Tokens

#pragma endregion Token

#pragma region Node

#pragma region Enum: Node Type
typedef enum _cs_node_type {
  // Data Types
  CS_NT_INT,
  CS_NT_FLT,
  CS_NT_STR,
  CS_NT_LIST,
  CS_NT_BLOCK,

  // Arithmetic Operations
  CS_NT_POW,
  CS_NT_MUL,
  CS_NT_DIV,
  CS_NT_MOD,
  CS_NT_ADD,
  CS_NT_SUB,
  CS_NT_NEG, // Negate

  // Logical Operations
  CS_NT_NOT,
  CS_NT_EQU,
  CS_NT_NEQ,
  CS_NT_GTR,
  CS_NT_GEQ,
  CS_NT_LSS,
  CS_NT_LEQ,
  CS_NT_AND,
  CS_NT_OR,

  // Set, Get
  CS_NT_VARSET,
  CS_NT_VARGET,
  CS_NT_FNDEF,
  CS_NT_FNCALL,

  // Control Flow
  CS_NT_FOR,
  CS_NT_WHILE,
  CS_NT_IF,
  CS_NT_RETURN,
  CS_NT_BREAK,
  CS_NT_CONTINUE,
  CS_NT_END // End of Block or Program
} cs_node_type;
#pragma endregion Enum: Node Type

#pragma region Struct: Node
typedef struct _cs_node {
  cs_node_type type;
  cs_position pos;
  /*
  Every operation requires at least one of these operations.
  - 'primary' holds the second to last thing that comes to mind when thinking
    about a procedure. This could be...
    ... the number that is to be powered.
    ... the true block of an if statement.
    ... the body of a for loop.
    ... the parameters or arguments to a function.
    ... the left operand in an arithmetic operation.
  - 'secondary' holds the last thing that comes to mind when thinking
    about a procedure. This could be...
    ... the number to power by.
    ... the false block of an if statement.
    ... the body of a function definition.
    ... the right operand in an arithmetic operation.
  - 'head' holds additional data about a procedure. This could be...
    ... the condition of an if statement, for, or while loop.
    ... the name of a function definition.
    ... a pointer to a list of node pointers.
  */
  union _content {
    struct _datum {
      cs_value_type type;
      cs_value value;
    } datum;
    struct _branches {
      struct _cs_node * primary;
      struct _cs_node * secondary;
      struct _cs_node * head;
    } branches;
  } content;
} cs_node;
#pragma endregion Struct: Node

#pragma region Node Type to WCS
const wchar_t * cs_node_type_to_wcs(cs_node_type type)
{
  switch(type)
  {
    // Data Types
    case CS_NT_INT: return L"INT";
    case CS_NT_FLT: return L"FLT";
    case CS_NT_STR: return L"STR";
    case CS_NT_LIST: return L"LIST";
    case CS_NT_BLOCK: return L"BLOCK";

    // Arithmetic Operations
    case CS_NT_POW: return L"POW";
    case CS_NT_MUL: return L"MUL";
    case CS_NT_DIV: return L"DIV";
    case CS_NT_MOD: return L"MOD";
    case CS_NT_ADD: return L"ADD";
    case CS_NT_SUB: return L"SUB";
    case CS_NT_NEG: return L"NEG";

    // Logical Operations
    case CS_NT_NOT: return L"NOT";
    case CS_NT_EQU: return L"EQU";
    case CS_NT_NEQ: return L"NEQ";
    case CS_NT_GTR: return L"GTR";
    case CS_NT_GEQ: return L"GEQ";
    case CS_NT_LSS: return L"LSS";
    case CS_NT_LEQ: return L"LEQ";
    case CS_NT_AND: return L"AND";
    case CS_NT_OR: return L"OR";

    // Set, Get
    case CS_NT_VARSET: return L"VARSET";
    case CS_NT_VARGET: return L"VARGET";
    case CS_NT_FNDEF: return L"FNDEF";
    case CS_NT_FNCALL: return L"FNCALL";

    // Control Flow
    case CS_NT_FOR: return L"FOR";
    case CS_NT_WHILE: return L"WHILE";
    case CS_NT_IF: return L"IF";
    case CS_NT_RETURN: return L"RETURN";
    case CS_NT_BREAK: return L"BREAK";
    case CS_NT_CONTINUE: return L"CONTINUE";
    case CS_NT_END: return L"END";
    
    default: return L"?";
  }
}
#pragma endregion Node Type to WCS

#pragma region Node to WCS
wchar_t * cs_node_to_wcs(cs_node node)
{
  switch(node.type)
  {
    case CS_NT_END: {
      cs_node ** nodes = (cs_node**)node.content.branches.head;
      break;
    }

    default:
      break;
  }
  return 0;
}
#pragma endregion Node to WCS

#pragma endregion Node

#pragma region AST
typedef struct _cs_ast {
  size_t idx;
  cs_node * node;
  cs_token * token;
} cs_ast;
#pragma endregion AST

#pragma region Lexer
cs_token * cs_lex_wcs(cs_file file)
{
  size_t maxTokens = 1; // FIXME:
  cs_token * tokens = malloc(maxTokens * sizeof(*tokens));
  if(tokens == NULL) ERR("malloc");
  size_t tokenCount = 0;
  size_t idx = 0;
  while(1)
  {
    if(tokenCount >= maxTokens) // DEBUG: seems to work...
    {
      maxTokens *= 2;
      cs_token * _tokens = realloc(tokens, maxTokens * sizeof(*_tokens));
      if(_tokens == NULL) ERR("realloc");
      tokens = _tokens;
    }
    
    #pragma region Number
    if(file.text[idx] >= L'0' && file.text[idx] <= L'9')
    {
      size_t size = 128;
      wchar_t * number = malloc(size * sizeof(*number));
      if(number == NULL) ERR("malloc");
      size_t idx_left = idx;
      number[0] = file.text[idx];
      idx++;
      while(file.text[idx] >= L'0' && file.text[idx] <= L'9') // Implies [idx] != 0
      {
        number[idx-idx_left] = file.text[idx];
        idx++;
      }
      number[idx-idx_left] = L'\0';
      if(file.text[idx] != L'.')
      {
        cs_token tk = (cs_token){CS_TT_INT, (cs_position){idx_left, idx}};
        tk.value._int = wcs_to_int(number);
        tokens[tokenCount++] = tk;
        continue;
      }
      number[idx-idx_left] = L'.';
      idx++;
      number[idx-idx_left] = L'\0';
      while(file.text[idx] >= L'0' && file.text[idx] <= L'9') // Implies [idx] != 0
      {
        number[idx-idx_left] = file.text[idx];
        idx++;
      }
      if(number[idx-idx_left] == L'\0') ERR("missing number after '.'");
      number[idx-idx_left] = L'\0';
      cs_token tk = {CS_TT_FLT, (cs_position){idx_left, idx}};
      tk.value._float = wcs_to_float(number);
      tokens[tokenCount++] = tk;
      continue;
    }
    #pragma endregion Number
    
    #pragma region String
    if(file.text[idx] == L'"')
    {
      size_t idx_left = idx;
      idx++;
      size_t bufferSize = SIZE; // FIXME:
      wchar_t * buffer = malloc(bufferSize * sizeof(*buffer));
      size_t bufferIndex = 0;
      char bEscape = 0;
      while(1) // DEBUG: hmm all over the place
      {
        if(file.text[idx] == L'\0') ERR("expected string termination");
        if(bufferIndex >= bufferSize)
        {
          // DEBUG: NOT EXTENSIVELY TESTED!
          bufferSize *= 2;
          wchar_t * _buffer = realloc(buffer, bufferSize * sizeof(*_buffer));
          if(_buffer == NULL) ERR("realloc");
          buffer = _buffer;
        }
        if(bEscape)
        {
          bEscape = 0;
          switch(file.text[idx])
          {
            case L'\\':
            case L'"':
              buffer[bufferIndex] = file.text[idx];
              break;
            case L'n':
              buffer[bufferIndex] = L'\n';
              break;
            case L'\r':
            case L'\n':
              idx++;
              continue;
            default:
              ERR("can't escape character");
              break;
          }
        }
        else
        {
          if(file.text[idx] == L'\\')
          {
            bEscape = 1;
            idx++;
            continue;
          }
          if(file.text[idx] == L'"')
          {
            idx++;
            break;
          }
          buffer[bufferIndex] = file.text[idx];
        }
        idx++;
        bufferIndex++;
      }
      buffer[bufferIndex] = L'\0';
      cs_token tk = {CS_TT_STR, (cs_position){idx_left, idx}};
      tk.value._wcs = buffer;
      tokens[tokenCount++] = tk;
      continue;
    }
    #pragma endregion String
    
    #pragma region Identifier (+ Clause Keywords) // FIXME: äöü
    if((file.text[idx] >= L'a' && file.text[idx] <= L'z')
    || (file.text[idx] >= L'A' && file.text[idx] <= L'Z')
    ||  file.text[idx] == L'_')
    {
      size_t idx_left = idx;
      size_t bufferSize = SIZE; // FIXME:
      wchar_t * buffer = malloc(bufferSize * sizeof(*buffer));
      buffer[0] = file.text[idx];
      idx++;
      size_t bufferIndex = 1;
      while((file.text[idx] >= L'a' && file.text[idx] <= L'z')
         || (file.text[idx] >= L'A' && file.text[idx] <= L'Z')
         || (file.text[idx] >= L'0' && file.text[idx] <= L'9')
         ||  file.text[idx] == L'_')
      {
        buffer[bufferIndex] = file.text[idx];
        idx++;
        bufferIndex++;
        if(bufferIndex >= bufferSize)
        {
          // DEBUG: NOT EXTENSIVELY TESTED!
          bufferSize *= 2;
          wchar_t * _buffer = realloc(buffer, bufferSize * sizeof(*_buffer));
          if(_buffer == NULL) ERR("realloc");
          buffer = _buffer;
        }
      }
      buffer[bufferIndex] = L'\0';
      cs_token tk = {0};
           if(!wcscmp(buffer, L"if"))    tk.type = CS_TT_IF;
      else if(!wcscmp(buffer, L"elif"))  tk.type = CS_TT_ELIF;
      else if(!wcscmp(buffer, L"else"))  tk.type = CS_TT_ELSE;
      else if(!wcscmp(buffer, L"for"))   tk.type = CS_TT_FOR;
      else if(!wcscmp(buffer, L"from"))  tk.type = CS_TT_FROM;
      else if(!wcscmp(buffer, L"to"))    tk.type = CS_TT_TO;
      else if(!wcscmp(buffer, L"while")) tk.type = CS_TT_WHILE;
      else if(!wcscmp(buffer, L"def"))   tk.type = CS_TT_DEF;
      else if(!wcscmp(buffer, L"return"))  tk.type = CS_TT_RETURN;
      else if(!wcscmp(buffer, L"break"))    tk.type = CS_TT_BREAK;
      else if(!wcscmp(buffer, L"continue")) tk.type = CS_TT_CONTINUE;
      else
      {
        tk.type = CS_TT_ID;
        tk.value._wcs = buffer;
      }
      tk.pos = (cs_position){idx_left, idx};
      tokens[tokenCount++] = tk;
      continue;
    }
    #pragma endregion Identifier (+ Clause Keywords)
    
    #pragma region Grouping and Ordering (- NEW)
    if(file.text[idx] == L'(')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_LPAR, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L')')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_RPAR, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'{')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_LBRC, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'}')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_RBRC, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'[')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_LSQB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L']')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_RSQB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L',')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_SEP, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'\0')
    {
      tokens[tokenCount] = (cs_token){CS_TT_EOF, (cs_position){idx, idx}, 0};
      break;
    }
    #pragma endregion Grouping and Ordering (- NEW, EOF)
    
    #pragma region Operators (+ Logical Equal)
    if(file.text[idx] == L'=')
    {
      idx++;
      if(file.text[idx] == L'=')
      {
        tokens[tokenCount++] = (cs_token){CS_TT_EQU, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokenCount++] = (cs_token){CS_TT_SET, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(file.text[idx] == L'+')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_ADD, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'-')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_SUB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'*')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_MUL, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'/')
    {
      idx++;
      if(file.text[idx] == L'/')
      {
        tokens[tokenCount++] = (cs_token){CS_TT_FDIV, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokenCount++] = (cs_token){CS_TT_DIV, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(file.text[idx] == L'%')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_MOD, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'^')
    {
      tokens[tokenCount++] = (cs_token){CS_TT_POW, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    #pragma endregion Operators (+ Logical Equal)

    #pragma region Comparisons (- Logical Equal)
    if(file.text[idx] == L'!')
    {
      if(file.text[idx+1] == L'=') // FIXME: UNSAFE
      {
        tokens[tokenCount++] = (cs_token){CS_TT_NEQ, (cs_position){idx, idx+2}, 0};
        idx += 2;
        continue;
      }
    }
    if(file.text[idx] == L'>')
    {
      idx++;
      if(file.text[idx] == L'=')
      {
        tokens[tokenCount++] = (cs_token){CS_TT_GEQ, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokenCount++] = (cs_token){CS_TT_GTR, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(file.text[idx] == L'<')
    {
      idx++;
      if(file.text[idx] == L'=')
      {
        tokens[tokenCount++] = (cs_token){CS_TT_LEQ, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokenCount++] = (cs_token){CS_TT_LSS, (cs_position){idx, idx+1}, 0};
      continue;
    }
    #pragma endregion Comparisons (- Logical Equal)

    #pragma region Whitespace
    if(file.text[idx] == L' ' || file.text[idx] == L'\t')
    {
      idx++;
      continue;
    }
    if(file.text[idx] == L'\r'
    || file.text[idx] == L'\n'
    || file.text[idx] == L';')
    {
      size_t idx_left = idx;
      idx++;
      while(file.text[idx] == L' '
         || file.text[idx] == L'\t'
         || file.text[idx] == L'\r'
         || file.text[idx] == L'\n'
         || file.text[idx] == L';')
      {
        idx++;
      }
      tokens[tokenCount++] = (cs_token){CS_TT_NEW, (cs_position){idx_left, idx}, 0};
      continue;
    }
    #pragma endregion Whitespace

    #pragma region Unknown Character
    ERR("unknown character");
    #pragma endregion Unknown Character
  }
  return tokens;
}
#pragma endregion Lexer

#pragma region Parser

cs_node * cs_parse_program(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index);

//token_index += cs_parse_expression(tokens, node.branches.primary);

//node.branches.primary = cs_parse_expression(tokens, &token_index);

#pragma region Parse Program
// Return Node: CS_NT_BLOCK
// Return Type: Datum
cs_node * cs_parse_program(cs_token * tokens, size_t * token_index)
{
  // Initialize Return Value
  size_t block_size = 4; // amt of cs_node*
  cs_node ** block = malloc(block_size * sizeof(*block));
  if(block == NULL) ERR("malloc");
  
  // Populate Return Value
  size_t token_index_start = *token_index;
  size_t block_index = 0;
  while(1)
  {
    if(block_index >= block_size)
    {
      block_size *= 2;
      cs_node ** _block = realloc(block, block_size * sizeof(_block));
      if(_block == NULL) ERR("realloc");
      block = _block;
      printf("block doubled\n"); // DEBUG:
    }
    if(tokens[*token_index].type == CS_TT_EOF) break;
    block[block_index] = cs_parse_statement(tokens, token_index);
    block_index++;
  }
  cs_node * end = malloc(sizeof(*end));
  if(end == NULL) ERR("malloc");
  end->type = CS_NT_END;
  block[block_index] = end;

  // Create Return Node
  cs_node * node = malloc(sizeof(*node));
  if(block == NULL) ERR("malloc");
  node->type = CS_NT_BLOCK;
  node->pos.start = tokens[token_index_start].pos.start;
  node->pos.end = tokens[*token_index].pos.end;
  node->content.datum.value._p = (void**)block;

  // Return Node
  return node;
}
#pragma endregion Parse Program

#pragma region Parse Statement
// Return Node: ?
// Return Type: ?
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index)
{
  cs_node * node = malloc(sizeof(*node));
  if(node == NULL) ERR("malloc");
  node->type = CS_NT_INT;
  node->pos = (cs_position){0, 0};
  node->content.datum.value._int = tokens[*token_index].value._int;
  (*token_index)++;
  return node;
}
#pragma endregion Parse Statement

#pragma endregion Parser

int main(int argc, char * argv[])
{
  cs_file file = {0};
  cs_file_read(&file, "./test.txt");
  cs_token * tokens = cs_lex_wcs(file);
  cs_tokens_display(tokens);
  
  size_t token_index = 0;
  cs_node * ast = cs_parse_program(tokens, &token_index);

  cs_node ** list = (cs_node**)ast->content.datum.value._p;

  int i = 0;
  while(1)
  {
    if(list[i]->type == CS_NT_END) break;
    wprintf(L"[%d] %d\n", i, list[i]->content.datum.value._p);
    i++;
  }
}

/*cs_node * num46 = malloc(sizeof(*num46));
  num46->type = CS_NT_INT; num46->content.datum.value._int = 46;
  cs_node * num420 = malloc(sizeof(*num420));
  num420->type = CS_NT_INT; num420->content.datum.value._int = 420;
  cs_node * eob1 = malloc(sizeof(*eob1));
  eob1->type = CS_NT_EOB;
  cs_node ** block1 = malloc(3 * sizeof(*block1));
  block1[0] = num46;
  block1[1] = num420;
  block1[2] = eob1;

  for(int i=0; i<3; i++) wprintf(L"%ls ", cs_node_type_to_wcs(block1[i]->type));
  wprintf(L"\n");

  cs_node * list1 = malloc(sizeof(*list1));
  list1->type = CS_NT_LIST;
  list1->content.datum.value._p = (void**)block1;
  cs_node * eob2 = malloc(sizeof(*eob2));
  eob2->type = CS_NT_EOB;
  cs_node ** block2 = malloc(3 * sizeof(*block2));
  block2[0] = list1;
  block2[1] = eob2;

  for(int i=0; i<2; i++) wprintf(L"%ls ", cs_node_type_to_wcs(block2[i]->type));
  wprintf(L"\n\n");

  cs_node ** block = (cs_node**)(block2[0]->content.datum.value._p);
  wprintf(L"%d", block[1]->content.datum.value._int);*/