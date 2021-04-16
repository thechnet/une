/*
test-2.c – CMDRScript C Interpreter Test 2
Modified 2021-04-07

This test is based on test-1.

macOS
clear && gcc test-2.c -o test-1 && ./test-1 test-2-program.txt
clear && clang test-2.c -o test-1.exe && ./test-1.exe test-2-program.txt

Windows
cls & gcc test-2.c -o test-2.exe && test-2.exe test-2-program.txt
cls & tcc test-2.c -o test-2.exe && test-2.exe test-2-program.txt
cls & clang test-2.c -o test-2.exe -Wno-deprecated && test-2.exe test-2-program.txt

Visual Studio Code Developer Prompt (Administrator)
cls & cl test-2.c /Fetest-2.exe /Fo"%temp%\cs.obj"

Planned for test-3:
- Rework node data and branches:
  - Every node has four 'cs_value's.
  - The content of the values is determined by the node type.

Planned for this test:
- Finish interpretation.

Backlog:
- Memory Pool
  - Nodes are placed next to each other in a memory pool (node_index).
  - Branches are just pointers to indices of other nodes.
  UNSURE: Is this extra effort worth it?
- Memory index for nodes to simplify freeing memory.
  UNSURE: Freeing memory as we go isn't that bad.

CHECKLIST:
- Do all variables allocate their OWN memory?
- Is there a way to not always have to duplicate strings for return values?
- Can atom include '(' highest_order_expression() ')'? (currently conditional_operation)
- Are we checking for NULL after every malloc or realloc?
- Do all nodes store correct positions?
- Allow Conditional Operation for Statements?
- TODO: Index ranges (0:5)
- TODO: Better SET and GET parser and interpreter implementations.

FIXME: Shared code between cs_node_to_wcs and cs_return_to_wcs.

Notes:
- The SET node returns the value the variable was set to. This potentially allows
  Python-like a = b = 1 statements.
*/

#pragma region Preprocessor

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <math.h>

// OS and compiler-specific differences
#if defined(__TINYC__)
  #include <io.h>
  #define CS_USE_OUTDATED_SWPRINTF
#elif defined(__clang__)
  #ifdef __APPLE__
    #include <unistd.h>
    #include <sys/uio.h>
  #else
    #include <io.h>
    #define access _access
    #define R_OK 4
  #endif
#elif defined(_MSC_VER)
  #include <io.h>
  #define R_OK 4
#else
  #include <unistd.h>
#endif
#ifdef _WIN32
  #include <windows.h>
  #define wchar_t WCHAR
#endif

#undef TRUE
#define TRUE 1
#undef FALSE
#define FALSE 0

// TCC implements an outdated swprintf.
#ifdef CS_USE_OUTDATED_SWPRINTF
  #define swprintf(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

#define CS_DEBUG_MALLOC_COUNTER
#ifdef CS_DEBUG_MALLOC_COUNTER
  int malloc_counter = 0;
  #define malloc(size) malloc((size)); malloc_counter++;
  #define free(memory) { free((memory)); malloc_counter--; }
#endif

// Temporary
#define SIZE_SMALL 128
#define SIZE_MEDIUM 4096
#define SIZE_BIG 32767

// Output Color Escape Sequences
#define CS_COLOR_SUCCESS L"\33[92m"
#define CS_COLOR_FAIL L"\33[31m"
#define CS_COLOR_NEUTRAL L"\33[0m"
#define CS_COLOR_HINT L"\33[90m"
#define CS_COLOR_TOKEN_TYPE L"\33[93m"
#define CS_COLOR_TOKEN_VALUE L"\33[92m"
#define CS_COLOR_NODE_BRANCH_TYPE L"\33[96m"
#define CS_COLOR_NODE_DATUM_TYPE L"\33[95m"
#define CS_COLOR_NODE_DATUM_VALUE L"\33[35m"

// Temporary Runtime Error Response
#define WERR(s) { wprintf(CS_COLOR_FAIL L"ln %d: %ls\33[0m\n", __LINE__, s); exit(1); }

#define CS_DO_READ
#define CS_DO_LEX
#define CS_DO_PARSE
#define CS_DO_INTERPRET

// #define CS_DEBUG_LOG_FREE

// #define CS_DEBUG_SOFT_ERROR
/*
Experimental - Tries to avoid aborting the entire parse by
replacing invalid nodes with a string node.
*/
#define CREATE_ERROR_NODE \
  cs_node * error_node = cs_node_create(CS_NT_STR);\
  error_node->content.value._wcs = CS_COLOR_FAIL L"ERROR HERE";
#define CREATE_ERROR_LIST \
  cs_node ** error_list = malloc(2 * sizeof(*error_list));\
  if(error_list == NULL) WERR(L"Out of memory");\
  cs_node * error_counter = cs_node_create(CS_NT_VOID);\
  error_counter->content.value._int = 1;\
  CREATE_ERROR_NODE;\
  error_list[0] = error_counter;\
  error_list[1] = error_node;\

#pragma endregion (Pre)

#pragma region Miscellaneous

#pragma region WCS to Int
int wcs_to_int(wchar_t * str)
{
  int out = 0;
  for(int i = 0; str[i] != L'\0'; i++)
  {
    if(str[i] < L'0' || str[i] > L'9') return 0;
    out = 10 * out + str[i] - L'0';
  }
  return out;
}
#pragma endregion wcs_to_int

#pragma region WCS to Float
float wcs_to_float(wchar_t * str)
{
  float out = 0;
  for(int var = 1, i = 0; str[i] != L'\0'; i++)
  {
    if((str[i] < L'0' || str[i] > L'9')        // Not a digit.
    && (str[i] != L'.' || var != 1)) return 0; // Second dot.
    if(var != 1) var *= 10;
    if(str[i] == L'.') {
      var *= 10;
      i++;
    }
    out = (var != 1 ? 1 : 10) * out + ((float)(str[i] - L'0')) / var;
  }
  return out;
}
#pragma endregion wcs_to_float

#pragma region Read File
wchar_t * file_read(char * path)
{
  size_t text_size = SIZE_MEDIUM;
  wchar_t * text = malloc(text_size * sizeof(*text));
  if(text == NULL) WERR(L"Out of memory");
  FILE * f = fopen(path, "r,ccs=UTF-8");
  if(f == NULL) WERR(L"File not found");
  size_t cursor = 0;
  wint_t c; /* Can represent any Unicode character + (!) WEOF.
               This is important when using fgetwc(), as otherwise,
               WEOF will overflow and be indistinguishable from an
               actual character.*/
  while(TRUE)
  {
    c = fgetwc(f);
    if(c == L'\r') continue;
    if(cursor >= text_size-1) // NUL
    {
      text_size *= 2;
      wchar_t * _text = realloc(text, text_size * sizeof(*_text));
      if(_text == NULL) WERR(L"Out of memory");
      text = _text;
    }
    if(c == WEOF) break;
    text[cursor] = c;
    cursor++;
  }
  fclose(f);
  text[cursor] = L'\0';
  return text;
}
#pragma endregion Read File

#pragma endregion Miscellaneous

#pragma region --- Enums, Unions, Structs ---

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
  void ** _vpp;
  void * _vp;
} cs_value;
#pragma endregion Value

#pragma region Return Value Type
typedef enum _cs_return_type {
  CS_RT_VOID, // End of list
  CS_RT_INT, // _int
  CS_RT_FLT, // _float
  CS_RT_STR, // _wcs
  CS_RT_LIST, // _vpp
  CS_RT_ID // _wcs // FIXME: Will hopefully soon become obsolete thanks to new node structure.
} cs_return_type;
#pragma endregion Return Value Type

#pragma region Return Value
typedef struct _cs_return {
  cs_return_type type;
  cs_value value;
} cs_return;
#pragma endregion Return Value

#pragma region Token Type
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
  CS_TT_NOT,
  CS_TT_AND,
  CS_TT_OR,
  CS_TT_EQU,
  CS_TT_NEQ,
  CS_TT_GTR,
  CS_TT_GEQ,
  CS_TT_LSS,
  CS_TT_LEQ,

  // Clause Keywords
  CS_TT_IF,
  CS_TT_ELIF,
  CS_TT_ELSE,
  CS_TT_FOR,
  CS_TT_FROM,
  CS_TT_TO,
  CS_TT_WHILE,
  CS_TT_DEF,
  CS_TT_CONTINUE,
  CS_TT_BREAK,
  CS_TT_RETURN

} cs_token_type;
#pragma endregion Token Type

#pragma region Token
typedef struct _cs_token {
  cs_token_type type;
  cs_position pos;
  cs_value value;
} cs_token;
#pragma endregion Token

#pragma region Node Type
typedef enum _cs_node_type {
  
  // Data Types
  CS_NT_INT,
  CS_NT_FLT,
  CS_NT_STR,
  CS_NT_ID,
  CS_NT_LIST,

  // Arithmetic Operations
  CS_NT_POW,
  CS_NT_MUL,
  CS_NT_DIV,
  CS_NT_FDIV,
  CS_NT_MOD,
  CS_NT_ADD,
  CS_NT_SUB,
  CS_NT_NEG,

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
  
  // Conditional Operation
  CS_NT_COP,

  // Set, Get
  CS_NT_IDX,
  CS_NT_SET,
  CS_NT_SET_IDX,
  CS_NT_GET,
  CS_NT_DEF,
  CS_NT_CALL,

  // Control Flow
  CS_NT_FOR,
  CS_NT_WHILE,
  CS_NT_IF,
  CS_NT_RETURN,
  CS_NT_BREAK,
  CS_NT_CONTINUE,
  CS_NT_STATEMENTS,
  CS_NT_VOID
  
} cs_node_type;
#pragma endregion Node Type

#pragma region Node
typedef struct _cs_node {
  cs_node_type type;
  cs_position pos;
  union _content {
    cs_value value;
    struct _branch {
        struct _cs_node * a;
        struct _cs_node * b;
        struct _cs_node * c;
        struct _cs_node * d;
    } branch;
  } content;
} cs_node;
#pragma endregion Node

#pragma region Error Type
typedef enum _cs_error_type {
  CS_ET_NO_ERROR, // Debugging
  CS_ET_EXPECTED_TOKEN,
  CS_ET_UNEXPECTED_TOKEN,
  CS_ET_ILLEGAL_CHARACTER,
  CS_ET_INCOMPLETE_FLOAT,
  CS_ET_ADD,
  CS_ET_SUB,
  CS_ET_MUL,
  CS_ET_DIV,
  CS_ET_FDIV,
  CS_ET_MOD,
  CS_ET_NEG,
  CS_ET_POW,
  CS_ET_ZERO_DIVISION,
  CS_ET_COMPARISON,
  CS_ET_NOT_INDEXABLE,
  CS_ET_NOT_INDEX_TYPE,
  CS_ET_INDEX_OUT_OF_RANGE,
  CS_ET_SET,
  CS_ET_GET
} cs_error_type;
#pragma endregion Error Type

#pragma region Error
typedef struct _cs_error {
  cs_error_type type;
  cs_position pos;
  cs_value values[2]; // If you change this number, don't forget to change it in cs_error_free!
  char __file__[SIZE_MEDIUM]; // Debugging
  size_t __line__;     // ''
} cs_error;
#pragma endregion Error

#pragma region Variable Symbol
typedef struct _cs_variable {
  wchar_t * name;
  cs_return content; // FIXME: Naming
} cs_variable;
#pragma endregion Variable Symbol

#pragma region Function Symbol
typedef struct _cs_function {
  wchar_t * name;
  cs_node * params;
  cs_node * body;
} cs_function;
#pragma endregion Function Symbol

#pragma region Context
typedef struct _cs_context {
  char * name;
  wchar_t * text;
  cs_token * tokens;
  size_t token_index;
  cs_node * ast;
  cs_error error;
  struct _cs_context * parent;
  cs_variable * variables;
  size_t variables_size;
  size_t variables_count;
  cs_function * functions;
  size_t functions_size;
  size_t functions_count;
} cs_context;
#pragma endregion Context

#pragma endregion --- Enums, Unions, Structs ---

#pragma region --- Interfaces ---

/* Token */

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

    default: WERR(L"Unknown token type");
  }
}
#pragma endregion Token Type to WCS

