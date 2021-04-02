/*
test-1.c – CMDRScript C Interpreter Test 1
Modified 2021-04-02

macOS
clear && gcc test-1.c -o test-1 && ./test-1

Windows
cls & tcc test-1.c -o test-1.exe && test-1.exe
cls & gcc test-1.c -o test-1.exe && test-1.exe

TODO:
- read tokens directly from file (cs_lex_file)
- store error in file?
- negate FIXME:
*/

#pragma region #

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define CS_PORTABLE_SWPRINTF
//#define CS_WINDOWS

#ifdef CS_WINDOWS
  #include <windows.h>
  #undef wchar_t
  #define wchar_t WCHAR
#endif

#ifdef CS_PORTABLE_SWPRINTF
  #define SWPRINTF(dest, size, format, ...) swprintf((dest), (size), (format), ##__VA_ARGS__)
#else
  #define SWPRINTF(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

#define SIZE (size_t)4096

#define WERR(s) { wprintf(L"\33[91mln %d: %ls\33[0m\n", __LINE__, s); exit(1); }

#pragma endregion #

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
  if(out->name == NULL) WERR(L"malloc");
  strcpy(out->name, path);
  size_t textChars = 64 + 1; // trailing NUL
  FILE * f = fopen(path, "r,ccs=UTF-8");
  if(f == NULL) WERR(L"file not found");
  out->text = malloc(textChars * sizeof(*(out->text)));
  if(out->text == NULL) WERR(L"malloc");
  size_t cursor = 0;
  wint_t c; /* Can represent any Unicode character + (!) WEOF.
               This is important when using fgetwc(), as otherwise,
               WEOF will overflow and be indistinguishable from an
               actual character.*/
  while(1)
  {
    c = fgetwc(f);
    if(c == L'\r') continue;
    if(cursor >= textChars-1) // NUL
    {
      textChars *= 2;
      wchar_t * _text = realloc(out->text, textChars * sizeof(*_text));
      if(_text == NULL) WERR(L"realloc");
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

#pragma region Position
typedef struct _cs_position {
  size_t start;
  size_t end;
} cs_position;
#pragma endregion Position

#pragma region Value
typedef union _cs_value {
    int _int;
    float _float;
    wchar_t * _wcs;
    void ** _p;
} cs_value;

typedef enum _cs_value_type {
  CS_VT_INT, // _int
  CS_VT_FLOAT, // _float
  CS_VT_WCS, // _wcs
  CS_VT_VOIDPP // _p
} cs_value_type;
#pragma endregion Union: Value, Enum: Value Type

#pragma region Datum
typedef struct _cs_datum {
  cs_value_type type;
  cs_value value;
} cs_datum;

/*wchar_t * cs_datum_to_wcs(cs_datum datum)
{
  wchar_t * wcs = malloc(SIZE * sizeof(*wcs));
  switch(datum.type)
  {
    case CS_VT_INT:
      SWPRINTF(wcs, SIZE, L"%d", datum.value._int);
      break;
    case CS_VT_FLOAT:
      SWPRINTF(wcs, SIZE, L"%f", datum.value._float);
      break;
    case CS_VT_WCS:
      SWPRINTF(wcs, SIZE, L"%ls", datum.value._wcs);
      break;
    case CS_VT_VOIDPP:
      SWPRINTF(wcs, SIZE, L"%p", datum.value._p);
      break;
    default:
      SWPRINTF(wcs, SIZE, L"Unknown Datum Value Type");
      break;
  }
  return wcs;
}*/
#pragma endregion Datum

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
  if(str == NULL) WERR(L"malloc");
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
  CS_NT_ID, // FIXME: Unsure about this one...
  CS_NT_LIST,

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
  CS_NT_INDEX,

  // Control Flow
  CS_NT_FOR,
  CS_NT_WHILE,
  CS_NT_IF,
  CS_NT_RETURN,
  CS_NT_BREAK,
  CS_NT_CONTINUE,
  CS_NT_BLOCK,
  CS_NT_END // End of Block or Program
} cs_node_type;
#pragma endregion Enum: Node Type

#pragma region Struct: Node
typedef struct _cs_node {
  cs_node_type type;
  cs_position pos;
  union _content {
    cs_datum datum;
    union _branch {
      struct __for {
        struct _cs_node * counter;
        struct _cs_node * from;
        struct _cs_node * to;
        struct _cs_node * body;
      } _for;
      struct __while {
        struct _cs_node * condition;
        struct _cs_node * body;
      } _while;
      struct __if {
        struct _cs_node * condition;
        struct _cs_node * truebody;
        struct _cs_node * falsebody;
      } _if;
      struct __fndef {
        struct _cs_node * name;
        struct _cs_node * params;
        struct _cs_node * body;
      } _fndef;
      struct __binop {
        struct _cs_node * left;
        struct _cs_node * right;
      } _binop;
      struct __unop {
        struct _cs_node * center;
      } _unop;
    } branch;
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
    case CS_NT_INDEX: return L"INDEX";

    // Control Flow
    case CS_NT_FOR: return L"FOR";
    case CS_NT_WHILE: return L"WHILE";
    case CS_NT_IF: return L"IF";
    case CS_NT_RETURN: return L"RETURN";
    case CS_NT_BREAK: return L"BREAK";
    case CS_NT_CONTINUE: return L"CONTINUE";
    case CS_NT_BLOCK: return L"BLOCK";
    case CS_NT_END: return L"END";
    
    default: return L"?";
  }
}
#pragma endregion Node Type to WCS

#pragma region Node to WCS
wchar_t * cs_node_to_wcs(cs_node * node)
{
  // FIXME: NOT DYNAMIC
  #define CS_NODE_DISPLAY_BUFFER_SIZE 32767
  if(node == NULL) return L"\33[90m-\33[0m";
  wchar_t * buffer = malloc(32767 * sizeof(*buffer));
  buffer[0] = L'\0';
  switch(node->type)
  {
    case CS_NT_BLOCK: {
      cs_node ** list = (cs_node**)node->content.datum.value._p;
      int offset = SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m%ls", cs_node_to_wcs(list[0]));
      int i = 1;
      while(list[i]->type != CS_NT_END)
      {
        offset += SWPRINTF(buffer+offset, CS_NODE_DISPLAY_BUFFER_SIZE, L"\n\33[0m%ls", cs_node_to_wcs(list[i]));
        i++;
      }
      break; }
    
    case CS_NT_INT:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[95mINT\33[0m:\33[35m%d\33[0m", node->content.datum.value._int);
      break;
    
    case CS_NT_FLT:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[95mFLOAT\33[0m:\33[35m%.2f\33[0m", node->content.datum.value._float);
      break;
    
    case CS_NT_STR:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[95mSTR\33[0m:\33[35m\"%ls\"\33[0m", node->content.datum.value._wcs);
      break;
    
    case CS_NT_ID:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[95mID\33[0m:\33[35m%ls\33[0m", node->content.datum.value._wcs);
      break;
    
    case CS_NT_LIST: {
      cs_node ** list = (cs_node**)node->content.datum.value._p;
      int offset = SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m[%ls", cs_node_to_wcs(list[0]));
      int i = 1;
      while(list[i]->type != CS_NT_END)
      {
        offset += SWPRINTF(buffer+offset, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m, %ls", cs_node_to_wcs(list[i]));
        i++;
      }
      SWPRINTF(buffer+offset, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m]");
      break; }

    case CS_NT_ADD:
    case CS_NT_SUB:
    case CS_NT_MUL:
    case CS_NT_DIV:
    case CS_NT_MOD:
    case CS_NT_POW:
    case CS_NT_INDEX:
    case CS_NT_EQU:
    case CS_NT_NEQ:
    case CS_NT_GTR:
    case CS_NT_GEQ:
    case CS_NT_LSS:
    case CS_NT_LEQ:
    case CS_NT_AND:
    case CS_NT_OR:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m(\33[96m%ls\33[0m %ls\33[0m, %ls\33[0m)", cs_node_type_to_wcs(node->type), cs_node_to_wcs(node->content.branch._binop.left), cs_node_to_wcs(node->content.branch._binop.right));
      break;
    
    case CS_NT_FOR:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m(\33[96mFOR\33[0m %ls\33[0m, %ls\33[0m, %ls\33[0m, %ls\33[0m)", cs_node_to_wcs(node->content.branch._for.counter), cs_node_to_wcs(node->content.branch._for.from), cs_node_to_wcs(node->content.branch._for.to), cs_node_to_wcs(node->content.branch._for.body));
      break;
    
    case CS_NT_WHILE:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m(\33[96mWHILE\33[0m %ls\33[0m, %ls\33[0m", cs_node_to_wcs(node->content.branch._while.condition), cs_node_to_wcs(node->content.branch._while.body));
      break;

    case CS_NT_IF:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m(\33[96mIF\33[0m %ls\33[0m, %ls\33[0m, %ls\33[0m)", cs_node_to_wcs(node->content.branch._if.condition), cs_node_to_wcs(node->content.branch._if.truebody), cs_node_to_wcs(node->content.branch._if.falsebody));
      break;
    
    case CS_NT_FNDEF:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m(\33[96mDEF\33[0m %ls\33[0m, %ls\33[0m, %ls\33[0m)", cs_node_to_wcs(node->content.branch._fndef.name), cs_node_to_wcs(node->content.branch._fndef.params), cs_node_to_wcs(node->content.branch._fndef.body));
      break;

    default:
      SWPRINTF(buffer, CS_NODE_DISPLAY_BUFFER_SIZE, L"\33[0m(UNKNOWN NODE)");
  }
  return buffer;
}
#pragma endregion Node to WCS

#pragma endregion Node

#pragma region Error

#pragma region Enum: Error Type
typedef enum _cs_error_type {
  CS_ET_NO_ERROR, // Debugging
  CS_ET_EXPECTED_TOKEN,
  CS_ET_UNEXPECTED_TOKEN,
  CS_ET_ILLEGAL_CHARACTER
} cs_error_type;
#pragma endregion Enum: Error Type

#pragma region Struct: Error
typedef struct _cs_error {
  cs_error_type type;
  cs_position pos;
  cs_datum info;
  char __file__[4096];     // Debugging
  size_t __line__; // ''
} cs_error;
#pragma endregion Struct: Error

#pragma region Error Info to WCS
wchar_t * cs_error_info_to_wcs(cs_error_type type, cs_datum info)
{
  wchar_t * wcs = malloc(SIZE * sizeof(*wcs));
  switch(type)
  {
    case CS_ET_NO_ERROR:
      SWPRINTF(wcs, SIZE, L"Undefined Error!");
      break;
    
    case CS_ET_EXPECTED_TOKEN:
      SWPRINTF(wcs, SIZE, L"Expected Token: %ls", cs_token_type_to_wcs((cs_token_type)info.value._int));
      break;
    
    case CS_ET_UNEXPECTED_TOKEN:
      SWPRINTF(wcs, SIZE, L"Unexpected Token: %ls", cs_token_type_to_wcs((cs_token_type)info.value._int));
      break;
    
    case CS_ET_ILLEGAL_CHARACTER:
      SWPRINTF(wcs, SIZE, L"Illegal Character: '%lc'", (wchar_t)info.value._int);
      break;
    
    default:
      return L"Unknown Error";
  }
  return wcs;
}
#pragma endregion Error Info to WCS

cs_error error = {CS_ET_NO_ERROR};

#pragma region Display Error
void cs_error_display(cs_file file)
{
  int line = 1;
  int line_begin = 0;
  int line_end = wcslen(file.text); /* Not entirely sure about this... Basically,
                                       if the error is raised right at the end of
                                       the file, its position will be outside the
                                       file, meaning bLocationFound can never be
                                       set, meaning line_end remains at 0. */
  char bLocationFound = 0;
  for(int i=0; i<wcslen(file.text)+1 /* Allows catching end of string */ ; i++)
  {
    if(bLocationFound)
    {
      if(file.text[i] == L'\n' || file.text[i] == L'\0')
      {
        line_end = i-1;
        break;
      }
    }
    else
    {
      if(i == error.pos.start)
      {
        bLocationFound = 1;
        continue;
      }
      if(file.text[i] == L'\n')
      {
        line++;
        line_begin = i+1;
      }
    }
  }
  wprintf(L"\33[94mFile %hs, Line %d (From %hs at %d):\n%.*ls\n\33[%dC%ls^\n%ls\n\n\33[90merror.pos.start: %d\nerror.pos.end: %d\nline_begin: %d\nline_end: %d\33[0m",
          file.name, line, error.__file__, error.__line__,
          line_end-line_begin,file.text+line_begin,
          error.pos.start-line_begin, (error.pos.start-line_begin > 0) ? L"" : L"\33[D",
          cs_error_info_to_wcs(error.type, error.info),
          error.pos.start, error.pos.end, line_begin, line_end); // Debugging
}
#pragma endregion Display Error

#pragma endregion Error

#pragma region Lexer
cs_token * cs_lex_wcs(cs_file file)
{
  size_t maxTokens = 1; // FIXME:
  cs_token * tokens = malloc(maxTokens * sizeof(*tokens));
  if(tokens == NULL) WERR(L"malloc");
  size_t tokenCount = 0;
  size_t idx = 0;
  while(1)
  {
    if(tokenCount+1 >= maxTokens) // DEBUG: seems to work...
    {
      maxTokens *= 2;
      cs_token * _tokens = realloc(tokens, maxTokens * sizeof(*_tokens));
      if(_tokens == NULL) WERR(L"realloc");
      tokens = _tokens;
      //wprintf(L"-> Tokens doubled\n");
    }
    
    #pragma region Number
    if(file.text[idx] >= L'0' && file.text[idx] <= L'9')
    {
      size_t size = 128;
      wchar_t * number = malloc(size * sizeof(*number));
      if(number == NULL) WERR(L"malloc");
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
      size_t idx_before_decimals = idx;
      while(file.text[idx] >= L'0' && file.text[idx] <= L'9') // Implies [idx] != 0
      {
        number[idx-idx_left] = file.text[idx];
        idx++;
      }
      if(idx == idx_before_decimals) WERR(L"missing number after '.'");
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
        if(file.text[idx] == L'\0') WERR(L"expected string termination");
        if(bufferIndex >= bufferSize)
        {
          // DEBUG: NOT EXTENSIVELY TESTED!
          bufferSize *= 2;
          wchar_t * _buffer = realloc(buffer, bufferSize * sizeof(*_buffer));
          if(_buffer == NULL) WERR(L"realloc");
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
              WERR(L"can't escape character");
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
          if(_buffer == NULL) WERR(L"realloc");
          buffer = _buffer;
        }
      }
      buffer[bufferIndex] = L'\0';
      cs_token tk = {0};
           if(!wcscmp(buffer, L"if"))       tk.type = CS_TT_IF;
      else if(!wcscmp(buffer, L"elif"))     tk.type = CS_TT_ELIF;
      else if(!wcscmp(buffer, L"else"))     tk.type = CS_TT_ELSE;
      else if(!wcscmp(buffer, L"for"))      tk.type = CS_TT_FOR;
      else if(!wcscmp(buffer, L"from"))     tk.type = CS_TT_FROM;
      else if(!wcscmp(buffer, L"to"))       tk.type = CS_TT_TO;
      else if(!wcscmp(buffer, L"while"))    tk.type = CS_TT_WHILE;
      else if(!wcscmp(buffer, L"def"))      tk.type = CS_TT_DEF;
      else if(!wcscmp(buffer, L"return"))   tk.type = CS_TT_RETURN;
      else if(!wcscmp(buffer, L"break"))    tk.type = CS_TT_BREAK;
      else if(!wcscmp(buffer, L"continue")) tk.type = CS_TT_CONTINUE;
      else if(!wcscmp(buffer, L"and"))      tk.type = CS_TT_AND;
      else if(!wcscmp(buffer, L"or"))       tk.type = CS_TT_OR;
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
      tokens[tokenCount++] = (cs_token){CS_TT_NEW, (cs_position){idx, idx}, 0}; // FIXME: Kind of weird...
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

    #pragma region Illegal Character
    error.type = CS_ET_ILLEGAL_CHARACTER;
    error.pos = (cs_position){idx, idx+1};
    error.info.value._int = (int)file.text[idx];
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
    #pragma endregion Illegal Character
  }
  return tokens;
}
#pragma endregion Lexer

#pragma region Parser

cs_node * cs_parse_block(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_statements(cs_token * tokens, size_t * token_index, cs_token_type eob_token);
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_if(cs_token * tokens, size_t * token_index, cs_token_type start_token);
cs_node * cs_parse_for(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_while(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_def(cs_token * tokens, size_t * token_index);
// cs_node * cs_parse_return(cs_token * tokens, size_t * token_index); //?
cs_node * cs_parse_conditions(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_condition(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_expression(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_term(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_power(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_index(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_atom(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_expression_sequence(cs_token * tokens, size_t * token_index, cs_token_type eos_token);
cs_node * cs_parse_id_sequence(cs_token * tokens, size_t * token_index, cs_token_type eos_token);
cs_node * cs_parse_id(cs_token * tokens, size_t * token_index);

#pragma region Parse Block
cs_node * cs_parse_block(cs_token * tokens, size_t * token_index)
{
  cs_node * block;
  if(tokens[*token_index].type == CS_TT_LBRC)
  {
    (*token_index)++;
    block = cs_parse_statements(tokens, token_index, CS_TT_RBRC);
  }
  else
  {
    block = cs_parse_statements(tokens, token_index, CS_TT_NEW);
  }
  //if(block == NULL) return NULL; // FIXME: Not needed, right?
  return block;
}
#pragma endregion Parse Block

#pragma region Parse Statements
cs_node * cs_parse_statements(cs_token * tokens, size_t * token_index, cs_token_type eob_token)
{
  size_t block_size = 4; // FIXME: // amt of cs_node*
  cs_node ** block = malloc(block_size * sizeof(*block));
  if(block == NULL) WERR(L"malloc");
  
  size_t token_index_start = *token_index;
  size_t block_index = 0;

  while(1)
  {
    if(tokens[*token_index].type == CS_TT_NEW)
    {
      (*token_index)++;
      continue;
    }
    if(block_index >= block_size)
    {
      block_size *= 2;
      cs_node ** _block = realloc(block, block_size * sizeof(_block));
      if(_block == NULL) WERR(L"realloc");
      block = _block;
      wprintf(L"-> Block doubled\n"); // DEBUG:
    }
    if(tokens[*token_index].type == eob_token)
    {
      wprintf(L"(%d)",eob_token);
      (*token_index)++;
      break;
    }
    if(tokens[*token_index].type == CS_TT_EOF)
    {
      wprintf(L"[%d]",eob_token);
      break;
    }
    if(tokens[*token_index].type == CS_TT_LBRC)
    {
      block[block_index] = cs_parse_block(tokens, token_index);
      if(block[block_index] == NULL) return NULL;
    }
    else
    {
      block[block_index] = cs_parse_statement(tokens, token_index);
      if(block[block_index] == NULL) return NULL;
      if(tokens[*token_index].type != eob_token
      && tokens[*token_index].type != CS_TT_NEW)
      {
        error.type = CS_ET_EXPECTED_TOKEN;
        error.pos = tokens[*token_index].pos;
        error.info.type = CS_VT_INT;
        error.info.value._int = (int)CS_TT_NEW;
        error.__line__ = __LINE__;
        strcpy(error.__file__, __FILE__);
        return NULL;
      }
    }
    block_index++;
  }

  cs_node * end = malloc(sizeof(*end));
  if(end == NULL) WERR(L"malloc");
  end->type = CS_NT_END;
  block[block_index] = end;

  cs_node * node = malloc(sizeof(*node));
  if(node == NULL) WERR(L"malloc");
  node->type = CS_NT_BLOCK;
  node->pos.start = tokens[token_index_start].pos.start;
  node->pos.end = tokens[*token_index].pos.end;
  node->content.datum.value._p = (void**)block;

  return node;
}
#pragma endregion Parse Statements

#pragma region Parse Statement
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index)
{
  switch(tokens[*token_index].type)
  {
    case CS_TT_FOR: return cs_parse_for(tokens, token_index);
    case CS_TT_IF: return cs_parse_if(tokens, token_index, CS_TT_IF);
    case CS_TT_WHILE: return cs_parse_while(tokens, token_index);
    case CS_TT_DEF: return cs_parse_def(tokens, token_index);
    
    default: {
      cs_node * expression = cs_parse_expression(tokens, token_index);
      if(expression == NULL) return NULL;
      return expression;
    }
  }
}
#pragma endregion Parse Statement

#pragma region Parse If Statement
cs_node * cs_parse_if(cs_token * tokens, size_t * token_index, cs_token_type start_token)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // if or elif
  if(tokens[*token_index].type != start_token)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = (int)start_token;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;

  // condition
  cs_node * condition = cs_parse_conditions(tokens, token_index);
  if(condition == NULL) return NULL;
  
  // { ... }
  cs_node * truebody = cs_parse_block(tokens, token_index);
  if(truebody == NULL) return NULL;

  cs_node * ifstatement = malloc(sizeof(*ifstatement));
  ifstatement->type = CS_NT_IF;
  ifstatement->pos.start = condition->pos.start; // Is overwritten later.
  ifstatement->pos.end = truebody->pos.end;
  ifstatement->content.branch._if.condition = condition;
  ifstatement->content.branch._if.truebody = truebody;
  ifstatement->content.branch._if.falsebody = NULL;

  // elif
  if(tokens[*token_index].type == CS_TT_ELIF)
  {
    cs_node * falsebody = cs_parse_if(tokens, token_index, CS_TT_ELIF);
    if(falsebody == NULL) return NULL;
    ifstatement->content.branch._if.falsebody = falsebody;
  }

  // else
  else if(tokens[*token_index].type == CS_TT_ELSE)
  {
    (*token_index)++;
    cs_node * falsebody = cs_parse_block(tokens, token_index);
    if(falsebody == NULL) return NULL;
    ifstatement->content.branch._if.falsebody = falsebody;
  }

  return ifstatement;
}
#pragma endregion Parse If Statement

#pragma region Parse For Loop
// Return Node: CS_NT_FOR
// Return Type: Branches
cs_node * cs_parse_for(cs_token * tokens, size_t * token_index)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // for
  if(tokens[*token_index].type != CS_TT_FOR)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_FOR;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;
  
  // i
  cs_node * counter = cs_parse_id(tokens, token_index);
  if(counter == NULL) return NULL;
  
  // from
  if(tokens[*token_index].type != CS_TT_FROM)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_FROM;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;
  
  // 1
  cs_node * from = cs_parse_expression(tokens, token_index);
  if(from == NULL) return NULL;
  
  // to
  if(tokens[*token_index].type != CS_TT_TO)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_TO;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;
  
  // 10
  cs_node * to = cs_parse_expression(tokens, token_index);
  if(to == NULL) return NULL;
  
  // { ... }
  cs_node * body = cs_parse_block(tokens, token_index);
  if(body == NULL) return NULL;
  
  // i, 1, 10, ...
  cs_node * node = malloc(sizeof(*node));
  node->type = CS_NT_FOR;
  node->pos = (cs_position){pos_start, tokens[(*token_index)-1].pos.end};
  node->content.branch._for.counter = counter;
  node->content.branch._for.from = from;
  node->content.branch._for.to = to;
  node->content.branch._for.body = body;
  return node;
}
#pragma endregion Parse For Loop

#pragma region Parse While Loop
// Return Node: CS_NT_WHILE
// Return Type: Branches
cs_node * cs_parse_while(cs_token * tokens, size_t * token_index)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // while
  if(tokens[*token_index].type != CS_TT_WHILE)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_WHILE;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;
  
  // condition
  cs_node * condition = cs_parse_condition(tokens, token_index);
  if(condition == NULL) return NULL;
  
  // { ... }
  cs_node * body = cs_parse_block(tokens, token_index);
  if(body == NULL) return NULL;
  
  cs_node * node = malloc(sizeof(*node));
  node->type = CS_NT_WHILE;
  node->pos = (cs_position){pos_start, body->pos.end};
  node->content.branch._while.condition = condition;
  node->content.branch._while.body = body;
  return node;
}
#pragma endregion Parse While Loop

#pragma region Parse Function Definition
// Return Node: CS_NT_FNDEF
// Return Type: Branches
cs_node * cs_parse_def(cs_token * tokens, size_t * token_index)
{
  size_t pos_start = tokens[*token_index].pos.start;

  // def
  if(tokens[*token_index].type != CS_TT_DEF)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_DEF;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;

  // name
  cs_node * name = cs_parse_id(tokens, token_index);
  if(name == NULL) return NULL;

  // (
  if(tokens[*token_index].type != CS_TT_LPAR)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_LPAR;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  (*token_index)++;

  // parameters)
  cs_node * params = cs_parse_id_sequence(tokens, token_index, CS_TT_RPAR);
  if(params == NULL) return NULL;

  // { ... }
  cs_node * body = cs_parse_block(tokens, token_index);
  if(body == NULL) return NULL;

  cs_node * fndef = malloc(sizeof(*fndef));
  if(fndef == NULL) WERR(L"malloc");
  fndef->type = CS_NT_FNDEF;
  fndef->pos = (cs_position){pos_start, body->pos.end};
  fndef->content.branch._fndef.name = name;
  fndef->content.branch._fndef.params = params;
  fndef->content.branch._fndef.body = body;

  return fndef;
}
#pragma endregion Parse Function Definition

#pragma region Parse Conditions
cs_node * cs_parse_conditions(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_condition(tokens, token_index);
  if(left == NULL) return NULL;
  
  while(tokens[*token_index].type == CS_TT_AND
     || tokens[*token_index].type == CS_TT_OR)
  {
    cs_node * new_left = malloc(sizeof(*new_left));
    new_left->type = tokens[*token_index].type == CS_TT_AND ? CS_NT_AND : CS_NT_OR;
    new_left->pos.start = left->pos.start;
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_condition(tokens, token_index);
    if(right == NULL) return NULL;
    new_left->content.branch._binop.right = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;

  }
  
  return left;
}
#pragma endregion Parse Conditions

#pragma region Parse Condition
cs_node * cs_parse_condition(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_expression(tokens, token_index);
  if(left == NULL) return NULL;
  
  while(tokens[*token_index].type == CS_TT_EQU
     || tokens[*token_index].type == CS_TT_NEQ
     || tokens[*token_index].type == CS_TT_GTR
     || tokens[*token_index].type == CS_TT_GEQ
     || tokens[*token_index].type == CS_TT_LSS
     || tokens[*token_index].type == CS_TT_LEQ)
  {
    cs_node * new_left = malloc(sizeof(*new_left));
    switch(tokens[*token_index].type)
    {
      case CS_TT_EQU: new_left->type = CS_NT_EQU; break;
      case CS_TT_NEQ: new_left->type = CS_NT_NEQ; break;
      case CS_TT_GTR: new_left->type = CS_NT_GTR; break;
      case CS_TT_GEQ: new_left->type = CS_NT_GEQ; break;
      case CS_TT_LSS: new_left->type = CS_NT_LSS; break;
      case CS_TT_LEQ: new_left->type = CS_NT_LEQ; break;
      default: WERR(L"what?");
    }
    new_left->pos.start = left->pos.start;
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_expression(tokens, token_index);
    if(right == NULL) return NULL;
    new_left->content.branch._binop.right = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;

  }
  
  return left;
}
#pragma endregion Parse Condition

#pragma region Parse Expression
cs_node * cs_parse_expression(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_term(tokens, token_index);
  if(left == NULL) return NULL;
  
  while(tokens[*token_index].type == CS_TT_ADD
     || tokens[*token_index].type == CS_TT_SUB)
  {
    cs_node * new_left = malloc(sizeof(*new_left));
    new_left->type = tokens[*token_index].type == CS_TT_ADD ? CS_NT_ADD : CS_NT_SUB;
    new_left->pos.start = left->pos.start;
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_term(tokens, token_index);
    if(right == NULL) return NULL;
    new_left->content.branch._binop.right = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Expression

#pragma region Parse Term
cs_node * cs_parse_term(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_power(tokens, token_index);
  if(left == NULL) return NULL;
  
  while(tokens[*token_index].type == CS_TT_MUL
     || tokens[*token_index].type == CS_TT_DIV
     || tokens[*token_index].type == CS_TT_MOD)
  {
    cs_node * new_left = malloc(sizeof(*new_left));
    switch(tokens[*token_index].type)
    {
      case CS_TT_MUL: new_left->type = CS_NT_MUL; break;
      case CS_TT_DIV: new_left->type = CS_NT_DIV; break;
      case CS_TT_MOD: new_left->type = CS_NT_MOD; break;
      default: WERR(L"what?!"); // FIXME: Ugly
    }
    new_left->pos.start = left->pos.start;
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_power(tokens, token_index);
    if(right == NULL) return NULL;
    new_left->content.branch._binop.right = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Term

#pragma region Parse Power
cs_node * cs_parse_power(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_index(tokens, token_index);
  if(left == NULL) return NULL;
  
  while(tokens[*token_index].type == CS_TT_POW)
  {
    cs_node * new_left = malloc(sizeof(*new_left));
    new_left->type = CS_NT_POW;
    new_left->pos.start = left->pos.start;
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_power(tokens, token_index);
    if(right == NULL) return NULL;
    new_left->content.branch._binop.right = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Power

#pragma region Parse Index
cs_node * cs_parse_index(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_atom(tokens, token_index);
  if(left == NULL) return NULL;
  
  while(tokens[*token_index].type == CS_TT_LSQB)
  {
    cs_node * new_left = malloc(sizeof(*new_left));
    new_left->type = CS_NT_INDEX;
    new_left->pos.start = left->pos.start;
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_expression(tokens, token_index);
    if(right == NULL) return NULL;
    new_left->content.branch._binop.right = right;
    
    if(tokens[*token_index].type != CS_TT_RSQB)
    {
      error.type = CS_ET_EXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)CS_TT_RSQB;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      return NULL;
    }
    (*token_index)++;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Index

#pragma region Parse Atom
cs_node * cs_parse_atom(cs_token * tokens, size_t * token_index)
{
  switch(tokens[*token_index].type)
  {
    case CS_TT_SUB: {
      cs_node * neg = malloc(sizeof(*neg));
      neg->type = CS_NT_NEG;
      neg->pos.start = tokens[*token_index].pos.start;
      (*token_index)++;
      cs_node * atom = cs_parse_atom(tokens, token_index);
      if(atom == NULL) return NULL;
      neg->content.branch._unop.center = atom;
      return atom; }

    case CS_TT_INT: {
      cs_node * num = malloc(sizeof(*num));
      num->type = CS_NT_INT;
      num->pos = tokens[*token_index].pos;
      num->content.datum.type = CS_VT_INT;
      num->content.datum.value._int = tokens[*token_index].value._int;
      (*token_index)++;
      return num; }
    
    case CS_TT_FLT: {
      cs_node * num = malloc(sizeof(*num));
      num->type = CS_NT_FLT;
      num->pos = tokens[*token_index].pos;
      num->content.datum.type = CS_VT_FLOAT;
      num->content.datum.value._float = tokens[*token_index].value._float;
      (*token_index)++;
      return num; }
    
    case CS_TT_STR: {
      cs_node * str = malloc(sizeof(*str));
      str->type = CS_NT_STR;
      str->pos = tokens[*token_index].pos;
      str->content.datum.type = CS_VT_WCS;
      str->content.datum.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;
      return str; }
    
    case CS_TT_ID: {
      cs_node * id = malloc(sizeof(*id));
      id->type = CS_NT_ID;
      id->pos = tokens[*token_index].pos;
      id->content.datum.type = CS_VT_WCS;
      id->content.datum.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;
      if(tokens[(*token_index)+1].type == CS_TT_LPAR)
      {
        cs_node * list = cs_parse_expression_sequence(tokens, token_index, CS_TT_RSQB);
        if(list == NULL) return NULL;
        cs_node * fncall = malloc(sizeof(*fncall));
        fncall->type = CS_NT_FNCALL;
        fncall->pos = (cs_position){id->pos.start, list->pos.end};
        fncall->content.branch._binop.left = id;
        fncall->content.branch._binop.right = list;
        return list;
      }
      else
      {
        return id;
      } }
    
    case CS_TT_LSQB: {
      (*token_index)++;
      cs_node * list = cs_parse_expression_sequence(tokens, token_index, CS_TT_RSQB);
      if(list == NULL) return NULL;
      return list; }
    
    case CS_TT_LPAR:
      (*token_index)++;
      cs_node * expression = cs_parse_conditions(tokens, token_index); // FIXME: Magic!
      if(expression == NULL) return NULL;
      if(tokens[*token_index].type != CS_TT_RPAR)
      {
        error.type = CS_ET_EXPECTED_TOKEN;
        error.pos = tokens[*token_index].pos;
        error.info.type = CS_VT_INT;
        error.info.value._int = (int)CS_TT_RPAR;
        error.__line__ = __LINE__;
        strcpy(error.__file__, __FILE__);
        return NULL;
      }
      (*token_index)++;
      return expression;
    
    default:
      error.type = CS_ET_UNEXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)(tokens[*token_index].type);
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      return NULL;
  }
}
#pragma endregion Parse Atom

#pragma region Parse Expression Sequence
cs_node * cs_parse_expression_sequence(cs_token * tokens, size_t * token_index, cs_token_type eos_token)
{
  size_t sequence_size = 4; // FIXME: // amt of cs_node*
  cs_node ** sequence = malloc(sequence_size * sizeof(*sequence));
  if(sequence == NULL) WERR(L"malloc");
  
  size_t token_index_start = *token_index;
  size_t sequence_index = 0;
  while(1)
  {
    if(tokens[*token_index].type == CS_TT_EOF)
    {
      error.type = CS_ET_UNEXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)CS_TT_EOF;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      return NULL;
    }
    // Skip SEP
    if(tokens[*token_index].type == CS_TT_SEP)
    {
      (*token_index)++;
      continue;
    }
    if(sequence_index >= sequence_size)
    {
      sequence_size *= 2;
      cs_node ** _sequence = realloc(sequence, sequence_size * sizeof(_sequence));
      if(_sequence == NULL) WERR(L"realloc");
      sequence = _sequence;
      wprintf(L"-> Sequence doubled\n"); // DEBUG:
    }
    if(tokens[*token_index].type == eos_token)
    {
      (*token_index)++;
      break;
    }
    sequence[sequence_index] = cs_parse_expression(tokens, token_index);
    if(sequence[sequence_index] == NULL) return NULL;
    sequence_index++;
  }
  cs_node * end = malloc(sizeof(*end));
  if(end == NULL) WERR(L"malloc");
  end->type = CS_NT_END;
  sequence[sequence_index] = end;

  cs_node * node = malloc(sizeof(*node));
  if(node == NULL) WERR(L"malloc");
  node->type = CS_NT_LIST;
  node->pos.start = tokens[token_index_start].pos.start; // FIXME:! '[' is not going to be included. (Also see cs_parse_statements)
  node->pos.end = tokens[*token_index].pos.end;
  node->content.datum.value._p = (void**)sequence;

  return node;
}
#pragma endregion Parse Expression Sequence

#pragma region Parse Identifier Sequence
cs_node * cs_parse_id_sequence(cs_token * tokens, size_t * token_index, cs_token_type eos_token)
{
  size_t sequence_size = 4; // FIXME: // amt of cs_node*
  cs_node ** sequence = malloc(sequence_size * sizeof(*sequence));
  if(sequence == NULL) WERR(L"malloc");
  
  size_t token_index_start = *token_index;
  size_t sequence_index = 0;
  while(1)
  {
    if(tokens[*token_index].type == CS_TT_EOF)
    {
      error.type = CS_ET_UNEXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)CS_TT_EOF;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      return NULL;
    }
    // Skip SEP
    if(tokens[*token_index].type == CS_TT_SEP)
    {
      (*token_index)++;
      continue;
    }
    if(sequence_index >= sequence_size)
    {
      sequence_size *= 2;
      cs_node ** _sequence = realloc(sequence, sequence_size * sizeof(_sequence));
      if(_sequence == NULL) WERR(L"realloc");
      sequence = _sequence;
      wprintf(L"-> Sequence doubled\n"); // DEBUG:
    }
    if(tokens[*token_index].type == eos_token)
    {
      (*token_index)++;
      break;
    }
    sequence[sequence_index] = cs_parse_id(tokens, token_index);
    if(sequence[sequence_index] == NULL) return NULL;
    sequence_index++;
  }
  cs_node * end = malloc(sizeof(*end));
  if(end == NULL) WERR(L"malloc");
  end->type = CS_NT_END;
  sequence[sequence_index] = end;

  cs_node * node = malloc(sizeof(*node));
  if(node == NULL) WERR(L"malloc");
  node->type = CS_NT_LIST;
  node->pos.start = tokens[token_index_start].pos.start; // FIXME:! '[' is not going to be included. (Also see cs_parse_statements)
  node->pos.end = tokens[*token_index].pos.end;
  node->content.datum.value._p = (void**)sequence;

  return node;
}
#pragma endregion Parse Identifier Sequence

#pragma region Parse Identifier
cs_node * cs_parse_id(cs_token * tokens, size_t * token_index)
{
  if(tokens[*token_index].type != CS_TT_ID)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_ID;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
    return NULL;
  }
  cs_node * id = malloc(sizeof(*id));
  id->type = CS_NT_ID;
  id->pos = tokens[*token_index].pos;
  id->content.datum.value._wcs = tokens[*token_index].value._wcs;
  (*token_index)++;
  return id;
}
#pragma endregion Parse Identifier

#pragma endregion Parser

#pragma region main
int main(int argc, char * argv[])
{
  cs_file file = {0};
  cs_file_read(&file, "test.txt");
  
  cs_token * tokens = cs_lex_wcs(file);
  if(tokens == NULL)
  {
    cs_error_display(file);
    return 1;
  }
  cs_tokens_display(tokens);
  wprintf(L"\n");
  
  size_t token_index = 0;
  cs_node * ast = cs_parse_statements(tokens, &token_index, CS_TT_EOF);
  if(ast == NULL)
  {
    cs_error_display(file);
    return 1;
  }
  
  wprintf(cs_node_to_wcs(ast));
  wprintf(L"\n\n");
  
  // cs_node ** inspect_nodes = (cs_node**)ast->content.datum.value._p;
  // wprintf(L"%d*%.2f [%ls]", inspect_nodes[0]->content.branch._binop.left->content.datum.value._int, inspect_nodes[0]->content.branch._binop.right->content.datum.value._float, cs_node_type_to_wcs(inspect_nodes[0]->content.branch._binop.right->type));
}
#pragma endregion main