#pragma region Token to WCS
/*
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than SIZE_MEDIUM. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
wchar_t * cs_token_to_wcs(cs_token token)
{
  wchar_t * str = malloc(SIZE_MEDIUM * sizeof(*str));
  if(str == NULL) WERR(L"Out of memory");
  wcscpy(str, CS_COLOR_TOKEN_TYPE);
  wcscat(str, cs_token_type_to_wcs(token.type));
  wcscat(str, CS_COLOR_NEUTRAL);
  switch(token.type)
  {
    case CS_TT_INT:
      swprintf(str+wcslen(str), SIZE_MEDIUM,
               L":" CS_COLOR_TOKEN_VALUE L"%d" CS_COLOR_NEUTRAL,
               token.value._int);
      break;
    
    case CS_TT_FLT:
      swprintf(str+wcslen(str), SIZE_MEDIUM,
               L":" CS_COLOR_TOKEN_VALUE L"%.3f" CS_COLOR_NEUTRAL,
               token.value._float);
      break;
    
    case CS_TT_STR:
      swprintf(str+wcslen(str), SIZE_MEDIUM,
               L":" CS_COLOR_TOKEN_VALUE L"\"%ls" CS_COLOR_TOKEN_VALUE L"\"" CS_COLOR_NEUTRAL,
               token.value._wcs);
      break;
    
    case CS_TT_ID:
      swprintf(str+wcslen(str), SIZE_MEDIUM,
               L":" CS_COLOR_TOKEN_VALUE L"%ls" CS_COLOR_NEUTRAL,
               token.value._wcs);
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
  wchar_t * token_as_wcs;
  while(TRUE)
  {
    token_as_wcs = cs_token_to_wcs(tokens[i]);
    wprintf(L"%ls ", token_as_wcs);
    free(token_as_wcs);
    if(tokens[i].type == CS_TT_EOF) break;
    i++;
  }
  wprintf(L"\n");
}
#pragma endregion Display Tokens

#pragma region Free Tokens
void cs_tokens_free(cs_token * tokens)
{
  if(tokens == NULL) return;
  size_t i = 0;
  while(tokens[i-1].type != CS_TT_EOF) // i-1: To also free CS_TT_EOF
  {
    switch(tokens[i].type)
    {
      case CS_TT_ID:
      case CS_TT_STR:
        free(tokens[i].value._wcs);
        #ifdef CS_DEBUG_LOG_FREE
          wprintf(L"Token: %ls\n", cs_token_type_to_wcs(tokens[i].type));
        #endif
        break;
      
      case CS_TT_INT:
      case CS_TT_FLT:
      case CS_TT_LPAR:
      case CS_TT_RPAR:
      case CS_TT_LBRC:
      case CS_TT_RBRC:
      case CS_TT_LSQB:
      case CS_TT_RSQB:
      case CS_TT_SEP:
      case CS_TT_NEW:
      case CS_TT_EOF:
      case CS_TT_SET:
      case CS_TT_ADD:
      case CS_TT_SUB:
      case CS_TT_MUL:
      case CS_TT_DIV:
      case CS_TT_FDIV:
      case CS_TT_MOD:
      case CS_TT_POW:
      case CS_TT_NOT:
      case CS_TT_AND:
      case CS_TT_OR:
      case CS_TT_EQU:
      case CS_TT_NEQ:
      case CS_TT_GTR:
      case CS_TT_GEQ:
      case CS_TT_LSS:
      case CS_TT_LEQ:
      case CS_TT_IF:
      case CS_TT_ELIF:
      case CS_TT_ELSE:
      case CS_TT_FOR:
      case CS_TT_FROM:
      case CS_TT_TO:
      case CS_TT_WHILE:
      case CS_TT_DEF:
      case CS_TT_CONTINUE:
      case CS_TT_BREAK:
      case CS_TT_RETURN:
        #ifdef CS_DEBUG_LOG_FREE
          wprintf(L"Token: %ls\n", cs_token_type_to_wcs(tokens[i].type));
        #endif
        break;
      
      default:
        WERR(L"Unhandled token type in cs_tokens_free()!\n");
    }
    i++;
  }
  free(tokens);
}
#pragma endregion Free Tokens

/* Node */

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
    case CS_NT_ID: return L"ID";

    // Arithmetic Operations
    case CS_NT_POW: return L"POW";
    case CS_NT_MUL: return L"MUL";
    case CS_NT_DIV: return L"DIV";
    case CS_NT_FDIV: return L"FDIV";
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
    
    // Conditional Operation
    case CS_NT_COP: return L"COP";

    // Set, Get
    case CS_NT_IDX: return L"IDX";
    case CS_NT_SET: return L"SET";
    case CS_NT_SET_IDX: return L"SET_IDX";
    case CS_NT_GET: return L"GET";
    case CS_NT_DEF: return L"DEF";
    case CS_NT_CALL: return L"CALL";

    // Control Flow
    case CS_NT_FOR: return L"FOR";
    case CS_NT_WHILE: return L"WHILE";
    case CS_NT_IF: return L"IF";
    case CS_NT_RETURN: return L"RETURN";
    case CS_NT_BREAK: return L"BREAK";
    case CS_NT_CONTINUE: return L"CONTINUE";
    case CS_NT_STATEMENTS: return L"STATEMENTS";
    case CS_NT_VOID: return L"VOID";
    
    default: return L"?";
  }
}
#pragma endregion Node Type to WCS

#pragma region Node to WCS
/*
This function is not dynamic and will cause a buffer overflow
if the returned array is longer than SIZE_MEDIUM. This could
realistically happen, were this function used in a real-world
environment, but since it is only used for debugging, I'm
leaving this vulnerability in here.
*/
wchar_t * cs_node_to_wcs(cs_node * node)
{
  wchar_t * buffer = malloc(SIZE_BIG * sizeof(*buffer));
  if(buffer == NULL) WERR(L"Out of memory");
  if(node == NULL)
  {
    wcscpy(buffer, CS_COLOR_HINT L"NULL" CS_COLOR_NEUTRAL);
    return buffer;
  }
  buffer[0] = L'\0';
  switch(node->type)
  {
    case CS_NT_STATEMENTS: {
      cs_node ** list = (cs_node**)node->content.value._vpp;
      if(list == NULL) WERR(L"Undefined statements pointer");
      size_t list_size = list[0]->content.value._int;
      if(list_size == 0)
      {
        swprintf(buffer, SIZE_BIG,
                 CS_COLOR_HINT L"NO STATEMENTS" CS_COLOR_NEUTRAL);
        break;
      }
      wchar_t * node_as_wcs = cs_node_to_wcs(list[1]);
      int offset = swprintf(buffer, SIZE_BIG,
                            CS_COLOR_NEUTRAL L"{%ls",
                            node_as_wcs);
      free(node_as_wcs);
      for(int i=2; i<=list_size; i++)
      {
        node_as_wcs = cs_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, SIZE_BIG,
                           CS_COLOR_NEUTRAL L"\n%ls",
                           node_as_wcs);
        free(node_as_wcs);
      }
      swprintf(buffer+offset, SIZE_BIG, CS_COLOR_NEUTRAL L"}");
      break; }
    
    case CS_NT_INT:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"%ls" CS_COLOR_NEUTRAL L":"
               CS_COLOR_NODE_DATUM_VALUE L"%d" CS_COLOR_NEUTRAL,
               cs_node_type_to_wcs(node->type),
               node->content.value._int);
      break;
    
    case CS_NT_FLT:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"%ls" CS_COLOR_NEUTRAL L":"
               CS_COLOR_NODE_DATUM_VALUE L"%.3f" CS_COLOR_NEUTRAL,
               cs_node_type_to_wcs(node->type),
               node->content.value._float);
      break;
    
    case CS_NT_STR:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"%ls" CS_COLOR_NEUTRAL L":"
               CS_COLOR_NODE_DATUM_VALUE L"\"%ls"
               CS_COLOR_NODE_DATUM_VALUE L"\"" CS_COLOR_NEUTRAL,
               cs_node_type_to_wcs(node->type),
               node->content.value._wcs);
      break;
    
    case CS_NT_ID:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"%ls" CS_COLOR_NEUTRAL L":"
               CS_COLOR_NODE_DATUM_VALUE L"%ls" CS_COLOR_NEUTRAL,
               cs_node_type_to_wcs(node->type),
               node->content.value._wcs);
      break;
    
    case CS_NT_LIST: {
      cs_node ** list = (cs_node**)node->content.value._vpp;
      if(list == NULL) WERR(L"Undefined list pointer");
      size_t list_size = list[0]->content.value._int;
      if(list_size == 0)
      {
        swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"[]");
        break;
      }
      wchar_t * node_as_wcs = cs_node_to_wcs(list[1]);
      int offset = swprintf(buffer, SIZE_BIG,
                            CS_COLOR_NEUTRAL L"[%ls",
                            node_as_wcs);
      free(node_as_wcs);
      for(int i=2; i<=list_size; i++)
      {
        node_as_wcs = cs_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, SIZE_BIG,
                           CS_COLOR_NEUTRAL L", %ls",
                           node_as_wcs);
        free(node_as_wcs);
      }
      swprintf(buffer+offset, SIZE_BIG, CS_COLOR_NEUTRAL L"]");
      break; }

    // Binary Operation
    case CS_NT_ADD:
    case CS_NT_SUB:
    case CS_NT_MUL:
    case CS_NT_DIV:
    case CS_NT_FDIV:
    case CS_NT_MOD:
    case CS_NT_POW:
    case CS_NT_IDX:
    case CS_NT_EQU:
    case CS_NT_NEQ:
    case CS_NT_GTR:
    case CS_NT_GEQ:
    case CS_NT_LSS:
    case CS_NT_LEQ:
    case CS_NT_AND:
    case CS_NT_OR:
    case CS_NT_CALL:
    case CS_NT_SET: {
      wchar_t * left_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.a);
      wchar_t * right_branch_as_wcs = cs_node_to_wcs(
                                      node->content.branch.b);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"%ls"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L")",
               cs_node_type_to_wcs(node->type),
               left_branch_as_wcs,
               right_branch_as_wcs);
      free(left_branch_as_wcs);
      free(right_branch_as_wcs);
      break; }
    
    // Unary Operation
    case CS_NT_NEG:
    case CS_NT_NOT:
    case CS_NT_GET:
    case CS_NT_RETURN: {
      wchar_t * center_branch_as_wcs = cs_node_to_wcs(
                                       node->content.branch.a);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"%ls"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L")",
               cs_node_type_to_wcs(node->type),
               center_branch_as_wcs);
      free(center_branch_as_wcs);
      break; }
    
    // Without operation
    case CS_NT_BREAK:
    case CS_NT_CONTINUE: {
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_BRANCH_TYPE L"%ls" CS_COLOR_NEUTRAL,
               cs_node_type_to_wcs(node->type));
      break; }
    
    case CS_NT_FOR: {
      wchar_t * counter_branch_as_wcs = cs_node_to_wcs(
                                        node->content.branch.a);
      wchar_t * from_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.b);
      wchar_t * to_branch_as_wcs = cs_node_to_wcs(
                                   node->content.branch.c);
      wchar_t * body_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.d);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"FOR"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L")",
               counter_branch_as_wcs,
               from_branch_as_wcs,
               to_branch_as_wcs,
               body_branch_as_wcs);
      free(counter_branch_as_wcs);
      free(from_branch_as_wcs);
      free(to_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }
    
    case CS_NT_WHILE: {
      wchar_t * condition_branch_as_wcs = cs_node_to_wcs(
                                          node->content.branch.a);
      wchar_t * body_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.b);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"WHILE"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L")",
               condition_branch_as_wcs,
               body_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }

    case CS_NT_IF: {
      wchar_t * condition_branch_as_wcs = cs_node_to_wcs(
                                          node->content.branch.a);
      wchar_t * truebody_branch_as_wcs = cs_node_to_wcs(
                                         node->content.branch.b);
      wchar_t * falsebody_branch_as_wcs = cs_node_to_wcs(
                                          node->content.branch.c);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE "IF"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               condition_branch_as_wcs,
               truebody_branch_as_wcs,
               falsebody_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(truebody_branch_as_wcs);
      free(falsebody_branch_as_wcs);
      break; }
    
    case CS_NT_COP: {
      wchar_t * trueconditions_branch_as_wcs = cs_node_to_wcs(
                                               node->content.branch.a);
      wchar_t * condition_branch_as_wcs = cs_node_to_wcs(
                                          node->content.branch.b);
      wchar_t * falseconditions_branch_as_wcs = cs_node_to_wcs(
                                                node->content.branch.c);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"COP"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               trueconditions_branch_as_wcs,
               condition_branch_as_wcs,
               falseconditions_branch_as_wcs);
      free(trueconditions_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(falseconditions_branch_as_wcs);
      break; }
    
    case CS_NT_SET_IDX: {
      wchar_t * name_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.a);
      wchar_t * index_branch_as_wcs = cs_node_to_wcs(
                                      node->content.branch.b);
      wchar_t * value_branch_as_wcs = cs_node_to_wcs(
                                      node->content.branch.c);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"SET_IDX"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               name_branch_as_wcs,
               index_branch_as_wcs,
               value_branch_as_wcs);
      free(name_branch_as_wcs);
      free(index_branch_as_wcs);
      free(value_branch_as_wcs);
      break; }
    
    case CS_NT_DEF: {
      wchar_t * name_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.a);
      wchar_t * params_branch_as_wcs = cs_node_to_wcs(
                                       node->content.branch.b);
      wchar_t * body_branch_as_wcs = cs_node_to_wcs(
                                     node->content.branch.c);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"DEF"
               CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls"
               CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               name_branch_as_wcs,
               params_branch_as_wcs,
               body_branch_as_wcs);
      free(name_branch_as_wcs);
      free(params_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }

    default:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(UNKNOWN NODE TYPE OR MEMORY)");
  }
  return buffer;
}
#pragma endregion Node to WCS

#pragma region Free Node
void cs_node_free(cs_node * node)
{
  if(node == NULL)
  {
    #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"Node: NULL\n");
    #endif
    return;
  }

  switch(node->type)
  {
    // No Operation (Data)
    case CS_NT_INT:
    case CS_NT_FLT:
    case CS_NT_BREAK:
    case CS_NT_CONTINUE:
    case CS_NT_VOID:
    case CS_NT_STR:
    case CS_NT_ID:
      /* DOC: Memory Management
      We don't free the WCS pointers because they are pointing at data stored in the tokens,
      which may still be needed after the parse. This memory is freed alongside the tokens.
      */
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Node: %ls\n", cs_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Sequences (Data)
    case CS_NT_LIST:
    case CS_NT_STATEMENTS: {
      cs_node ** list = (cs_node**)node->content.value._vpp;
      if(list == NULL)
      {
        // wprintf(L"Warning: Node to be freed has undefined list pointer.\n");
        break;
      }
      size_t list_size = list[0]->content.value._int;
      for(size_t i=0; i<=list_size; i++)
      {
        cs_node_free(list[i]);
      }
      free(list);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Node: %ls\n", cs_node_type_to_wcs(node->type));
      #endif
      free(node);
      break; }
    
    // Unary Operation
    case CS_NT_NEG:
    case CS_NT_NOT:
    case CS_NT_GET:
    case CS_NT_RETURN:
      cs_node_free(node->content.branch.a);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Node: %ls\n", cs_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Binary Operation
    case CS_NT_POW:
    case CS_NT_MUL:
    case CS_NT_DIV:
    case CS_NT_FDIV:
    case CS_NT_MOD:
    case CS_NT_ADD:
    case CS_NT_SUB:
    case CS_NT_EQU:
    case CS_NT_NEQ:
    case CS_NT_GTR:
    case CS_NT_GEQ:
    case CS_NT_LSS:
    case CS_NT_LEQ:
    case CS_NT_AND:
    case CS_NT_OR:
    case CS_NT_SET:
    case CS_NT_CALL:
    case CS_NT_WHILE:
    case CS_NT_IDX:
      cs_node_free(node->content.branch.a);
      cs_node_free(node->content.branch.b);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Node: %ls\n", cs_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Ternary Operation
    case CS_NT_COP:
    case CS_NT_DEF:
    case CS_NT_IF:
    case CS_NT_SET_IDX:
      cs_node_free(node->content.branch.a);
      cs_node_free(node->content.branch.b);
      cs_node_free(node->content.branch.c);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Node: %ls\n", cs_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    // Quaternary Operation
    case CS_NT_FOR:
      cs_node_free(node->content.branch.a);
      cs_node_free(node->content.branch.b);
      cs_node_free(node->content.branch.c);
      cs_node_free(node->content.branch.d);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Node: %ls\n", cs_node_type_to_wcs(node->type));
      #endif
      free(node);
      break;
    
    default: WERR(L"Unhandled node type in cs_node_free()!\n");
  }
}
#pragma endregion Free Node

#pragma region Create Node
cs_node * cs_node_create(cs_node_type type)
{
  cs_node * node = malloc(sizeof(*node));
  if(node == NULL) WERR(L"Out of memory");
  node->type = type;
  node->pos = (cs_position){0, 0};
  node->content.branch.a = NULL;
  node->content.branch.b = NULL;
  node->content.branch.c = NULL;
  node->content.branch.d = NULL;
  return node;
}
#pragma endregion Create Node

/* Return Value */

#pragma region Return Value Type to WCS
// FIXME: This function is technically not needed.
const wchar_t * cs_return_type_to_wcs(cs_return_type return_type)
{
  switch(return_type)
  {
    case CS_RT_INT: return L"INT";
    case CS_RT_FLT: return L"FLT";
    case CS_RT_STR: return L"STR";
    case CS_RT_LIST: return L"LIST";
    case CS_RT_VOID: return L"VOID";
    default: return L"Unknown Return Type!";
  }
}
#pragma endregion Return Value Type to WCS

#pragma region Return Value to Boolean
int cs_return_to_boolean(cs_return return_value)
{
  switch(return_value.type)
  {
    case CS_RT_INT: return return_value.value._int == 0 ? 0 : 1;
    case CS_RT_FLT: return return_value.value._float == 0.0 ? 0 : 1;
    case CS_RT_STR: return wcslen(return_value.value._wcs) == 0 ? 0 : 1;
    case CS_RT_LIST: return ((cs_return*)return_value.value._vp)[0].value._int == 0 ? 0 : 1;
    default: WERR(L"cs_return_to_boolean: Unhandled return value type");
  }
}
#pragma endregion Return Value to Boolean

#pragma region Compare Return Values
int cs_return_compare(cs_return left, cs_return right)
{
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return left.value._int == right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return (float)left.value._int == right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return left.value._float == (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return left.value._float == right.value._float;
  }
  
  // STR and STR
  else if(left.type == CS_RT_STR && right.type == CS_RT_STR)
  {
    return !wcscmp(left.value._wcs, right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == CS_RT_LIST && right.type == CS_RT_LIST)
  {
    cs_return * left_list = (cs_return*)left.value._vp;
    cs_return * right_list = (cs_return*)right.value._vp;
    size_t left_size = left_list[0].value._int;
    size_t right_size = right_list[0].value._int;
    if(left_size != right_size)
    {
      return 0;
    }
    else
    {
      for(size_t i=1; i<=left_size; i++)
      {
        if(!cs_return_compare(left_list[i], right_list[i])) return 0;
      }
    }
    return 1;
  }
  
  // Illegal Comparison
  return -1;
}
#pragma endregion Compare Return Values

#pragma region Return Value to WCS
wchar_t * cs_return_to_wcs(cs_return return_value)
{
  wchar_t * wcs = malloc(SIZE_MEDIUM * sizeof(*wcs)); // FIXME: SIZE
  if(wcs == NULL) WERR("Out of memory");
  int offset = swprintf(wcs, SIZE_MEDIUM,
                        CS_COLOR_NEUTRAL L"%ls" CS_COLOR_HINT L": " CS_COLOR_SUCCESS,
                        cs_return_type_to_wcs(return_value.type));
  switch(return_value.type)
  {
    case CS_RT_INT:
      swprintf(wcs+offset, SIZE_MEDIUM, L"%d", return_value.value._int);
      break;
    
    case CS_RT_FLT:
      swprintf(wcs+offset, SIZE_MEDIUM, L"%.3f", return_value.value._float);
      break;
    
    case CS_RT_STR:
      swprintf(wcs+offset, SIZE_MEDIUM, CS_COLOR_HINT L"\"" CS_COLOR_SUCCESS L"%ls" CS_COLOR_HINT L"\"", return_value.value._wcs);
      break;
    
    case CS_RT_LIST: {
      cs_return * list = (cs_return*)return_value.value._vp;
      if(list == NULL) WERR(L"Undefined list pointer");
      size_t list_size = list[0].value._int;
      if(list_size == 0)
      {
        swprintf(wcs+offset, SIZE_MEDIUM, CS_COLOR_NEUTRAL L"[]");
        break;
      }
      wchar_t * return_as_wcs = cs_return_to_wcs(list[1]);
      offset += swprintf(wcs+offset, SIZE_MEDIUM,
                            CS_COLOR_NEUTRAL L"[%ls",
                            return_as_wcs);
      free(return_as_wcs);
      for(int i=2; i<=list_size; i++)
      {
        return_as_wcs = cs_return_to_wcs(list[i]);
        offset += swprintf(wcs+offset, SIZE_MEDIUM,
                           CS_COLOR_NEUTRAL L", " CS_COLOR_SUCCESS L"%ls",
                           return_as_wcs);
        free(return_as_wcs);
      }
      swprintf(wcs+offset, SIZE_MEDIUM, CS_COLOR_NEUTRAL L"]");
      break; }
    
    case CS_RT_VOID:
      WERR(L"cs_return_to_wcs: Illegal");
  }
  return wcs;
}
#pragma endregion Return Value to WCS

#pragma region Free Return Value
void cs_return_free(cs_return return_value)
{
  switch(return_value.type)
  {
    case CS_RT_STR:
    case CS_RT_ID:
      free(return_value.value._wcs);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Return: %d\n", return_value.type);
      #endif
      break;
    
    case CS_RT_LIST: {
      cs_return * list = (cs_return*)return_value.value._vp;
      size_t list_size = list[0].value._int;
      // Here we start at 0 because we _do_ want to free the CS_RT_VOID value.
      for(size_t i=0; i<=list_size; i++)
      {
        cs_return_free(list[i]);
      }
      free(list);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Return: %d\n", return_value.type);
      #endif
      break; }
    
    case CS_RT_INT:
    case CS_RT_FLT:
    case CS_RT_VOID:
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Return: %d\n", return_value.type);
      #endif
      break;
    
    default: WERR(L"Unhandled return value type in cs_return_free()\n");
  }
}
#pragma endregion Free Return Value

#pragma region Copy Return Value
cs_return cs_return_copy(cs_return src)
{
  cs_return dest;
  dest.type = src.type;
  
  switch(src.type)
  {
    case CS_RT_INT:
    case CS_RT_VOID: // FIXME:?
      dest.value._int = src.value._int;
      break;
    
    case CS_RT_FLT:
      dest.value._float = src.value._float;
      break;
    
    case CS_RT_STR: {
      wchar_t * string = malloc((wcslen(src.value._wcs+1)*sizeof(*string)));
      if(string == NULL) WERR(L"Out of memory");
      wcscpy(string, src.value._wcs);
      dest.value._wcs = string;
      break; }
    
    case CS_RT_LIST: {
      cs_return * list = (cs_return*)src.value._vp;
      size_t list_size = list[0].value._int;
      cs_return * list_copy = malloc((list_size+1)*sizeof(*list_copy));
      if(list_copy == NULL) WERR(L"Out of memory");
      // Here we start at 0 because we _do_ want to copy the CS_RT_VOID value.
      for(int i=0; i<=list_size; i++)
      {
        list_copy[i] = cs_return_copy(list[i]);
      }
      dest.value._vp = (void*)list_copy;
      break; }
  }
  
  return dest;
}
#pragma endregion Copy Return Value

/* Error */

#pragma region Error Value to WCS
wchar_t * cs_error_value_to_wcs(cs_error_type type, cs_value * values)
{
  wchar_t * wcs = malloc(SIZE_MEDIUM * sizeof(*wcs));
  if(wcs == NULL) WERR(L"Out of memory");
  switch(type)
  {
    case CS_ET_NO_ERROR:
      swprintf(wcs, SIZE_MEDIUM, L"No error defined.");
      break;
    
    case CS_ET_EXPECTED_TOKEN: {
      int offset = swprintf(wcs, SIZE_MEDIUM, L"Expected token: %ls",
                            cs_token_type_to_wcs((cs_token_type)values[0]._int));
      if(values[1]._wcs != NULL)
      {
        swprintf(wcs+offset, SIZE_MEDIUM, L" or %ls",
                 cs_token_type_to_wcs((cs_token_type)values[1]._int));
      }
      break; }
    
    case CS_ET_UNEXPECTED_TOKEN:
      swprintf(wcs, SIZE_MEDIUM, L"Unexpected token: %ls",
               cs_token_type_to_wcs((cs_token_type)values[0]._int));
      break;
    
    case CS_ET_ILLEGAL_CHARACTER:
      swprintf(wcs, SIZE_MEDIUM, L"Illegal character: '%lc'",
               (wchar_t)values[0]._int);
      break;
    
    case CS_ET_INCOMPLETE_FLOAT:
      swprintf(wcs, SIZE_MEDIUM, L"Expected digit after '.'");
      break;
    
    case CS_ET_ADD: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot add %ls to %ls",
               cs_return_type_to_wcs((cs_return_type)values[1]._int),
               cs_return_type_to_wcs((cs_return_type)values[0]._int));
      break; }
    
    case CS_ET_SUB: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot subtract %ls from %ls",
               cs_return_type_to_wcs((cs_return_type)values[1]._int),
               cs_return_type_to_wcs((cs_return_type)values[0]._int));
      break; }
    
    case CS_ET_MUL: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot multiply %ls by %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int),
               cs_return_type_to_wcs((cs_return_type)values[1]._int));
      break; }
    
    case CS_ET_DIV: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot divide %ls by %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int),
               cs_return_type_to_wcs((cs_return_type)values[1]._int));
      break; }
    
    case CS_ET_FDIV: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot floor divide %ls by %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int),
               cs_return_type_to_wcs((cs_return_type)values[1]._int));
      break; }
    
    case CS_ET_MOD: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot get modulus of %ls and %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int),
               cs_return_type_to_wcs((cs_return_type)values[1]._int));
      break; }
    
    case CS_ET_NEG: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot negate %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int));
      break; }
    
    case CS_ET_POW: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot get %ls to the power of %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int),
               cs_return_type_to_wcs((cs_return_type)values[1]._int));
      break; }
    
    case CS_ET_ZERO_DIVISION:
      swprintf(wcs, SIZE_MEDIUM, L"Cannot divide by zero");
      break;
    
    case CS_ET_COMPARISON: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot compare %ls and %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int),
               cs_return_type_to_wcs((cs_return_type)values[1]._int));
      break; }
    
    case CS_ET_NOT_INDEXABLE: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot get index of %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int));
      break; }
    
    case CS_ET_NOT_INDEX_TYPE: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot use %ls as index",
               cs_return_type_to_wcs((cs_return_type)values[0]._int));
      break; }
    
    case CS_ET_INDEX_OUT_OF_RANGE: {
      swprintf(wcs, SIZE_MEDIUM, L"Index %d is out of range", values[0]._int);
      break; }
    
    case CS_ET_SET: {
      swprintf(wcs, SIZE_MEDIUM, L"Cannot assign value to %ls",
               cs_return_type_to_wcs((cs_return_type)values[0]._int));
      break; }
    
    case CS_ET_GET: {
      swprintf(wcs, SIZE_MEDIUM, L"Variable '%ls' does not exist", values[0]._wcs);
      break; }
    
    default:
      swprintf(wcs, SIZE_MEDIUM, L"Unknown error!");
      break;
  }
  return wcs;
}
#pragma endregion Error Info to WCS

#pragma region Display Error
void cs_error_display(cs_context * context)
{
  int line = 1;
  int line_begin = 0;
  int line_end = 0;
  size_t pos_start = context->error.pos.start;
  size_t pos_end = context->error.pos.end;
  char location_found = FALSE;
  for(int i=0; i<wcslen(context->text)+1 /* Catch '\0' */ ; i++)
  {
    if(i == pos_start) location_found = TRUE;
    if(location_found && (context->text[i] == L'\n' || context->text[i] == L'\0'))
    {
      line_end = i;
      break;
    }
    if(context->text[i] == L'\n')
    {
      line++;
      line_begin = i+1;
    }
  }
  wprintf(CS_COLOR_NEUTRAL L"\33[1mFile %hs, Line %d (%hs @ %d):\33[0m"
          CS_COLOR_NEUTRAL L"\n%.*ls\n",
          context->name, line, context->error.__file__, context->error.__line__,
          line_end-line_begin, context->text+line_begin);
  wprintf(L"\33[%dC%ls\33[1m" CS_COLOR_FAIL,
          pos_start-line_begin, (pos_start-line_begin > 0) ? L"" : L"\33[D");
  if(pos_start >= pos_end) WERR(L"pos_start >= pos_end\n"); // DEBUG: For debugging only.
  for(int i=0; i<pos_end-pos_start; i++)
  {
    wprintf(L"~");
  }
  wchar_t * error_info_as_wcs = cs_error_value_to_wcs(context->error.type, context->error.values);
  wprintf(CS_COLOR_FAIL L"\33[1m\n%ls\n\n\33[0m" CS_COLOR_HINT
          L"pos_start: %d\npos_end: %d\nline_begin: %d\nline_end: %d\33[0m\n",
          error_info_as_wcs,
          pos_start, pos_end, line_begin, line_end); // DEBUG: For debugging only.
  free(error_info_as_wcs);
}
#pragma endregion Display Error

#pragma region Free Error
void cs_error_free(cs_error error)
{
  switch(error.type)
  {
    case CS_ET_NO_ERROR:
    case CS_ET_EXPECTED_TOKEN:
    case CS_ET_UNEXPECTED_TOKEN:
    case CS_ET_ILLEGAL_CHARACTER:
    case CS_ET_INCOMPLETE_FLOAT:
    case CS_ET_ADD:
    case CS_ET_SUB:
    case CS_ET_MUL:
    case CS_ET_DIV:
    case CS_ET_FDIV:
    case CS_ET_MOD:
    case CS_ET_NEG:
    case CS_ET_POW:
    case CS_ET_ZERO_DIVISION:
    case CS_ET_COMPARISON:
    case CS_ET_NOT_INDEXABLE:
    case CS_ET_NOT_INDEX_TYPE:
    case CS_ET_INDEX_OUT_OF_RANGE:
    case CS_ET_SET:
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Error: %d\n", error.type);
      #endif
      break;
    
    case CS_ET_GET:
      free(error.values[0]._wcs);
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"Error: %d\n", error.type);
      #endif
      break;
    
    default: WERR(L"Unhandled error type in cs_error_free()!");
  }
}
#pragma endregion Free Error

/* Variable Symbol */

#pragma region Free Variable Symbol
void cs_variable_free(cs_variable variable)
{
  free(variable.name);
  cs_return_free(variable.content);
  #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"Variable: %ls\n", variable.name);
  #endif
}
#pragma endregion Free Variable Symbol

/* Function Symbol */

#pragma region Free Function Symbol
void cs_function_free(cs_function function)
{
  free(function.name);
  cs_node_free(function.params);
  cs_node_free(function.body);
  #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"Function: %ls\n", function.name);
  #endif
}
#pragma endregion Free Function Symbol

/* Context */

#pragma region Free Context
// FIXME: Complete?
void cs_context_free(cs_context * context)
{
  // 1)
  free(context->name);
  free(context->text);

  // 2)
  #ifdef CS_DO_INTERPRET
    for(size_t i=0; i<context->variables_count; i++)
    {
      cs_variable_free(context->variables[i]);
    }
    free(context->variables);
  #endif
  
  // 3)
  #ifdef CS_DO_INTERPRET
    for(size_t i=0; i<context->functions_count; i++)
    {
      cs_function_free(context->functions[i]);
    }
    free(context->functions);
  #endif
  
  // 4)
  #ifdef CS_DO_PARSE
    cs_node_free(context->ast);
  #endif
  
  // 5)
  #ifdef CS_DO_LEX
    cs_tokens_free(context->tokens);
  #endif
  
  // 6)
  // It's possible for error to contain pointers to allocated memory
  // that's used to provide details for an error message.
  // An example of this is the CS_ET_GET (see cs_interpret_get).
  cs_error_free(context->error);
  
  free(context);
  #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"Context\n");
  #endif
}
#pragma endregion Free Context

#pragma region Create Context
cs_context * cs_context_create(void)
{
  cs_context * context = malloc(sizeof(*context));
  if(context == NULL) WERR("Out of memory");
  context->name = NULL;
  context->text = NULL;
  context->error.type = CS_ET_NO_ERROR;
  context->error.pos.start = 0;
  context->error.pos.end = 1;
  context->error.values[0]._wcs = NULL;
  context->error.values[1]._wcs = NULL;
  context->tokens = NULL;
  context->token_index = 0;
  context->ast = NULL;
  context->variables = NULL;
  context->variables_size = 0;
  context->variables_count = 0;
  context->functions = NULL;
  context->functions_size = 0;
  context->functions_count = 0;
  return context;
}
#pragma endregion Create Context

//

#pragma endregion --- Interfaces ---

#pragma region --- Lexer ---
cs_token * cs_lex_wcs(wchar_t * text, cs_error * error)
{
  size_t tokens_size = SIZE_MEDIUM; // FIXME:
  cs_token * tokens = malloc(tokens_size * sizeof(*tokens));
  if(tokens == NULL) WERR(L"Out of memory");
  size_t tokens_index = 0;
  size_t idx = 0;
  while(TRUE)
  {
    if(tokens_index+1 >= tokens_size)
    {
      tokens_size *= 2;
      cs_token * _tokens = realloc(tokens, tokens_size * sizeof(*_tokens));
      if(_tokens == NULL) WERR(L"Out of memory");
      tokens = _tokens;
      wprintf(L"Warning: Tokens doubled\n");
    }
    
    #pragma region Number
    if(text[idx] >= L'0' && text[idx] <= L'9')
    {
      size_t size = SIZE_SMALL;
      wchar_t * buffer = malloc(size * sizeof(*buffer));
      if(buffer == NULL) WERR(L"Out of memory");
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
        cs_token tk = (cs_token){CS_TT_INT, (cs_position){idx_left, idx}};
        tk.value._int = wcs_to_int(buffer);
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
      cs_token tk = {CS_TT_FLT, (cs_position){idx_left, idx}};
      tk.value._float = wcs_to_float(buffer);
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
      size_t string_size = SIZE_MEDIUM; // FIXME:
      wchar_t * string = malloc(string_size * sizeof(*string));
      if(string == NULL) WERR(L"Out of memory");
      size_t string_index = 0;
      char escape = FALSE;
      while(TRUE)
      {
        if(text[idx] == L'\0') WERR(L"expected string termination");
        if(string_index >= string_size)
        {
          string_size *= 2;
          wchar_t * _string = realloc(string, string_size * sizeof(*_string));
          if(_string == NULL) WERR(L"Out of memory");
          string = _string;
        }
        if(escape)
        {
          escape = FALSE;
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
            escape = TRUE;
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
      cs_token tk = {CS_TT_STR, (cs_position){idx_left, idx}};
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
      size_t id_size = SIZE_MEDIUM; // FIXME:
      wchar_t * id = malloc(id_size * sizeof(*id));
      if(id == NULL) WERR(L"Out of memory");
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
          wchar_t * _id = realloc(id, id_size * sizeof(*_id));
          if(_id == NULL) WERR(L"Out of memory");
          id = _id;
        }
      }
      id[id_index] = L'\0';
      cs_token tk = {0};
           if(!wcscmp(id, L"if"))       tk.type = CS_TT_IF;
      else if(!wcscmp(id, L"elif"))     tk.type = CS_TT_ELIF;
      else if(!wcscmp(id, L"else"))     tk.type = CS_TT_ELSE;
      else if(!wcscmp(id, L"for"))      tk.type = CS_TT_FOR;
      else if(!wcscmp(id, L"from"))     tk.type = CS_TT_FROM;
      else if(!wcscmp(id, L"to"))       tk.type = CS_TT_TO;
      else if(!wcscmp(id, L"while"))    tk.type = CS_TT_WHILE;
      else if(!wcscmp(id, L"def"))      tk.type = CS_TT_DEF;
      else if(!wcscmp(id, L"return"))   tk.type = CS_TT_RETURN;
      else if(!wcscmp(id, L"break"))    tk.type = CS_TT_BREAK;
      else if(!wcscmp(id, L"continue")) tk.type = CS_TT_CONTINUE;
      else if(!wcscmp(id, L"and"))      tk.type = CS_TT_AND;
      else if(!wcscmp(id, L"or"))       tk.type = CS_TT_OR;
      else if(!wcscmp(id, L"not"))      tk.type = CS_TT_NOT;
      else
      {
        tk.type = CS_TT_ID;
        tk.value._wcs = id;
      }
      if(tk.type != CS_TT_ID) free(id);
      tk.pos = (cs_position){idx_left, idx};
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion Identifier (+ Clause Keywords)
    
    #pragma region Grouping and Ordering (- NEW)
    if(text[idx] == L'(')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_LPAR, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L')')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_RPAR, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'{')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_LBRC, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'}')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_RBRC, (cs_position){idx, idx+1}, 0};
      tokens[tokens_index++] = (cs_token){CS_TT_NEW, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'[')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_LSQB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L']')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_RSQB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L',')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_SEP, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'\0')
    {
      if(tokens_index == 0 || tokens[tokens_index-1].type != CS_TT_NEW)
      {
        tokens[tokens_index++] = (cs_token){CS_TT_NEW, (cs_position){idx, idx+1}, 0};
      }
      tokens[tokens_index] = (cs_token){CS_TT_EOF, (cs_position){idx, idx}, 0};
      break;
    }
    #pragma endregion Grouping and Ordering (- NEW, EOF)
    
    #pragma region Operators (+ Logical Equal)
    if(text[idx] == L'=')
    {
      idx++;
      if(text[idx] == L'=')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_EQU, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_SET, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(text[idx] == L'+')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_ADD, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'-')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_SUB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'*')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_MUL, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'/')
    {
      idx++;
      if(text[idx] == L'/')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_FDIV, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_DIV, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(text[idx] == L'%')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_MOD, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(text[idx] == L'^')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_POW, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    #pragma endregion Operators (+ Logical Equal)

    #pragma region Comparisons (- Logical Equal)
    if(text[idx] == L'!')
    {
      if(text[idx+1] == L'=')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_NEQ, (cs_position){idx, idx+2}, 0};
        idx += 2;
        continue;
      }
      // FIXME:
      error->type = CS_ET_ILLEGAL_CHARACTER;
      error->pos = (cs_position){idx, idx+1};
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
        tokens[tokens_index++] = (cs_token){CS_TT_GEQ, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_GTR, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(text[idx] == L'<')
    {
      idx++;
      if(text[idx] == L'=')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_LEQ, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_LSS, (cs_position){idx, idx+1}, 0};
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
      || tokens[tokens_index-1].type != CS_TT_NEW)
      {
        tokens[tokens_index++] = (cs_token){CS_TT_NEW, (cs_position){idx_left, idx}, 0};
      }
      continue;
    }
    #pragma endregion Whitespace

    #pragma region Illegal Character
    error->type = CS_ET_ILLEGAL_CHARACTER;
    error->pos = (cs_position){idx, idx+1};
    error->values[0]._int = (int)text[idx];
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    return NULL;
    #pragma endregion Illegal Character
  }
  return tokens;
}
#pragma endregion --- Lexer ---

#pragma region --- Parser ---

#pragma region (Prototypes)
cs_node * cs_parse_block(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node ** cs_parse_sequence(
  cs_token * tokens, size_t * token_index, cs_error * error,
  cs_node * (*parser)(cs_token * tokens, size_t * token_index, cs_error * error),
  cs_token_type tt_end_of_item,
  cs_token_type tt_end_of_sequence
);
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_if(cs_token * tokens, size_t * token_index, cs_error * error, cs_token_type start_token);
cs_node * cs_parse_for(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_while(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_def(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_return(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_conditional_operation(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_conditions(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_condition(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_expression(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_term(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_power(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_index(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_atom(cs_token * tokens, size_t * token_index, cs_error * error);
cs_node * cs_parse_id(cs_token * tokens, size_t * token_index, cs_error * error);
#pragma endregion (Prototypes)

#pragma region Parse Block
cs_node * cs_parse_block(
  cs_token * tokens,
  size_t * token_index,
  cs_error * error
)
{
  // {
  if(tokens[*token_index].type != CS_TT_LBRC)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)CS_TT_LBRC;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  cs_node * block = cs_node_create(CS_NT_STATEMENTS);
  block->pos.start = tokens[*token_index].pos.start;
  
  // ...
  cs_node ** sequence = cs_parse_sequence(
    tokens, token_index, error,
    &cs_parse_statement, CS_TT_NEW, CS_TT_RBRC
  );
  if(sequence == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_LIST;
      sequence = error_list;
    #else
      cs_node_free(block);
      return NULL;
    #endif
  }
  block->content.value._vpp = (void**)sequence;
  
  if(sequence[0]->content.value._int == 0)
  {
    cs_node_free(sequence[0]);
    free(sequence);
    error->type = CS_ET_UNEXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)CS_TT_RBRC;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_LIST;
      sequence = error_list;
    #else
      cs_node_free(block);
      return NULL;
    #endif
  }

  // Technically redundant since cs_parse_sequence already checks for this...
  // }
  if(tokens[*token_index].type != CS_TT_RBRC)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)CS_TT_RBRC;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      cs_node_free(block);
      return NULL;
    #endif
  }

  block->pos.end = tokens[*token_index].pos.end;
  
  // ... it doesn't advance past it, though.
  (*token_index)++;

  return block;
}
#pragma endregion Parse Block

#pragma region Parse Sequence
/*
Unlike all other cs_parse_ functions, this function does NOT return a node.
Instead it returns a list of nodes (cs_node**) to be set to the datum of a
node by the parent function.
*/
cs_node ** cs_parse_sequence(
  cs_token * tokens, size_t * token_index, cs_error * error,
  cs_node * (*parser)(cs_token * tokens, size_t * token_index, cs_error * error),
  cs_token_type tt_end_of_item,
  cs_token_type tt_end_of_sequence
)
{
  size_t sequence_size = SIZE_MEDIUM; // FIXME:
  cs_node ** sequence = malloc(sequence_size * sizeof(*sequence));
  if(sequence == NULL) WERR(L"Out of memory");
  
  size_t pos_start = tokens[*token_index].pos.start;

  cs_node * counter = cs_node_create(CS_NT_VOID);
  sequence[0] = counter;
  
  size_t sequence_index = 1;

  while(TRUE)
  {
    if(tokens[*token_index].type == tt_end_of_item)
    {
      (*token_index)++;
    }

    if(sequence_index >= sequence_size)
    {
      sequence_size *= 2;
      cs_node ** _sequence = realloc(sequence, sequence_size * sizeof(_sequence));
      if(_sequence == NULL) WERR(L"Out of memory");
      sequence = _sequence;
      wprintf(L"Warning: Sequence doubled\n");
    }

    // EXPECTED END OF SEQUENCE
    if(tokens[*token_index].type == tt_end_of_sequence)
    {
      /*
      We don't skip EOF because it may still be needed by
      other functions up the call chain.
      */
      //if(tt_end_of_sequence != CS_TT_EOF) (*token_index)++;
      /*
      We don't skip the end of the sequence because:
      - If it is EOF, that may still be needed by other 
        functions up the call chain.
      - The last token's position my still be needed to
        determine the sequence's position.
      */
      break;
    }

    // UNEXPECTED END OF SEQUENCE
    if(tokens[*token_index].type == CS_TT_EOF)
    {
      /*
      This can happen if a block, list, list of parameters, or list of arguments is opened at the end of the file without being closed.
      */
      for(size_t i=0; i<sequence_index; i++) cs_node_free(sequence[i]);
      free(sequence);
      error->type = CS_ET_UNEXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)CS_TT_EOF;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_LIST;
        return error_list;
      #else
        return NULL;
      #endif
    }

    // Get item.
    sequence[sequence_index] = (*parser)(tokens, token_index, error);
    if(sequence[sequence_index] == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        sequence[sequence_index] = error_node;
      #else
        for(size_t i=0; i<sequence_index; i++) cs_node_free(sequence[i]);
        free(sequence);
        return NULL;
      #endif
    }
    sequence_index++;
    
    if(tokens[*token_index].type != tt_end_of_sequence
    && tokens[*token_index].type != tt_end_of_item)
    {
      for(size_t i=0; i<sequence_index; i++) cs_node_free(sequence[i]);
      free(sequence);
      error->type = CS_ET_EXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)tt_end_of_item;
      error->values[1]._int = (int)tt_end_of_sequence;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_LIST;
        return error_list;
      #else
        return NULL;
      #endif
    }
  }

  sequence[0]->content.value._int = sequence_index-1;

  return sequence;
}
#pragma endregion Parse Sequence

#pragma region Parse Statement
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index, cs_error * error)
{
  if(tokens[*token_index].type == CS_TT_NEW) (*token_index)++; // FIXME: Unsure...
  
  switch(tokens[*token_index].type)
  {
    case CS_TT_FOR: return cs_parse_for(tokens, token_index, error);
    case CS_TT_IF: return cs_parse_if(tokens, token_index, error, CS_TT_IF);
    case CS_TT_WHILE: return cs_parse_while(tokens, token_index, error);
    case CS_TT_DEF: return cs_parse_def(tokens, token_index, error);
    case CS_TT_LBRC: return cs_parse_block(tokens, token_index, error);
    case CS_TT_RETURN: return cs_parse_return(tokens, token_index, error);
    
    #pragma region Break
    case CS_TT_BREAK: {
      cs_node * breaknode = cs_node_create(CS_NT_BREAK);
      breaknode->pos = tokens[*token_index].pos;
      (*token_index)++;
      return breaknode; }
    #pragma endregion Break
    
    #pragma region Continue
    case CS_TT_CONTINUE: {
      cs_node * continuenode = cs_node_create(CS_NT_CONTINUE);
      continuenode->pos = tokens[*token_index].pos;
      (*token_index)++;
      return continuenode; }
    #pragma endregion Continue
    
    #pragma region Variable Definition
    case CS_TT_ID: {
      size_t token_index_before = *token_index; // Needed to return in case this is not a variable definition.
      
      cs_node * target = cs_parse_id(tokens, token_index, error);
      if(target == NULL)
      {
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          target = error_node;
        #else
          return NULL;
        #endif
      }
      
      cs_node * varset = cs_node_create(CS_NT_SET);
      varset->pos.start = target->pos.start;
      varset->content.branch.a = target;
      
      // Check if SET_IDX - If true, adjust the node type.
      if(tokens[*token_index].type == CS_TT_LSQB)
      {
        (*token_index)++;
        
        cs_node * position = cs_parse_conditional_operation(tokens, token_index, error);
        if(position == NULL)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            position = error_node;
          #else
            cs_node_free(varset);
            return NULL;
          #endif
        }
        
        if(tokens[*token_index].type != CS_TT_RSQB)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            return error_node;
          #else
            cs_node_free(varset);
            cs_node_free(position);
            return NULL;
          #endif
        }
        (*token_index)++;
        
        varset->type = CS_NT_SET_IDX;
        varset->content.branch.b = position;
      }
      
      if(tokens[*token_index].type == CS_TT_SET)
      {
        (*token_index)++;
        
        cs_node * conditional_operation = cs_parse_conditional_operation(tokens, token_index, error);
        if(conditional_operation == NULL)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            conditional_operation = error_node;
          #else
            cs_node_free(varset);
            return NULL;
          #endif
        }
        
        varset->pos.end = conditional_operation->pos.end;
        if(varset->type == CS_NT_SET_IDX)
        {
          varset->content.branch.c = conditional_operation;
        }
        else
        {
          varset->content.branch.b = conditional_operation;
        }
        return varset;
      }
      else
      {
        cs_node_free(varset);
        *token_index = token_index_before;
        /*
        Notice how there is no break here: If the above code ended here it means
        the statement is not a variable definition, leaving the only option
        for it to be a conditional operation.
        */
      } }
    #pragma endregion Variable Definition

    #pragma region Conditional Operation
    default: {
      cs_node * expression = cs_parse_conditional_operation(tokens, token_index, error);
      if(expression == NULL)
      {
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          return error_node;
        #else
          return NULL;
        #endif
      }
      return expression; }
    #pragma endregion Conditional Operation
  }
}
#pragma endregion Parse Statement

#pragma region Parse If Statement
cs_node * cs_parse_if(
  cs_token * tokens, size_t * token_index, cs_error * error,
  cs_token_type start_token)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // if or elif
  if(tokens[*token_index].type != start_token)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->values[0]._int = (int)start_token;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  // condition
  cs_node * condition = cs_parse_conditional_operation(tokens, token_index, error);
  if(condition == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      condition = error_node;
    #else
      return NULL;
    #endif
  }
  
  // statement
  cs_node * truebody = cs_parse_statement(tokens, token_index, error);
  if(truebody == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      truebody = error_node;
    #else
      cs_node_free(condition);
      return NULL;
    #endif
  }

  cs_node * ifstatement = cs_node_create(CS_NT_IF);
  ifstatement->pos.start = condition->pos.start; // Is overwritten later.
  ifstatement->pos.end = truebody->pos.end;
  ifstatement->content.branch.a = condition;
  ifstatement->content.branch.b = truebody;

  /*
  Whitespace. This counts for both 'elif' and 'else' because 'elif'
  creates an entirely new if node where this statement then removes
  whitespace in front of 'else'.
  */
  if(tokens[*token_index].type == CS_TT_NEW) (*token_index)++;

  if(tokens[*token_index].type == CS_TT_ELIF)
  {
    // elif ...
    cs_node * falsebody = cs_parse_if(tokens, token_index, error, CS_TT_ELIF);
    if(falsebody == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        falsebody = error_node;
      #else
        cs_node_free(ifstatement);
        return NULL;
      #endif
    }
    ifstatement->content.branch.c = falsebody;
  }
  else if(tokens[*token_index].type == CS_TT_ELSE)
  {
    // else statement
    (*token_index)++;
    cs_node * falsebody = cs_parse_statement(tokens, token_index, error);
    if(falsebody == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        falsebody = error_node;
      #else
        cs_node_free(ifstatement);
        return NULL;
      #endif
    }
    ifstatement->content.branch.c = falsebody;
  }

  return ifstatement;
}
#pragma endregion Parse If Statement

#pragma region Parse For Loop
// Return Node: CS_NT_FOR
// Return Type: Branches
cs_node * cs_parse_for(cs_token * tokens, size_t * token_index, cs_error * error)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // for
  if(tokens[*token_index].type != CS_TT_FOR)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->values[0]._int = CS_TT_FOR;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // i
  cs_node * counter = cs_parse_id(tokens, token_index, error);
  if(counter == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      counter = error_node;
    #else
      return NULL;
    #endif
  }
  
  // from
  if(tokens[*token_index].type != CS_TT_FROM)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->values[0]._int = CS_TT_FROM;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      cs_node_free(counter);
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // 1
  cs_node * from = cs_parse_conditional_operation(tokens, token_index, error);
  if(from == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      from = error_node;
    #else
      cs_node_free(counter);
      cs_node_free(from);
      return NULL;
    #endif
  }
  
  // to
  if(tokens[*token_index].type != CS_TT_TO)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->values[0]._int = CS_TT_TO;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // 10
  cs_node * to = cs_parse_conditional_operation(tokens, token_index, error);
  if(to == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      to = error_node;
    #else
      cs_node_free(counter);
      cs_node_free(from);
      return NULL;
    #endif
  }
  
  // ({) ... (})
  cs_node * body = cs_parse_statement(tokens, token_index, error);
  if(body == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      body = error_node;
    #else
      cs_node_free(counter);
      cs_node_free(from);
      cs_node_free(to);
      return NULL;
    #endif
  }
  
  // i, 1, 10, ...
  cs_node * node = cs_node_create(CS_NT_FOR);
  node->pos = (cs_position){pos_start, tokens[(*token_index)-1].pos.end};
  node->content.branch.a = counter;
  node->content.branch.b = from;
  node->content.branch.c = to;
  node->content.branch.d = body;
  return node;
}
#pragma endregion Parse For Loop

#pragma region Parse While Loop
// Return Node: CS_NT_WHILE
// Return Type: Branches
cs_node * cs_parse_while(cs_token * tokens, size_t * token_index, cs_error * error)
{
  size_t pos_start = tokens[*token_index].pos.start;
  
  // while
  if(tokens[*token_index].type != CS_TT_WHILE)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->values[0]._int = CS_TT_WHILE;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;
  
  // condition
  cs_node * condition = cs_parse_conditional_operation(tokens, token_index, error);
  if(condition == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      condition = error_node;
    #else
      return NULL;
    #endif
  }
  
  // { ... }
  cs_node * body = cs_parse_statement(tokens, token_index, error);
  if(body == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      body = error_node;
    #else
      cs_node_free(condition);
      return NULL;
    #endif
  }
  
  cs_node * node = cs_node_create(CS_NT_WHILE);
  node->pos = (cs_position){pos_start, body->pos.end};
  node->content.branch.a = condition;
  node->content.branch.b = body;
  return node;
}
#pragma endregion Parse While Loop

#pragma region Parse Function Definition
// Return Node: CS_NT_DEF
// Return Type: Branches
cs_node * cs_parse_def(cs_token * tokens, size_t * token_index, cs_error * error)
{
  size_t pos_start = tokens[*token_index].pos.start;

  // def
  if(tokens[*token_index].type != CS_TT_DEF)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)CS_TT_DEF;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  // name
  cs_node * name = cs_parse_id(tokens, token_index, error);
  if(name == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      name = error_node;
    #else
      return NULL;
    #endif
  }

  // (
  if(tokens[*token_index].type != CS_TT_LPAR)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)CS_TT_LPAR;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      cs_node_free(name);
      return NULL;
    #endif
  }

  cs_node * params = cs_node_create(CS_NT_LIST);
  params->pos.start = tokens[*token_index].pos.start;

  (*token_index)++;

  // parameters
  cs_node ** sequence = cs_parse_sequence(
    tokens, token_index, error,
    &cs_parse_id, CS_TT_SEP, CS_TT_RPAR
  );
  if(sequence == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_LIST;
      sequence = error_list;
    #else
      cs_node_free(name);
      cs_node_free(params);
      return NULL;
    #endif
  }

  params->pos.end = tokens[*token_index].pos.end;

  // ) // FIXME: Add redundant check here?
  (*token_index)++;

  params->content.value._vpp = (void**)sequence;

  // { ... }
  cs_node * body = cs_parse_statement(tokens, token_index, error);
  if(body == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      body = error_node;
    #else
      cs_node_free(name);
      cs_node_free(params);
      return NULL;
    #endif
  }

  cs_node * fndef = cs_node_create(CS_NT_DEF);
  fndef->pos = (cs_position){pos_start, body->pos.end};
  fndef->content.branch.a = name;
  fndef->content.branch.b = params;
  fndef->content.branch.c = body;

  return fndef;
}
#pragma endregion Parse Function Definition

#pragma region Parse Conditional Operation
cs_node * cs_parse_conditional_operation(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * trueconditions = cs_parse_conditions(tokens, token_index, error);
  if(trueconditions == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      trueconditions = error_node;
    #else
      return NULL;
    #endif
  }
  
  if(tokens[*token_index].type == CS_TT_IF)
  {
    (*token_index)++;
    
    cs_node * condition = cs_parse_conditions(tokens, token_index, error);
    if(condition == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        condition = error_node;
      #else
        cs_node_free(trueconditions);
        return NULL;
      #endif
    }
    
    if(tokens[*token_index].type != CS_TT_ELSE)
    {
      error->type = CS_ET_EXPECTED_TOKEN;
      error->values[0]._int = (int)CS_TT_ELSE;
      error->pos = tokens[*token_index].pos;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        cs_node_free(trueconditions);
        cs_node_free(condition);
        return NULL;
      #endif
    }
    (*token_index)++;
    
    cs_node * falseconditions = cs_parse_conditional_operation(tokens, token_index, error);
    if(falseconditions == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        falseconditions = error_node;
      #else
        cs_node_free(trueconditions);
        cs_node_free(condition);
        return NULL;
      #endif
    }
    
    cs_node * conditional_operation = cs_node_create(CS_NT_COP);
    conditional_operation->pos.start = trueconditions->pos.start;
    conditional_operation->pos.end = falseconditions->pos.end;
    conditional_operation->content.branch.a = trueconditions;
    conditional_operation->content.branch.b = condition;
    conditional_operation->content.branch.c = falseconditions;
    trueconditions = conditional_operation;
  }
  
  return trueconditions;
}
#pragma endregion Parse Conditional Operation

#pragma region Parse Conditions
cs_node * cs_parse_conditions(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * left = cs_parse_condition(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == CS_TT_AND
     || tokens[*token_index].type == CS_TT_OR)
  {
    cs_node * new_left = cs_node_create(
      tokens[*token_index].type == CS_TT_AND ? CS_NT_AND : CS_NT_OR);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    cs_node * right = cs_parse_condition(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Conditions

#pragma region Parse Condition
cs_node * cs_parse_condition(cs_token * tokens, size_t * token_index, cs_error * error)
{
  if(tokens[*token_index].type == CS_TT_NOT)
  {
    cs_node * not = cs_node_create(CS_NT_NOT);
    not->pos.start = tokens[*token_index].pos.start;
    (*token_index)++;
    
    cs_node * condition = cs_parse_condition(tokens, token_index, error);
    if(condition == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        condition = error_node;
      #else
        cs_node_free(not);
        return NULL;
      #endif
    }
    
    not->pos.end = condition->pos.end;
    not->content.branch.a = condition;
    
    return not;
  }
  
  cs_node * left = cs_parse_expression(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == CS_TT_EQU
     || tokens[*token_index].type == CS_TT_NEQ
     || tokens[*token_index].type == CS_TT_GTR
     || tokens[*token_index].type == CS_TT_GEQ
     || tokens[*token_index].type == CS_TT_LSS
     || tokens[*token_index].type == CS_TT_LEQ)
  {
    cs_node * new_left = cs_node_create(CS_NT_EQU);
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
    new_left->content.branch.a = left;
    (*token_index)++;

    cs_node * right = cs_parse_expression(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Condition

#pragma region Parse Expression
cs_node * cs_parse_expression(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * left = cs_parse_term(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == CS_TT_ADD
     || tokens[*token_index].type == CS_TT_SUB)
  {
    cs_node * new_left = cs_node_create(
      tokens[*token_index].type == CS_TT_ADD ? CS_NT_ADD : CS_NT_SUB);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    cs_node * right = cs_parse_term(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Expression

#pragma region Parse Term
cs_node * cs_parse_term(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * left = cs_parse_power(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == CS_TT_MUL
     || tokens[*token_index].type == CS_TT_DIV
     || tokens[*token_index].type == CS_TT_FDIV
     || tokens[*token_index].type == CS_TT_MOD)
  {
    cs_node * new_left = cs_node_create(CS_NT_MUL);
    switch(tokens[*token_index].type)
    {
      case CS_TT_MUL: new_left->type = CS_NT_MUL; break;
      case CS_TT_DIV: new_left->type = CS_NT_DIV; break;
      case CS_TT_FDIV: new_left->type = CS_NT_FDIV; break;
      case CS_TT_MOD: new_left->type = CS_NT_MOD; break;
      default: WERR(L"what?!"); // FIXME: Ugly
    }
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    cs_node * right = cs_parse_power(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Term

#pragma region Parse Power
cs_node * cs_parse_power(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * left = cs_parse_index(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == CS_TT_POW)
  {
    cs_node * new_left = cs_node_create(CS_NT_POW);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    // FIXME: RECURSIVE WHEN IT DOESNT HAVE TO BE, SEE EXPRESSION
    cs_node * right = cs_parse_power(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    new_left->pos.end = right->pos.end;
    
    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Power

#pragma region Parse Index
cs_node * cs_parse_index(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * left = cs_parse_atom(tokens, token_index, error);
  if(left == NULL)
  {
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      left = error_node;
    #else
      return NULL;
    #endif
  }
  
  while(tokens[*token_index].type == CS_TT_LSQB)
  {
    cs_node * new_left = cs_node_create(CS_NT_IDX);
    new_left->pos.start = left->pos.start;
    new_left->content.branch.a = left;
    (*token_index)++;

    cs_node * right = cs_parse_conditions(tokens, token_index, error);
    if(right == NULL)
    {
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        right = error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->content.branch.b = right;
    
    if(tokens[*token_index].type != CS_TT_RSQB)
    {
      error->type = CS_ET_EXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)CS_TT_RSQB;
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->pos.end = tokens[*token_index].pos.end;
    (*token_index)++;

    left = new_left;
  }
  
  return left;
}
#pragma endregion Parse Index

#pragma region Parse Atom
cs_node * cs_parse_atom(cs_token * tokens, size_t * token_index, cs_error * error)
{
  switch(tokens[*token_index].type)
  {
    case CS_TT_SUB: {
      cs_node * neg = cs_node_create(CS_NT_NEG);
      neg->pos.start = tokens[*token_index].pos.start;
      (*token_index)++;
      cs_node * expression = cs_parse_power(tokens, token_index, error);
      if(expression == NULL)
      {
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          expression = error_node;
        #else
          cs_node_free(neg);
          return NULL;
        #endif
      }
      neg->content.branch.a = expression;
      neg->pos.end = expression->pos.end;
      return neg; }

    case CS_TT_INT: {
      cs_node * num = cs_node_create(CS_NT_INT);
      num->pos = tokens[*token_index].pos;
      num->content.value._int = tokens[*token_index].value._int;
      (*token_index)++;
      return num; }
    
    case CS_TT_FLT: {
      cs_node * num = cs_node_create(CS_NT_FLT);
      num->pos = tokens[*token_index].pos;
      num->content.value._float = tokens[*token_index].value._float;
      (*token_index)++;
      return num; }
    
    case CS_TT_STR: {
      cs_node * str = cs_node_create(CS_NT_STR);
      str->pos = tokens[*token_index].pos;
      // DOC: Memory Management: This shows that nodes reference tokens' WCS instead of storing their own.
      str->content.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;
      return str; }
    
    case CS_TT_ID: {
      cs_node * node = cs_node_create(CS_NT_ID);
      node->pos = tokens[*token_index].pos;
      node->content.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;

      // Function Call
      if(tokens[*token_index].type == CS_TT_LPAR)
      {
        cs_node * fncall = cs_node_create(CS_NT_CALL);
        fncall->pos.start = node->pos.start;
        fncall->content.branch.a = node;

        cs_node * params = cs_node_create(CS_NT_LIST);
        params->pos.start = tokens[*token_index].pos.start;
        (*token_index)++;
        
        cs_node ** sequence = cs_parse_sequence(
          tokens, token_index, error,
          &cs_parse_conditions, CS_TT_SEP, CS_TT_RPAR
        );
        if(sequence == NULL)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_LIST;
            sequence = error_list;
          #else
            cs_node_free(fncall);
            cs_node_free(params);
            return NULL;
          #endif
        }
        params->content.value._vpp = (void**)sequence;
        params->pos.end = tokens[*token_index].pos.end;
        (*token_index)++;

        fncall->pos.end = params->pos.end;
        fncall->content.branch.b = params;
        node = fncall;
      }
      
      // Get Variable
      else
      {
        cs_node * varget = cs_node_create(CS_NT_GET);
        varget->pos = node->pos;
        varget->content.branch.a = node;
        node = varget;
      }
      
      return node; }
    
    case CS_TT_LSQB: {
      cs_node * list = cs_node_create(CS_NT_LIST);
      list->pos.start = tokens[*token_index].pos.start;
      // [
      (*token_index)++;
      // ...
      cs_node ** sequence = cs_parse_sequence(
        tokens, token_index, error,
        &cs_parse_conditions, CS_TT_SEP, CS_TT_RSQB
      );
      if(sequence == NULL)
      {
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_LIST;
          sequence = error_list;
        #else
          cs_node_free(list);
          return NULL;
        #endif
      }
      // ] // FIXME: Add redundant check here?
      list->pos.end = tokens[*token_index].pos.end;
      (*token_index)++;
      list->content.value._vpp = (void**)sequence;
      return list; }
    
    case CS_TT_LPAR: {
      (*token_index)++;
      cs_node * expression = cs_parse_conditional_operation(tokens, token_index, error);
      if(expression == NULL)
      {
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          expression = error_node;
        #else
          return NULL;
        #endif
      }
      if(tokens[*token_index].type != CS_TT_RPAR)
      {
        cs_node_free(expression);
        error->type = CS_ET_EXPECTED_TOKEN;
        error->pos = tokens[*token_index].pos;
        error->values[0]._int = (int)CS_TT_RPAR;
        error->__line__ = __LINE__;
        strcpy(error->__file__, __FILE__);
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          (*token_index)++;
          return error_node;
        #else
          return NULL;
        #endif
      }
      (*token_index)++;
      return expression; }
    
    default: {
      error->type = CS_ET_UNEXPECTED_TOKEN;
      error->pos = tokens[*token_index].pos;
      error->values[0]._int = (int)(tokens[*token_index].type);
      error->__line__ = __LINE__;
      strcpy(error->__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        return NULL;
      #endif
      }
  }
}
#pragma endregion Parse Atom

#pragma region Parse Identifier
cs_node * cs_parse_id(cs_token * tokens, size_t * token_index, cs_error * error)
{
  if(tokens[*token_index].type != CS_TT_ID)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->values[0]._int = CS_TT_ID;
    error->pos = tokens[*token_index].pos;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      (*token_index)++;
      return error_node;
    #else
      return NULL;
    #endif
  }
  cs_node * id = cs_node_create(CS_NT_ID);
  id->pos = tokens[*token_index].pos;
  id->content.value._wcs = tokens[*token_index].value._wcs;
  (*token_index)++;
  return id;
}
#pragma endregion Parse Identifier

#pragma region Parse Return Statement
cs_node * cs_parse_return(cs_token * tokens, size_t * token_index, cs_error * error)
{
  cs_node * returnnode = cs_node_create(CS_NT_RETURN);
  returnnode->pos = tokens[*token_index].pos; /* If the parser finds a return value after this,
                                                 pos.end is changed further down */

  // return
  if(tokens[*token_index].type != CS_TT_RETURN)
  {
    error->type = CS_ET_EXPECTED_TOKEN;
    error->pos = tokens[*token_index].pos;
    error->values[0]._int = (int)CS_TT_RETURN;
    error->__line__ = __LINE__;
    strcpy(error->__file__, __FILE__);
    #ifdef CS_DEBUG_SOFT_ERROR
      CREATE_ERROR_NODE;
      return error_node;
    #else
      return NULL;
    #endif
  }
  (*token_index)++;

  // value?
  cs_node * value;
  if(tokens[*token_index].type != CS_TT_NEW
  && tokens[*token_index].type != CS_TT_EOF) // FIXME: ?
  {
    value = cs_parse_conditional_operation(tokens, token_index, error);
    // DOC: NULL _here_ means an error.
    if(value == NULL) WERR(L"Out of memory");
    returnnode->pos.end = value->pos.end;
  }
  else
  {
    // DOC: NULL _here_ means no return value was specified. This tells the interpreter
    // not to try to interpret a return value where there is none.
    value = NULL;
  }
  
  returnnode->content.branch.a = value;
  
  return returnnode;
}
#pragma endregion Parse Return Statement

#pragma endregion --- Parser ---

#pragma region --- Interpreter ---

#pragma region (Prototypes)
cs_return cs_interpret(cs_node * node, cs_context * context);
cs_return cs_interpret_statements(cs_node * node, cs_context * context);
cs_return cs_interpret_add(cs_node * node, cs_context * context);
cs_return cs_interpret_sub(cs_node * node, cs_context * context);
cs_return cs_interpret_mul(cs_node * node, cs_context * context);
cs_return cs_interpret_div(cs_node * node, cs_context * context);
cs_return cs_interpret_fdiv(cs_node * node, cs_context * context);
cs_return cs_interpret_mod(cs_node * node, cs_context * context);
cs_return cs_interpret_neg(cs_node * node, cs_context * context);
cs_return cs_interpret_pow(cs_node * node, cs_context * context);
cs_return cs_interpret_not(cs_node * node, cs_context * context);
cs_return cs_interpret_equ(cs_node * node, cs_context * context);
cs_return cs_interpret_neq(cs_node * node, cs_context * context);
cs_return cs_interpret_gtr(cs_node * node, cs_context * context);
cs_return cs_interpret_geq(cs_node * node, cs_context * context);
cs_return cs_interpret_lss(cs_node * node, cs_context * context);
cs_return cs_interpret_leq(cs_node * node, cs_context * context);
cs_return cs_interpret_and(cs_node * node, cs_context * context);
cs_return cs_interpret_or(cs_node * node, cs_context * context);
cs_return cs_interpret_cop(cs_node * node, cs_context * context);
cs_return cs_interpret_idx(cs_node * node, cs_context * context);
cs_return cs_interpret_set(cs_node * node, cs_context * context);
cs_return cs_interpret_get(cs_node * node, cs_context * context);
#pragma endregion (Prototypes)

#pragma region Interpret
cs_return cs_interpret(cs_node * node, cs_context * context)
{
  switch(node->type)
  {
    case CS_NT_STATEMENTS: return cs_interpret_statements(node, context);
    
    case CS_NT_ADD: return cs_interpret_add(node, context);
    case CS_NT_SUB: return cs_interpret_sub(node, context);
    case CS_NT_MUL: return cs_interpret_mul(node, context);
    case CS_NT_DIV: return cs_interpret_div(node, context);
    case CS_NT_FDIV: return cs_interpret_fdiv(node, context);
    case CS_NT_MOD: return cs_interpret_mod(node, context);
    case CS_NT_NEG: return cs_interpret_neg(node, context);
    case CS_NT_POW: return cs_interpret_pow(node, context);
    case CS_NT_NOT: return cs_interpret_not(node, context);
    case CS_NT_EQU: return cs_interpret_equ(node, context);
    case CS_NT_NEQ: return cs_interpret_neq(node, context);
    case CS_NT_GTR: return cs_interpret_gtr(node, context);
    case CS_NT_GEQ: return cs_interpret_geq(node, context);
    case CS_NT_LSS: return cs_interpret_lss(node, context);
    case CS_NT_LEQ: return cs_interpret_leq(node, context);
    case CS_NT_AND: return cs_interpret_and(node, context);
    case CS_NT_OR: return cs_interpret_or(node, context);
    case CS_NT_COP: return cs_interpret_cop(node, context);
    case CS_NT_IDX: return cs_interpret_idx(node, context);
    
    case CS_NT_SET: return cs_interpret_set(node, context);
    case CS_NT_GET: return cs_interpret_get(node, context);
    
    case CS_NT_INT: {
      cs_return return_value;
      return_value.type = CS_RT_INT;
      return_value.value._int = node->content.value._int;
      return return_value; }
    
    case CS_NT_FLT: {
      cs_return return_value;
      return_value.type = CS_RT_FLT;
      return_value.value._float = node->content.value._float;
      return return_value; }
    
    case CS_NT_STR: {
      // DOC: Memory Management: Here we can see that return values COPY strings.
      wchar_t * string = malloc((wcslen(node->content.value._wcs)+1)*sizeof(*string));
      if(string == NULL) WERR(L"Out of memory");
      wcscpy(string, node->content.value._wcs);
      cs_return return_value;
      return_value.type = CS_RT_STR;
      return_value.value._wcs = string;
      return return_value; }
    
    // FIXME: Ugly, but will soon become obsolete with new node structure.
    case CS_NT_ID: {
      wchar_t * string = malloc((wcslen(node->content.value._wcs)+1)*sizeof(*string));
      if(string == NULL) WERR(L"Out of memory");
      wcscpy(string, node->content.value._wcs);
      cs_return return_value;
      return_value.type = CS_RT_ID;
      return_value.value._wcs = string;
      return return_value; }
    
    case CS_NT_LIST: {
      cs_node ** list = (cs_node**)node->content.value._vpp;
      size_t list_size = list[0]->content.value._int;
      cs_return * new_list = malloc((list_size+1)*sizeof(*new_list));
      if(new_list == NULL) WERR("Out of memory");
      new_list[0].type = CS_RT_VOID;
      new_list[0].value._int = list_size;
      for(size_t i=1; i<=list_size; i++)
      {
        new_list[i] = cs_interpret(list[i], context);
      }
      cs_return return_value;
      return_value.type = CS_RT_LIST;
      return_value.value._vp = new_list;
      return return_value; }
    
    default: WERR(L"cs_interpret: Node type not implemented");
  }
}
#pragma endregion Interpret

#pragma region Interpret Statements Node
cs_return cs_interpret_statements(cs_node * node, cs_context * context)
{
  // Default Return Value of a Block or Program
  cs_return return_value;
  return_value.type = CS_RT_INT;
  return_value.value._int = 46;
  // return_value.type = CS_RT_STR;
  // return_value.value._wcs = malloc(100*sizeof(wchar_t));
  // wcscpy(return_value.value._wcs, L"test");
  
  cs_node ** nodes = (cs_node**)node->content.value._vpp;
  size_t nodes_size = nodes[0]->content.value._int;
  for(size_t i=1; i<=nodes_size; i++)
  {
    if(nodes[i]->type == CS_NT_CONTINUE)
    {
      i = 1;
      continue;
    }
    if(nodes[i]->type == CS_NT_BREAK) break;
    if(nodes[i]->type == CS_NT_RETURN)
    {
      // DOC: If this case applies, it means the program defines its own return value,
      // meaning we should free the default return value.
      if(nodes[i]->content.branch.a != NULL)
      {
        cs_return_free(return_value);
        return_value = cs_interpret(nodes[i]->content.branch.a, context);
      }
      // Since there is a break statement following here no matter what, we don't
      // need to check whether cs_interpret returned an error or not,
      // since we are going to return the result right away either way.
      break;
    }
    cs_return _return_value = cs_interpret(nodes[i], context); // FIXME: Check this.
    if(_return_value.type == CS_RT_VOID)
    {
      // DOC: We only keep _return_value if there was an error. Otherwise, return values of
      // standalone expressions (or conditional operations, to be exact) are discarded right away.
      cs_return_free(return_value);
      return_value = _return_value;
      break;
    }
    // DOC: We need to free _return_value after every iteration because its contents
    // are not stored anywhere. (See comment above.)
    cs_return_free(_return_value);
  }
  
  return return_value;
}
#pragma endregion Interpret Statements Node

#pragma region Interpret Add Node
cs_return cs_interpret_add(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = left.value._int + right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = (float)left.value._int + right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float + (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float + right.value._float;
  }
  
  // STR and STR
  else if(left.type == CS_RT_STR && right.type == CS_RT_STR)
  {
    size_t left_len = wcslen(left.value._wcs);
    size_t right_len = wcslen(right.value._wcs);
    wchar_t * wcs = malloc((left_len+right_len+1)*sizeof(*wcs));
    if(wcs == NULL) WERR("Out of memory");
    wcscpy(wcs, left.value._wcs);
    wcscpy(wcs+left_len, right.value._wcs);
    return_value.type = CS_RT_STR;
    return_value.value._wcs = wcs;
  }
  
  // LIST and LIST
  else if(left.type == CS_RT_LIST && right.type == CS_RT_LIST)
  {
    // FIXME: Unsafe, doesn't duplicate strings etc., only pointers.
    cs_return * left_list = (cs_return*)left.value._vp;
    cs_return * right_list = (cs_return*)right.value._vp;
    size_t left_size = left_list[0].value._int;
    size_t right_size = right_list[0].value._int;
    cs_return * new_list = malloc((left_size+right_size+1)*sizeof(*new_list));
    if(new_list == NULL) WERR("Out of memory");
    new_list[0].type = CS_RT_VOID;
    new_list[0].value._int = left_size + right_size;
    memcpy((void*)(new_list+1),
           (void*)(left_list+1),
           left_size*sizeof(cs_return));
    memcpy((void*)(new_list+1+left_size),
           (void*)(right_list+1),
           right_size*sizeof(cs_return));
    return_value.type = CS_RT_LIST;
    return_value.value._vp = (void*)new_list;
  }

  // Error
  else
  {
    context->error.type = CS_ET_ADD;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Add Node

#pragma region Interpret Subtract Node
cs_return cs_interpret_sub(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = left.value._int - right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = (float)left.value._int - right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float - (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float - right.value._float;
  }

  // Error
  else
  {
    context->error.type = CS_ET_SUB;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Subtract Node

#pragma region Interpret Multiplication Node
cs_return cs_interpret_mul(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = left.value._int * right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = (float)left.value._int * right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float * (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float * right.value._float;
  }
  
  // INT and STR
  else if((left.type == CS_RT_INT && right.type == CS_RT_STR)
       || (left.type == CS_RT_STR && right.type == CS_RT_INT))
  {
    int count;
    wchar_t * wcs;
    if(left.type == CS_RT_INT)
    {
      count = left.value._int;
      wcs = right.value._wcs;
    }
    else
    {
      count = right.value._int;
      wcs = left.value._wcs;
    }
    size_t wcs_size = wcslen(wcs);
    wchar_t * string = malloc((wcs_size*count+1)*sizeof(*string));
    if(string == NULL) WERR("Out of memory");
    for(int i=0; i<count; i++)
    {
      wcscpy(string+i*wcs_size, wcs);
    }
    return_value.type = CS_RT_STR;
    return_value.value._wcs = string;
  }
  
  // INT and LIST
  else if((left.type == CS_RT_INT && right.type == CS_RT_LIST)
       || (left.type == CS_RT_LIST && right.type == CS_RT_INT))
  {
    int count;
    cs_return * list;
    if(left.type == CS_RT_INT)
    {
      count = left.value._int;
      list = (cs_return*)right.value._vp;
    }
    size_t list_size = list[0].value._int;
    cs_return * new_list = malloc(count*list_size*sizeof(*new_list));
    if(new_list == NULL) WERR("Out of memory");
    new_list[0].type = CS_RT_VOID;
    new_list[0].value._int = list_size;
    for(int i=0; i<count; i++)
    {
      memcpy((void*)(new_list+1+i*count),
             (void*)(list+1),
             list_size*sizeof(cs_return));
    }
    cs_return_free(left);
    cs_return_free(right);
    return_value.type = CS_RT_LIST;
    return_value.value._vp = (void*)new_list;
  }

  // Error
  else
  {
    context->error.type = CS_ET_MUL;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Multiplication Node

#pragma region Interpret Division Node
cs_return cs_interpret_div(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  if((right.type == CS_RT_INT && right.value._int == 0)
  || (right.type == CS_RT_FLT && right.value._float == 0.0))
  {
    context->error.type = CS_ET_ZERO_DIVISION;
    context->error.pos = node->content.branch.b->pos;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  // INT and FLT
  else if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    if(left.value._int % right.value._int == 0)
    {
      return_value.type = CS_RT_INT;
      return_value.value._int = left.value._int / right.value._int;
    }
    else
    {
      return_value.type = CS_RT_FLT;
      return_value.value._float = (float)left.value._int / right.value._int;
    }
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = (float)left.value._int / right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float / (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = left.value._float / right.value._float;
  }

  // Error
  else
  {
    context->error.type = CS_ET_DIV;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Division Node

#pragma region Interpret Floor Division Node
cs_return cs_interpret_fdiv(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  if((right.type == CS_RT_INT && right.value._int == 0)
  || (right.type == CS_RT_FLT && right.value._float == 0.0))
  {
    context->error.type = CS_ET_ZERO_DIVISION;
    context->error.pos = node->content.branch.b->pos;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = left.value._int / right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = floor((float)left.value._int / right.value._float);
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = floor(left.value._float / (float)right.value._int);
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = floor(left.value._float / right.value._float);
  }

  // Error
  else
  {
    context->error.type = CS_ET_FDIV;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Floor Division Node

#pragma region Interpret Modulus Node
cs_return cs_interpret_mod(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = left.value._int % right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = fmod((float)left.value._int, right.value._float);
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = fmod(left.value._float, (float)right.value._int);
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = fmod(left.value._float, right.value._float);
  }

  // Error
  else
  {
    context->error.type = CS_ET_MOD;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Modulus Node

#pragma region Interpret Negate Node
cs_return cs_interpret_neg(cs_node * node, cs_context * context)
{
  cs_return center = cs_interpret(node->content.branch.a, context);
  
  cs_return return_value;
  
  if(center.type == CS_RT_VOID) return center;
  
  if(center.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = -center.value._int;
  }
  else if(center.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = -center.value._float;
  }
  else
  {
    context->error.type = CS_ET_NEG;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)center.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(center);
  
  return return_value;
}
#pragma endregion Interpret Negate Node

#pragma region Interpret Power Node
cs_return cs_interpret_pow(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = pow(left.value._int, right.value._int);
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = powf((float)left.value._int, right.value._float);
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = powf(left.value._float, (float)right.value._int);
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.type = CS_RT_FLT;
    return_value.value._float = powf(left.value._float, right.value._float);
  }
  
  // Error
  else
  {
    context->error.type = CS_ET_POW;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Power Node

#pragma region Interpret Not Node
cs_return cs_interpret_not(cs_node * node, cs_context * context)
{
  cs_return center = cs_interpret(node->content.branch.a, context);
  if(center.type == CS_RT_VOID) return center;
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  return_value.value._int = !cs_return_to_boolean(center);
  
  cs_return_free(center);
  
  return return_value;
}
#pragma endregion Interpret Not Node

#pragma region Interpret Equals Node
cs_return cs_interpret_equ(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  int result = cs_return_compare(left, right);
  
  if(result == -1)
  {
    context->error.type = CS_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  else
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = result;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Equals Node

#pragma region Interpret Not Equals Node
cs_return cs_interpret_neq(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  int result = cs_return_compare(left, right);
  
  if(result == -1)
  {
    context->error.type = CS_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  else
  {
    return_value.type = CS_RT_INT;
    return_value.value._int = !result;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Not Equals Node

#pragma region Interpret Greater Than Node
cs_return cs_interpret_gtr(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._int > right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.value._int = (float)left.value._int > right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._float > (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.value._int = left.value._float > right.value._float;
  }
  
  // STR and STR
  else if(left.type == CS_RT_STR && right.type == CS_RT_STR)
  {
    return_value.value._int = wcslen(left.value._wcs) > wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == CS_RT_LIST && right.type == CS_RT_LIST)
  {
    return_value.value._int =
      ((cs_return*)left.value._vp)[0].value._int > ((cs_return*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = CS_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Greater Than Node

#pragma region Interpret Greater Than or Equals Node
cs_return cs_interpret_geq(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._int >= right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.value._int = (float)left.value._int >= right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._float >= (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.value._int = left.value._float >= right.value._float;
  }
  
  // STR and STR
  else if(left.type == CS_RT_STR && right.type == CS_RT_STR)
  {
    return_value.value._int = wcslen(left.value._wcs) >= wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == CS_RT_LIST && right.type == CS_RT_LIST)
  {
    return_value.value._int =
      ((cs_return*)left.value._vp)[0].value._int >= ((cs_return*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = CS_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Greater Than or Equals Node

#pragma region Interpret Less Than Node
cs_return cs_interpret_lss(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._int < right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.value._int = (float)left.value._int < right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._float < (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.value._int = left.value._float < right.value._float;
  }
  
  // STR and STR
  else if(left.type == CS_RT_STR && right.type == CS_RT_STR)
  {
    return_value.value._int = wcslen(left.value._wcs) < wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == CS_RT_LIST && right.type == CS_RT_LIST)
  {
    return_value.value._int =
      ((cs_return*)left.value._vp)[0].value._int < ((cs_return*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = CS_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Less Than Node

#pragma region Interpret Less Than or Equals Node
cs_return cs_interpret_leq(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  
  // INT and FLT
  if(left.type == CS_RT_INT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._int <= right.value._int;
  }
  else if(left.type == CS_RT_INT && right.type == CS_RT_FLT)
  {
    return_value.value._int = (float)left.value._int <= right.value._float;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_INT)
  {
    return_value.value._int = left.value._float <= (float)right.value._int;
  }
  else if(left.type == CS_RT_FLT && right.type == CS_RT_FLT)
  {
    return_value.value._int = left.value._float <= right.value._float;
  }
  
  // STR and STR
  else if(left.type == CS_RT_STR && right.type == CS_RT_STR)
  {
    return_value.value._int = wcslen(left.value._wcs) <= wcslen(right.value._wcs);
  }
  
  // LIST and LIST
  else if(left.type == CS_RT_LIST && right.type == CS_RT_LIST)
  {
    return_value.value._int =
      ((cs_return*)left.value._vp)[0].value._int <= ((cs_return*)right.value._vp)[0].value._int;
  }
  
  // Illegal Comparison
  else
  {
    context->error.type = CS_ET_COMPARISON;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)left.type;
    context->error.values[1]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Less Than or Equals Node

#pragma region Interpret And Node
cs_return cs_interpret_and(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  return_value.value._int = cs_return_to_boolean(left) && cs_return_to_boolean(right);
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret And Node

#pragma region Interpret Or Node
cs_return cs_interpret_or(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  return_value.type = CS_RT_INT;
  return_value.value._int = cs_return_to_boolean(left) || cs_return_to_boolean(right);
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Or Node

#pragma region Interpret Conditional Operation Node
cs_return cs_interpret_cop(cs_node * node, cs_context * context)
{
  cs_return truevalue = cs_interpret(node->content.branch.a, context);
  if(truevalue.type == CS_RT_VOID) return truevalue;
  cs_return condition = cs_interpret(node->content.branch.b, context);
  if(condition.type == CS_RT_VOID)
  {
    cs_return_free(truevalue);
    return condition;
  }
  cs_return falsevalue = cs_interpret(node->content.branch.c, context);
  if(falsevalue.type == CS_RT_VOID)
  {
    cs_return_free(truevalue);
    cs_return_free(condition);
    return falsevalue;
  }
  
  cs_return return_value;
  
  if(cs_return_to_boolean(condition))
  {
    return_value = truevalue;
    cs_return_free(falsevalue);
  }
  else
  {
    return_value = falsevalue;
    cs_return_free(truevalue);
  }
  
  cs_return_free(condition);
  
  return return_value;
}
#pragma endregion Interpret Conditional Operation Node

#pragma region Interpret Index Node
cs_return cs_interpret_idx(cs_node * node, cs_context * context)
{
  cs_return left = cs_interpret(node->content.branch.a, context);
  if(left.type == CS_RT_VOID) return left;
  cs_return right = cs_interpret(node->content.branch.b, context);
  if(right.type == CS_RT_VOID)
  {
    cs_return_free(left);
    return right;
  }
  
  cs_return return_value;
  
  if(right.type != CS_RT_INT)
  {
    context->error.type = CS_ET_NOT_INDEX_TYPE;
    context->error.pos = node->content.branch.b->pos;
    context->error.values[0]._int = (int)right.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
  }
  else
  {
    switch(left.type)
    {
      case CS_RT_STR:
        if(right.value._int >= 0 && right.value._int <= wcslen(left.value._wcs)-1)
        {
          wchar_t * string = malloc(2*sizeof(*string));
          if(string == NULL) WERR("Out of memory");
          string[0] = left.value._wcs[right.value._int];
          string[1] = L'\0';
          return_value.type = CS_RT_STR;
          return_value.value._wcs = string;
        }
        else
        {
          context->error.type = CS_ET_INDEX_OUT_OF_RANGE;
          context->error.pos = node->content.branch.b->pos;
          context->error.values[0]._int = right.value._int;
          context->error.__line__ = __LINE__;
          strcpy(context->error.__file__, __FILE__);
          return_value.type = CS_RT_VOID;
        }
        break;
      
      case CS_RT_LIST: {
        cs_return * list = (cs_return*)left.value._vp;
        size_t list_size = list[0].value._int;
        if(right.value._int >= 0 && right.value._int <= list_size-1)
        {
          return_value = cs_return_copy(list[right.value._int+1]);
        }
        else
        {
          context->error.type = CS_ET_INDEX_OUT_OF_RANGE;
          context->error.pos = node->content.branch.b->pos;
          context->error.values[0]._int = right.value._int;
          context->error.__line__ = __LINE__;
          strcpy(context->error.__file__, __FILE__);
          return_value.type = CS_RT_VOID;
        }
        break; }
      
      default:
        context->error.type = CS_ET_NOT_INDEXABLE;
        context->error.pos = node->pos;
        context->error.values[0]._int = (int)left.type;
        context->error.__line__ = __LINE__;
        strcpy(context->error.__file__, __FILE__);
        return_value.type = CS_RT_VOID;
    }
  }
  
  cs_return_free(left);
  cs_return_free(right);
  
  return return_value;
}
#pragma endregion Interpret Index Node

#pragma region Interpret Set Node
cs_return cs_interpret_set(cs_node * node, cs_context * context)
{
  cs_return name = cs_interpret(node->content.branch.a, context);
  if(name.type == CS_RT_VOID) return name;
  cs_return value = cs_interpret(node->content.branch.b, context);
  if(value.type == CS_RT_VOID)
  {
    cs_return_free(name);
    return value;
  }
  
  cs_return return_value;
  
  if(name.type != CS_RT_ID)
  {
    context->error.type = CS_ET_SET;
    context->error.pos = node->pos;
    context->error.values[0]._int = (int)name.type;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
    
    // Here we are freeing 'value' because we no longer want it.
    cs_return_free(value);
  }
  else
  {
    return_value = value;
    
    // Check if variable already exists.
    int exists_at = -1;
    for(size_t i=0; i<context->variables_count; i++)
    {
      if(wcscmp(context->variables[i].name, name.value._wcs) == 0)
      {
        exists_at = i;
        break;
      }
    }
    
    // Variable does not exist
    if(exists_at == -1)
    {
      cs_variable var;
      var.name = malloc((wcslen(name.value._wcs)+1)*sizeof(*var.name));
      if(var.name == NULL) WERR("Out of memory");
      wcscpy(var.name, name.value._wcs);
      var.content = cs_return_copy(value);
      if(context->variables_count >= context->variables_size)
      {
        context->variables_size *= 2;
        cs_variable * _variables = realloc(context->variables, context->variables_size*sizeof(*_variables));
        if(_variables == NULL) WERR("Out of memory");
        context->variables = _variables;
      }
      context->variables[context->variables_count] = var;
      context->variables_count++;
    }
    
    // Variable does exist
    else
    {
      cs_return_free(context->variables[exists_at].content);
      context->variables[exists_at].content = cs_return_copy(value);
    }

    // Here we are NOT freeing 'value' because we can use it as return value.
  }
  
  cs_return_free(name);
  
  return return_value;
}
#pragma endregion Interpret Set Node

#pragma region Interpret Get Node
cs_return cs_interpret_get(cs_node * node, cs_context * context)
{
  cs_return name = cs_interpret(node->content.branch.a, context);
  if(name.type == CS_RT_VOID) return name;
  
  // We don't need to check if name.type is CS_RT_ID because the parser
  // _only_ creates a CS_NT_GET if it finds an CS_TT_ID.
  
  cs_return return_value;
  
  // Check if variable exists.
  int exists_at = -1;
  for(size_t i=0; i<context->variables_count; i++)
  {
    if(wcscmp(context->variables[i].name, name.value._wcs) == 0)
    {
      exists_at = i;
      break;
    }
  }
  
  // Variable exists.
  if(exists_at > -1)
  {
    return_value = cs_return_copy(context->variables[exists_at].content);
    
    // Here we free 'name' because we no longer want it.
    cs_return_free(name);
  }
  
  // Variable does not exist.
  else
  {
    context->error.type = CS_ET_GET;
    context->error.pos = node->pos;
    context->error.values[0]._wcs = name.value._wcs;
    context->error.__line__ = __LINE__;
    strcpy(context->error.__file__, __FILE__);
    return_value.type = CS_RT_VOID;
    
    // Here we DON'T free 'name' because we can use its WCS in our return value.
  }
  
  return return_value;
}
#pragma endregion Interpret Get Node

#pragma endregion --- Interpreter ---

#pragma region Main
int main(int argc, char * argv[])
{
  // Prints a bar to distinguish new output from old output in the terminal.
  wprintf(CS_COLOR_SUCCESS L"\33[7m\33[K\33[27m\33[0m\n");

  if(argc < 1 || access(argv[1], R_OK))
  {
    WERR(L"file not found");
  }
  
  cs_context * context = cs_context_create();
  
  context->name = malloc((strlen(argv[1])+1)*sizeof(*context->name));
  if(context->name == NULL) WERR("Out of memory");
  strcpy(context->name, argv[1]);
  #ifdef CS_DO_READ
    context->text = file_read(argv[1]);
  #endif
  
  #ifdef CS_DO_LEX
    context->tokens = cs_lex_wcs(context->text, &context->error);
    if(context->tokens == NULL)
    {
      cs_error_display(context);
      wprintf(L"\n");
      return 1;
    }
    cs_tokens_display(context->tokens);
  #endif

  #ifdef CS_DO_PARSE
    wprintf(L"\n");
    context->ast = cs_node_create(CS_NT_STATEMENTS);
    context->ast->pos.start = context->tokens[context->token_index].pos.start;
    cs_node ** sequence = cs_parse_sequence(
      context->tokens, &context->token_index, &context->error,
      &cs_parse_statement, CS_TT_NEW, CS_TT_EOF
    );
    if(sequence == NULL)
    {
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"\n");
      #endif
      cs_error_display(context);
      wprintf(L"\n");
      cs_node_free(context->ast);
      return 1;
    }
    context->ast->pos.end = context->tokens[context->token_index].pos.end;
    context->ast->content.value._vpp = (void**)sequence;
  
    wchar_t * node_as_wcs = cs_node_to_wcs(context->ast);
    wprintf(node_as_wcs);
    free(node_as_wcs);
    wprintf(L"\n");
  #endif
  
  #ifdef CS_DO_INTERPRET
    context->variables_size = SIZE_SMALL; // FIXME: SIZE
    context->variables = malloc(context->variables_size*sizeof(*context->variables))
    if(context->variables == NULL) WERR("Out of memory");
    context->functions_size = SIZE_SMALL; // FIXME: SIZE
    context->functions = malloc(context->functions_size*sizeof(*context->functions))
    if(context->functions == NULL) WERR("Out of memory");
    cs_return return_value = cs_interpret(context->ast, context);
    if(return_value.type == CS_RT_VOID)
    {
      wprintf(L"\n");
      cs_error_display(context);
    }
    else
    {
      wchar_t * return_as_wcs = cs_return_to_wcs(return_value);
      wprintf(L"\n%ls\n", return_as_wcs);
      free(return_as_wcs);
    }
    cs_return_free(return_value);
  #endif

  #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"\n");
  #endif

  cs_context_free(context);
  
  #ifdef CS_DEBUG_MALLOC_COUNTER
    if(malloc_counter != 0)
    {
      wprintf(CS_COLOR_FAIL L"\n%d memory location(s) not freed.\33[0m", malloc_counter);
    }
    else
    {
      wprintf(CS_COLOR_SUCCESS L"\nAll memory locations freed.\33[0m");
    }
  #else
    wprintf(L"\n\33[97mMemory freed.\33[0m");
  #endif
  
  wprintf(L"\n");
}
#pragma endregion Main