/*
test-1.c – CMDRScript C Interpreter Test 1
Modified 2021-04-05

macOS
clear && gcc test-1.c -o test-1 && ./test-1 test.txt
clear && clang test-1.c -o test-1.exe && ./test-1.exe test-1-nodes.txt

Windows
cls & gcc test-1.c -o test-1.exe && test-1.exe test-1-nodes.txt
cls & tcc test-1.c -o test-1.exe && test-1.exe test-1-nodes.txt
cls & clang test-1.c -o test-1.exe -Wno-deprecated && test-1.exe test-1-nodes.txt

Visual Studio Code Developer Prompt (Administrator)
cls & cl test-1.c /Fetest-1.exe /Fo"%temp%\cs.obj"

Planned for test-2:
- Program Struct
  Stores:
  - tokens, token_index
  - ast (, node_index if using Memory Pool)
  - file(s?)
  - error
- Rework node data and branches:
  - Every node has up to 4 unions of either an int, float, wchar_t*, void**, or cs_node*.
  - The nodes are given generic names.
  - When defined, they are defined in the order that they appear in the syntax.
  - The nodes don't store the types of the individual unions, as those can be derived from the main node type.
- Memory Pool (test-2?)
  - Nodes are placed next to each other in a memory pool (node_index).
  - Branches are just pointers to indices of other nodes.
  UNSURE: If this extra effort worth it?
- Memory index for nodes to simplify freeing memory.
  UNSURE: Freeing memory as we go isn't that bad.
- Read tokens directly from a file (cs_lex_file).
  WONT DO: The file contents are needed to display the error.
  Rereading the file to display the error creates the chance for the file
  to have changed during the interpretation of the program.
- "äöü"... in identifiers.
  WONT DO: Effort/performance-loss to quality gain ratio isn't good enough.

CHECKLIST:
- Can atom include '(' highest_order_expression() ')'? (currently conditional_operation)
*/

#pragma region def

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// OS and compiler-specific differences
#if defined(__TINYC__)
  #include <io.h>
  #define CS_USE_OUTDATED_swprintf
#elif defined(__clang__)
  #include <io.h>
  #define R_OK 4
  #define access _access
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
#ifdef CS_USE_OUTDATED_swprintf
  #define swprintf(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

#define CS_DEBUG_MALLOC_COUNTER
#ifdef CS_DEBUG_MALLOC_COUNTER
  size_t malloc_counter = 0;
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

// #define CS_DEBUG_LOG_FREE

#define CS_DEBUG_SOFT_ERROR
/*
Experimental - Tries to avoid aborting the entire parse by
replacing invalid nodes with a string node.
*/
#define CREATE_ERROR_NODE \
  cs_node * error_node = cs_node_create(CS_NT_STR);\
  error_node->content.datum.type = CS_VT_WCS;\
  error_node->content.datum.value._wcs = CS_COLOR_FAIL L"ERROR HERE";
#define CREATE_ERROR_LIST \
  cs_node ** error_list = malloc(2 * sizeof(*error_list));\
  if(error_list == NULL) WERR(L"malloc");\
  CREATE_ERROR_NODE;\
  cs_node * error_end = cs_node_create(CS_NT_END);\
  error_list[0] = error_node;\
  error_list[1] = error_end;

#pragma endregion def

#pragma region wcs_to_int
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

#pragma region wcs_to_float
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

#pragma region [File]
typedef struct _cs_file {
  char * name;
  wchar_t * text;
} cs_file;

void cs_file_read(cs_file * out, char * path)
{
  out->name = malloc((strlen(path) + 1) * sizeof(*(out->name)));
  if(out->name == NULL) WERR(L"malloc");
  strcpy(out->name, path);
  size_t text_size = SIZE_MEDIUM;
  FILE * f = fopen(path, "r,ccs=UTF-8");
  if(f == NULL) WERR(L"file not found");
  out->text = malloc(text_size * sizeof(*(out->text)));
  if(out->text == NULL) WERR(L"malloc");
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
      wchar_t * _text = realloc(out->text, text_size * sizeof(*_text));
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

void cs_file_free(cs_file file)
{
  #ifdef CS_DEBUG_LOG_FREE
  wprintf(L"file\n");
  #endif
  free(file.name);
  free(file.text);
}
#pragma endregion [File]

#pragma region [Position]
typedef struct _cs_position {
  size_t start;
  size_t end;
} cs_position;
#pragma endregion [Position]

#pragma region [Value]
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
#pragma endregion [Value]

#pragma region [Datum]
typedef struct _cs_datum {
  cs_value_type type;
  cs_value value;
} cs_datum;
#pragma endregion [Datum]

#pragma region [Token]

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
  if(str == NULL) WERR(L"malloc");
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
               L":" CS_COLOR_TOKEN_VALUE L"%.2f" CS_COLOR_NEUTRAL,
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
  size_t i = 0;
  while(tokens[i].type != CS_TT_EOF)
  {
    #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"token:%ls\n", cs_token_type_to_wcs(tokens[i].type));
    #endif
    
    switch(tokens[i].type)
    {
      case CS_TT_ID:
      case CS_TT_STR:
        free(tokens[i].value._wcs);
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
        break;
      
      default: WERR(L"Unhandled token type in cs_tokens_free()!\n");
    }
    i++;
  }
  free(tokens);
}
#pragma endregion Free Tokens

#pragma endregion [Token]

#pragma region [Node]

#pragma region Enum: Node Type
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
  CS_NT_SET,
  CS_NT_GET,
  CS_NT_DEF,
  CS_NT_CALL,
  CS_NT_IDX,

  // Control Flow
  CS_NT_FOR,
  CS_NT_WHILE,
  CS_NT_IF,
  CS_NT_RETURN,
  CS_NT_BREAK,
  CS_NT_CONTINUE,
  CS_NT_STATEMENTS,
  CS_NT_END
  
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
      struct __condop {
        struct _cs_node * trueconditions;
        struct _cs_node * condition;
        struct _cs_node * falseconditions;
      } _condop;
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
    case CS_NT_SET: return L"SET";
    case CS_NT_GET: return L"GET";
    case CS_NT_DEF: return L"DEF";
    case CS_NT_CALL: return L"CALL";
    case CS_NT_IDX: return L"IDX";

    // Control Flow
    case CS_NT_FOR: return L"FOR";
    case CS_NT_WHILE: return L"WHILE";
    case CS_NT_IF: return L"IF";
    case CS_NT_RETURN: return L"RETURN";
    case CS_NT_BREAK: return L"BREAK";
    case CS_NT_CONTINUE: return L"CONTINUE";
    case CS_NT_STATEMENTS: return L"STATEMENTS";
    case CS_NT_END: return L"END";
    
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
  if(buffer == NULL) WERR(L"malloc");
  if(node == NULL)
  {
    wcscpy(buffer, CS_COLOR_HINT L"NULL" CS_COLOR_NEUTRAL);
    return buffer;
  }
  buffer[0] = L'\0';
  switch(node->type)
  {
    case CS_NT_STATEMENTS: {
      cs_node ** list = (cs_node**)node->content.datum.value._p;
      if(list == NULL) WERR(L"undefined statements pointer");
      if(list[0]->type == CS_NT_END)
      {
        swprintf(buffer, SIZE_BIG, CS_COLOR_HINT L"NO STATEMENTS" CS_COLOR_NEUTRAL);
        break;
      }
      wchar_t * node_as_wcs = cs_node_to_wcs(list[0]);
      int offset = swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"{%ls", node_as_wcs);
      free(node_as_wcs);
      int i = 1;
      while(list[i]->type != CS_NT_END)
      {
        node_as_wcs = cs_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, SIZE_BIG, CS_COLOR_NEUTRAL L"\n%ls", node_as_wcs);
        free(node_as_wcs);
        i++;
      }
      swprintf(buffer+offset, SIZE_BIG, CS_COLOR_NEUTRAL L"}");
      break; }
    
    case CS_NT_INT:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"INT" CS_COLOR_NEUTRAL L":" CS_COLOR_NODE_DATUM_VALUE L"%d" CS_COLOR_NEUTRAL,
               node->content.datum.value._int);
      break;
    
    case CS_NT_FLT:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"FLT" CS_COLOR_NEUTRAL L":" CS_COLOR_NODE_DATUM_VALUE L"%.2f" CS_COLOR_NEUTRAL,
               node->content.datum.value._float);
      break;
    
    case CS_NT_STR:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"STR" CS_COLOR_NEUTRAL L":" CS_COLOR_NODE_DATUM_VALUE L"\"%ls" CS_COLOR_NODE_DATUM_VALUE L"\"" CS_COLOR_NEUTRAL,
               node->content.datum.value._wcs);
      break;
    
    case CS_NT_ID:
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NODE_DATUM_TYPE L"ID" CS_COLOR_NEUTRAL L":" CS_COLOR_NODE_DATUM_VALUE L"%ls" CS_COLOR_NEUTRAL,
               node->content.datum.value._wcs);
      break;
    
    case CS_NT_LIST: {
      cs_node ** list = (cs_node**)node->content.datum.value._p;
      if(list == NULL) WERR(L"undefined list pointer");
      if(list[0]->type == CS_NT_END)
      {
        swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"[]");
        break;
      }
      wchar_t * node_as_wcs = cs_node_to_wcs(list[0]);
      int offset = swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"[%ls", node_as_wcs);
      free(node_as_wcs);
      int i = 1;
      while(list[i]->type != CS_NT_END)
      {
        node_as_wcs = cs_node_to_wcs(list[i]);
        offset += swprintf(buffer+offset, SIZE_BIG, CS_COLOR_NEUTRAL L", %ls", node_as_wcs);
        free(node_as_wcs);
        i++;
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
      wchar_t * left_branch_as_wcs = cs_node_to_wcs(node->content.branch._binop.left);
      wchar_t * right_branch_as_wcs = cs_node_to_wcs(node->content.branch._binop.right);
      swprintf(buffer, SIZE_BIG,
               CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"%ls" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               cs_node_type_to_wcs(node->type), left_branch_as_wcs, right_branch_as_wcs);
      free(left_branch_as_wcs);
      free(right_branch_as_wcs);
      break; }
    
    // Unary Operation
    case CS_NT_NEG:
    case CS_NT_NOT:
    case CS_NT_RETURN: {
      wchar_t * center_branch_as_wcs = cs_node_to_wcs(node->content.branch._unop.center);
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"%ls" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L")",
               cs_node_type_to_wcs(node->type), center_branch_as_wcs);
      free(center_branch_as_wcs);
      break; }
    
    // Without operation
    case CS_NT_BREAK:
    case CS_NT_CONTINUE: {
      swprintf(buffer, SIZE_BIG, CS_COLOR_NODE_BRANCH_TYPE L"%ls" CS_COLOR_NEUTRAL, cs_node_type_to_wcs(node->type));
      break; }
    
    case CS_NT_FOR: {
      wchar_t * counter_branch_as_wcs = cs_node_to_wcs(node->content.branch._for.counter);
      wchar_t * from_branch_as_wcs = cs_node_to_wcs(node->content.branch._for.from);
      wchar_t * to_branch_as_wcs = cs_node_to_wcs(node->content.branch._for.to);
      wchar_t * body_branch_as_wcs = cs_node_to_wcs(node->content.branch._for.body);
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"FOR" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               counter_branch_as_wcs, from_branch_as_wcs, to_branch_as_wcs, body_branch_as_wcs);
      free(counter_branch_as_wcs);
      free(from_branch_as_wcs);
      free(to_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }
    
    case CS_NT_WHILE: {
      wchar_t * condition_branch_as_wcs = cs_node_to_wcs(node->content.branch._while.condition);
      wchar_t * body_branch_as_wcs = cs_node_to_wcs(node->content.branch._while.body);
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"WHILE" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               condition_branch_as_wcs, body_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }

    case CS_NT_IF: {
      wchar_t * condition_branch_as_wcs = cs_node_to_wcs(node->content.branch._if.condition);
      wchar_t * truebody_branch_as_wcs = cs_node_to_wcs(node->content.branch._if.truebody);
      wchar_t * falsebody_branch_as_wcs = cs_node_to_wcs(node->content.branch._if.falsebody);
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE "IF" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               condition_branch_as_wcs, truebody_branch_as_wcs, falsebody_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(truebody_branch_as_wcs);
      free(falsebody_branch_as_wcs);
      break; }
    
    case CS_NT_COP: {
      wchar_t * condition_branch_as_wcs = cs_node_to_wcs(node->content.branch._condop.condition);
      wchar_t * trueconditions_branch_as_wcs = cs_node_to_wcs(node->content.branch._condop.trueconditions);
      wchar_t * falseconditions_branch_as_wcs = cs_node_to_wcs(node->content.branch._condop.falseconditions);
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"COP" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               condition_branch_as_wcs, trueconditions_branch_as_wcs, falseconditions_branch_as_wcs);
      free(condition_branch_as_wcs);
      free(trueconditions_branch_as_wcs);
      free(falseconditions_branch_as_wcs);
      break; }
    
    case CS_NT_DEF: {
      wchar_t * name_branch_as_wcs = cs_node_to_wcs(node->content.branch._fndef.name);
      wchar_t * params_branch_as_wcs = cs_node_to_wcs(node->content.branch._fndef.params);
      wchar_t * body_branch_as_wcs = cs_node_to_wcs(node->content.branch._fndef.body);
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(" CS_COLOR_NODE_BRANCH_TYPE L"DEF" CS_COLOR_NEUTRAL L" %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L", %ls" CS_COLOR_NEUTRAL L")",
               name_branch_as_wcs, params_branch_as_wcs, body_branch_as_wcs);
      free(name_branch_as_wcs);
      free(params_branch_as_wcs);
      free(body_branch_as_wcs);
      break; }

    default:
      swprintf(buffer, SIZE_BIG, CS_COLOR_NEUTRAL L"(UNKNOWN NODE TYPE OR MEMORY)");
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
    wprintf(L"node:NULL ");
    #endif
    return;
  }
  #ifdef CS_DEBUG_LOG_FREE
  wprintf(L"node:%ls\n", cs_node_type_to_wcs(node->type));
  #endif

  switch(node->type)
  {
    case CS_NT_INT:
    case CS_NT_FLT:
    case CS_NT_BREAK:
    case CS_NT_CONTINUE:
    case CS_NT_END:
      free(node);
      break;
    
    case CS_NT_STR:
    case CS_NT_ID:
      /*
      We don't free these pointers because they are pointing at data stored in the tokens,
      which may still be needed after the parse. This memory is freed alongside the tokens.
      // free(node->content.datum.value._wcs);
      */
      free(node);
      break;
      
    case CS_NT_LIST:
    case CS_NT_STATEMENTS: {
      cs_node ** list = (cs_node**)node->content.datum.value._p;
      if(list == NULL)
      {
        // wprintf(L"Warning: Node to be freed has undefined list pointer.\n");
        break;
      }
      size_t i = 0;
      while(list[i]->type != CS_NT_END) cs_node_free(list[i++]);
      cs_node_free(list[i]); // END
      free(list);
      free(node);
      break; }
  
    case CS_NT_NEG:
    case CS_NT_NOT:
    case CS_NT_GET:
    case CS_NT_RETURN:
      cs_node_free(node->content.branch._unop.center);
      free(node);
      break;
    
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
    case CS_NT_IDX: {
      cs_node_free(node->content.branch._binop.left);
      cs_node_free(node->content.branch._binop.right);
      free(node);
      break;
    }
    
    case CS_NT_COP:
      cs_node_free(node->content.branch._condop.trueconditions);
      cs_node_free(node->content.branch._condop.condition);
      cs_node_free(node->content.branch._condop.falseconditions);
      free(node);
      break;
  
    case CS_NT_DEF:
      cs_node_free(node->content.branch._fndef.name);
      cs_node_free(node->content.branch._fndef.params);
      cs_node_free(node->content.branch._fndef.body);
      free(node);
      break;
  
    case CS_NT_FOR:
      cs_node_free(node->content.branch._for.counter);
      cs_node_free(node->content.branch._for.from);
      cs_node_free(node->content.branch._for.to);
      cs_node_free(node->content.branch._for.body);
      free(node);
      break;
    
    case CS_NT_WHILE:
      cs_node_free(node->content.branch._while.condition);
      cs_node_free(node->content.branch._while.body);
      free(node);
      break;
    
    case CS_NT_IF:
      cs_node_free(node->content.branch._if.condition);
      cs_node_free(node->content.branch._if.truebody);
      cs_node_free(node->content.branch._if.falsebody);
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
  if(node == NULL) WERR(L"malloc");
  node->type = type;
  node->pos = (cs_position){0, 0};
  node->content.branch._for.counter = NULL; // FIXME:
  node->content.branch._for.from = NULL;
  node->content.branch._for.to = NULL;
  node->content.branch._for.body = NULL;
  return node;
}
#pragma endregion Create Node

#pragma endregion [Node]

#pragma region [Error]

#pragma region Enum: Error Type
typedef enum _cs_error_type {
  CS_ET_NO_ERROR, // Debugging
  CS_ET_EXPECTED_TOKEN,
  CS_ET_UNEXPECTED_TOKEN,
  CS_ET_ILLEGAL_CHARACTER,
  CS_ET_INCOMPLETE_FLOAT
} cs_error_type;
#pragma endregion Enum: Error Type

#pragma region Struct: Error
typedef struct _cs_error {
  cs_error_type type;
  cs_position pos;
  cs_datum info;
  char __file__[4096]; // Debugging
  size_t __line__;     // ''
} cs_error;
#pragma endregion Struct: Error

#pragma region Error Info to WCS
wchar_t * cs_error_info_to_wcs(cs_error_type type, cs_datum info)
{
  wchar_t * wcs = malloc(SIZE_MEDIUM * sizeof(*wcs));
  if(wcs == NULL) WERR(L"malloc");
  switch(type)
  {
    case CS_ET_NO_ERROR:
      swprintf(wcs, SIZE_MEDIUM, L"Undefined Error!");
      break;
    
    case CS_ET_EXPECTED_TOKEN:
      swprintf(wcs, SIZE_MEDIUM, L"Expected Token: %ls", cs_token_type_to_wcs((cs_token_type)info.value._int));
      break;
    
    case CS_ET_UNEXPECTED_TOKEN:
      swprintf(wcs, SIZE_MEDIUM, L"Unexpected Token: %ls", cs_token_type_to_wcs((cs_token_type)info.value._int));
      break;
    
    case CS_ET_ILLEGAL_CHARACTER:
      swprintf(wcs, SIZE_MEDIUM, L"Illegal Character: '%lc'", (wchar_t)info.value._int);
      break;
    
    case CS_ET_INCOMPLETE_FLOAT:
      swprintf(wcs, SIZE_MEDIUM, L"Expected digit after '.'");
      break;
    
    default:
      swprintf(wcs, SIZE_MEDIUM, L"Unknown Error!");
      break;
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
  int line_end = 0;
  char location_found = FALSE;
  for(int i=0; i<wcslen(file.text)+1 /* Catch '\0' */ ; i++)
  {
    if(i == error.pos.start) location_found = TRUE;
    if(location_found && (file.text[i] == L'\n' || file.text[i] == L'\0'))
    {
      line_end = i;
      break;
    }
    if(file.text[i] == L'\n')
    {
      line++;
      line_begin = i+1;
    }
  }
  // FIXME: 1m?
  wprintf(CS_COLOR_NEUTRAL L"\33[1mFile %hs, Line %d (%hs @ %d):\33[0m" CS_COLOR_NEUTRAL L"\n%.*ls\n",
          file.name, line, error.__file__, error.__line__,
          line_end-line_begin,file.text+line_begin);
  wprintf(L"\33[%dC%ls\33[1m" CS_COLOR_FAIL,
          error.pos.start-line_begin, (error.pos.start-line_begin > 0) ? L"" : L"\33[D");
  if(error.pos.start>error.pos.end) WERR(L"error.pos.start > error.pos.end\n"); // DEBUG: For debugging only.
  for(int i=0; i<error.pos.end-error.pos.start; i++) wprintf(L"~");
  wchar_t * error_info_as_wcs = cs_error_info_to_wcs(error.type, error.info);
  wprintf(CS_COLOR_FAIL L"\33[1m\n%ls\n\n\33[0m" CS_COLOR_HINT L"error.pos.start: %d\nerror.pos.end: %d\nline_begin: %d\nline_end: %d\33[0m\n",
          error_info_as_wcs,
          error.pos.start, error.pos.end, line_begin, line_end); // DEBUG: For debugging only.
  free(error_info_as_wcs);
}
#pragma endregion Display Error

#pragma endregion [Error]

#pragma region --- Lexer ---
cs_token * cs_lex_wcs(cs_file file)
{
  size_t tokens_size = SIZE_MEDIUM; // FIXME:
  cs_token * tokens = malloc(tokens_size * sizeof(*tokens));
  if(tokens == NULL) WERR(L"malloc");
  size_t tokens_index = 0;
  size_t idx = 0;
  while(TRUE)
  {
    if(tokens_index+1 >= tokens_size)
    {
      tokens_size *= 2;
      cs_token * _tokens = realloc(tokens, tokens_size * sizeof(*_tokens));
      if(_tokens == NULL) WERR(L"realloc");
      tokens = _tokens;
      wprintf(L"Warning: Tokens doubled\n");
    }
    
    #pragma region Number
    if(file.text[idx] >= L'0' && file.text[idx] <= L'9')
    {
      size_t size = SIZE_SMALL;
      wchar_t * buffer = malloc(size * sizeof(*buffer));
      if(buffer == NULL) WERR(L"malloc");
      size_t idx_left = idx;
      buffer[0] = file.text[idx];
      idx++;
      // FIXME: NOT DYNAMIC
      while(file.text[idx] >= L'0' && file.text[idx] <= L'9') // Implies [idx] != 0
      {
        buffer[idx-idx_left] = file.text[idx];
        idx++;
      }
      buffer[idx-idx_left] = L'\0';
      if(file.text[idx] != L'.')
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
      while(file.text[idx] >= L'0' && file.text[idx] <= L'9') // Implies [idx] != 0
      {
        buffer[idx-idx_left] = file.text[idx];
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
    if(file.text[idx] == L'"')
    {
      size_t idx_left = idx;
      idx++;
      size_t string_size = SIZE_MEDIUM; // FIXME:
      wchar_t * string = malloc(string_size * sizeof(*string));
      if(string == NULL) WERR(L"malloc");
      size_t string_index = 0;
      char escape = FALSE;
      while(TRUE)
      {
        if(file.text[idx] == L'\0') WERR(L"expected string termination");
        if(string_index >= string_size)
        {
          string_size *= 2;
          wchar_t * _string = realloc(string, string_size * sizeof(*_string));
          if(_string == NULL) WERR(L"realloc");
          string = _string;
        }
        if(escape)
        {
          escape = FALSE;
          switch(file.text[idx])
          {
            case L'\\':
            case L'"':
              string[string_index] = file.text[idx];
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
          if(file.text[idx] == L'\\')
          {
            escape = TRUE;
            idx++;
            continue;
          }
          if(file.text[idx] == L'"')
          {
            idx++;
            break;
          }
          string[string_index] = file.text[idx];
        }
        idx++;
        string_index++;
      }
      string[string_index] = L'\0';
      cs_token tk = {CS_TT_STR, (cs_position){idx_left, idx}};
      tk.value._wcs = string;
      tokens[tokens_index++] = tk;
      continue;
    }
    #pragma endregion String
    
    #pragma region Identifier (+ Clause Keywords)
    if((file.text[idx] >= L'a' && file.text[idx] <= L'z')
    || (file.text[idx] >= L'A' && file.text[idx] <= L'Z')
    ||  file.text[idx] == L'_')
    {
      size_t idx_left = idx;
      size_t id_size = SIZE_MEDIUM; // FIXME:
      wchar_t * id = malloc(id_size * sizeof(*id));
      if(id == NULL) WERR(L"malloc");
      id[0] = file.text[idx];
      idx++;
      size_t id_index = 1;
      while((file.text[idx] >= L'a' && file.text[idx] <= L'z')
         || (file.text[idx] >= L'A' && file.text[idx] <= L'Z')
         || (file.text[idx] >= L'0' && file.text[idx] <= L'9')
         ||  file.text[idx] == L'_')
      {
        id[id_index] = file.text[idx];
        idx++;
        id_index++;
        if(id_index >= id_size)
        {
          id_size *= 2;
          wchar_t * _id = realloc(id, id_size * sizeof(*_id));
          if(_id == NULL) WERR(L"realloc");
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
    if(file.text[idx] == L'(')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_LPAR, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L')')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_RPAR, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'{')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_LBRC, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'}')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_RBRC, (cs_position){idx, idx+1}, 0};
      tokens[tokens_index++] = (cs_token){CS_TT_NEW, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'[')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_LSQB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L']')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_RSQB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L',')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_SEP, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'\0')
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
    if(file.text[idx] == L'=')
    {
      idx++;
      if(file.text[idx] == L'=')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_EQU, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_SET, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(file.text[idx] == L'+')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_ADD, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'-')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_SUB, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'*')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_MUL, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'/')
    {
      idx++;
      if(file.text[idx] == L'/')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_FDIV, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_DIV, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(file.text[idx] == L'%')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_MOD, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    if(file.text[idx] == L'^')
    {
      tokens[tokens_index++] = (cs_token){CS_TT_POW, (cs_position){idx, idx+1}, 0};
      idx++;
      continue;
    }
    #pragma endregion Operators (+ Logical Equal)

    #pragma region Comparisons (- Logical Equal)
    if(file.text[idx] == L'!')
    {
      if(file.text[idx+1] == L'=')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_NEQ, (cs_position){idx, idx+2}, 0};
        idx += 2;
        continue;
      }
      // FIXME:
      error.type = CS_ET_ILLEGAL_CHARACTER;
      error.pos = (cs_position){idx, idx+1};
      error.info.value._int = (int)file.text[idx];
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      return NULL;
    }
    if(file.text[idx] == L'>')
    {
      idx++;
      if(file.text[idx] == L'=')
      {
        tokens[tokens_index++] = (cs_token){CS_TT_GEQ, (cs_position){idx-1, idx+1}, 0};
        idx++;
        continue;
      }
      tokens[tokens_index++] = (cs_token){CS_TT_GTR, (cs_position){idx, idx+1}, 0};
      continue;
    }
    if(file.text[idx] == L'<')
    {
      idx++;
      if(file.text[idx] == L'=')
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
         || file.text[idx] == L';') idx++;
      if(tokens_index == 0
      || tokens[tokens_index-1].type != CS_TT_NEW)
      {
        tokens[tokens_index++] = (cs_token){CS_TT_NEW, (cs_position){idx_left, idx}, 0};
      }
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
#pragma endregion --- Lexer ---

#pragma region --- Parser ---

#pragma region (Prototypes)
cs_node * cs_parse_block(cs_token * tokens, size_t * token_index);
cs_node ** cs_parse_sequence(
  cs_token * tokens, size_t * token_index,
  cs_node * (*parser)(cs_token * tokens, size_t * token_index),
  cs_token_type tt_end_of_item,
  cs_token_type tt_end_of_sequence
);
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_if(cs_token * tokens, size_t * token_index, cs_token_type start_token);
cs_node * cs_parse_for(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_while(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_def(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_return(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_conditional_operation(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_conditions(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_condition(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_expression(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_term(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_power(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_index(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_atom(cs_token * tokens, size_t * token_index);
cs_node * cs_parse_id(cs_token * tokens, size_t * token_index);
#pragma endregion (Prototypes)

#pragma region Parse Block
cs_node * cs_parse_block(cs_token * tokens, size_t * token_index)
{
  // {
  if(tokens[*token_index].type != CS_TT_LBRC)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_LBRC;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
    tokens, token_index,
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
  block->content.datum.type = CS_VT_VOIDPP;
  block->content.datum.value._p = (void**)sequence;
  
  if(sequence[0]->type == CS_NT_END)
  {
    cs_node_free(sequence[0]);
    free(sequence);
    error.type = CS_ET_UNEXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_RBRC;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
    error.type = CS_ET_EXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_RBRC;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
  cs_token * tokens, size_t * token_index,
  cs_node * (*parser)(cs_token * tokens, size_t * token_index),
  cs_token_type tt_end_of_item,
  cs_token_type tt_end_of_sequence
)
{
  size_t sequence_size = SIZE_MEDIUM; // FIXME:
  cs_node ** sequence = malloc(sequence_size * sizeof(*sequence));
  if(sequence == NULL) WERR(L"malloc");
  
  size_t pos_start = tokens[*token_index].pos.start;

  size_t sequence_index = 0;

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
      if(_sequence == NULL) WERR(L"realloc");
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
      error.type = CS_ET_UNEXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)CS_TT_EOF;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_LIST;
        return error_list;
      #else
        return NULL;
      #endif
    }

    // Get item.
    sequence[sequence_index] = (*parser)(tokens, token_index);
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
      error.type = CS_ET_EXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)tt_end_of_item;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_LIST;
        return error_list;
      #else
        return NULL;
      #endif
    }
  }

  cs_node * end = cs_node_create(CS_NT_END);
  sequence[sequence_index] = end;

  return sequence;
}
#pragma endregion Parse Sequence

#pragma region Parse Statement
cs_node * cs_parse_statement(cs_token * tokens, size_t * token_index)
{
  if(tokens[*token_index].type == CS_TT_NEW) (*token_index)++; // FIXME: Unsure...
  
  switch(tokens[*token_index].type)
  {
    case CS_TT_FOR: return cs_parse_for(tokens, token_index);
    case CS_TT_IF: return cs_parse_if(tokens, token_index, CS_TT_IF);
    case CS_TT_WHILE: return cs_parse_while(tokens, token_index);
    case CS_TT_DEF: return cs_parse_def(tokens, token_index);
    case CS_TT_LBRC: return cs_parse_block(tokens, token_index);
    case CS_TT_RETURN: return cs_parse_return(tokens, token_index);
    
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
      
      cs_node * target = cs_parse_id(tokens, token_index);
      if(target == NULL)
      {
        #ifdef CS_DEBUG_SOFT_ERROR
          CREATE_ERROR_NODE;
          target = error_node;
        #else
          return NULL;
        #endif
      }
      
      if(tokens[*token_index].type == CS_TT_LSQB)
      {
        (*token_index)++;
        
        cs_node * position = cs_parse_conditional_operation(tokens, token_index);
        if(position == NULL)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            position = error_node;
          #else
            cs_node_free(target);
            return NULL;
          #endif
        }
        
        if(tokens[*token_index].type != CS_TT_RSQB)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            return error_node;
          #else
            cs_node_free(target);
            cs_node_free(position);
            return NULL;
          #endif
        }

        cs_node * index = cs_node_create(CS_NT_IDX);
        index->pos.start = target->pos.start;
        index->pos.end = tokens[*token_index].pos.end;
        (*token_index)++; // ]
        index->content.branch._binop.left = target;
        index->content.branch._binop.right = position;
        target = index;
      }
      
      if(tokens[*token_index].type == CS_TT_SET)
      {
        (*token_index)++;
        
        cs_node * conditional_operation = cs_parse_conditional_operation(tokens, token_index);
        if(conditional_operation == NULL)
        {
          #ifdef CS_DEBUG_SOFT_ERROR
            CREATE_ERROR_NODE;
            conditional_operation = error_node;
          #else
            cs_node_free(target);
            return NULL;
          #endif
        }
        
        cs_node * varset = cs_node_create(CS_NT_SET);
        varset->pos.start = target->pos.start;
        varset->pos.end = conditional_operation->pos.end;
        varset->content.branch._binop.left = target;
        varset->content.branch._binop.right = conditional_operation;
        return varset;
      }
      else
      {
        cs_node_free(target);
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
      cs_node * expression = cs_parse_conditional_operation(tokens, token_index);
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
  cs_node * condition = cs_parse_conditions(tokens, token_index);
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
  cs_node * truebody = cs_parse_statement(tokens, token_index);
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
  ifstatement->content.branch._if.condition = condition;
  ifstatement->content.branch._if.truebody = truebody;

  /*
  Whitespace. This counts for both 'elif' and 'else' because 'elif'
  creates an entirely new if node where this statement then removes
  whitespace in front of 'else'.
  */
  if(tokens[*token_index].type == CS_TT_NEW) (*token_index)++;

  // elif ...
  if(tokens[*token_index].type == CS_TT_ELIF)
  {
    cs_node * falsebody = cs_parse_if(tokens, token_index, CS_TT_ELIF);
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
    ifstatement->content.branch._if.falsebody = falsebody;
  }

  // else statement
  else if(tokens[*token_index].type == CS_TT_ELSE)
  {
    (*token_index)++;
    cs_node * falsebody = cs_parse_statement(tokens, token_index);
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
  cs_node * counter = cs_parse_id(tokens, token_index);
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
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_FROM;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
  cs_node * from = cs_parse_conditions(tokens, token_index);
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
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_TO;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
  cs_node * to = cs_parse_conditions(tokens, token_index);
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
  cs_node * body = cs_parse_statement(tokens, token_index);
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
  cs_node * condition = cs_parse_condition(tokens, token_index);
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
  cs_node * body = cs_parse_statement(tokens, token_index);
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
  node->content.branch._while.condition = condition;
  node->content.branch._while.body = body;
  return node;
}
#pragma endregion Parse While Loop

#pragma region Parse Function Definition
// Return Node: CS_NT_DEF
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
  cs_node * name = cs_parse_id(tokens, token_index);
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
    error.type = CS_ET_EXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_LPAR;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
    tokens, token_index,
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

  params->content.datum.type = CS_VT_VOIDPP;
  params->content.datum.value._p = (void**)sequence;

  // { ... }
  cs_node * body = cs_parse_statement(tokens, token_index);
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
  fndef->content.branch._fndef.name = name;
  fndef->content.branch._fndef.params = params;
  fndef->content.branch._fndef.body = body;

  return fndef;
}
#pragma endregion Parse Function Definition

#pragma region Parse Conditional Operation
cs_node * cs_parse_conditional_operation(cs_token * tokens, size_t * token_index)
{
  cs_node * trueconditions = cs_parse_conditions(tokens, token_index);
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
    
    cs_node * condition = cs_parse_conditions(tokens, token_index);
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
      error.type = CS_ET_EXPECTED_TOKEN;
      error.info.value._int = (int)CS_TT_ELSE;
      error.pos = tokens[*token_index].pos;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
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
    
    cs_node * falseconditions = cs_parse_conditional_operation(tokens, token_index);
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
    conditional_operation->content.branch._condop.trueconditions = trueconditions;
    conditional_operation->content.branch._condop.condition = condition;
    conditional_operation->content.branch._condop.falseconditions = falseconditions;
    trueconditions = conditional_operation;
  }
  
  return trueconditions;
}
#pragma endregion Parse Conditional Operation

#pragma region Parse Conditions
cs_node * cs_parse_conditions(cs_token * tokens, size_t * token_index)
{
  cs_node * left = cs_parse_condition(tokens, token_index);
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
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_condition(tokens, token_index);
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
  if(tokens[*token_index].type == CS_TT_NOT)
  {
    cs_node * not = cs_node_create(CS_NT_NOT);
    not->pos.start = tokens[*token_index].pos.start;
    (*token_index)++;
    
    cs_node * condition = cs_parse_condition(tokens, token_index);
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
    not->content.branch._unop.center = condition;
    
    return not;
  }
  
  cs_node * left = cs_parse_expression(tokens, token_index);
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
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_expression(tokens, token_index);
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
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_term(tokens, token_index);
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
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_power(tokens, token_index);
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
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    // FIXME: RECURSIVE WHEN IT DOESNT HAVE TO BE, SEE EXPRESSION
    cs_node * right = cs_parse_power(tokens, token_index);
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
    new_left->content.branch._binop.left = left;
    (*token_index)++;

    cs_node * right = cs_parse_conditions(tokens, token_index);
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
    new_left->content.branch._binop.right = right;
    
    if(tokens[*token_index].type != CS_TT_RSQB)
    {
      error.type = CS_ET_EXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)CS_TT_RSQB;
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
      #ifdef CS_DEBUG_SOFT_ERROR
        CREATE_ERROR_NODE;
        return error_node;
      #else
        cs_node_free(new_left);
        return NULL;
      #endif
    }
    new_left->pos.end = right->pos.end;
    (*token_index)++;

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
      cs_node * neg = cs_node_create(CS_NT_NEG);
      neg->pos.start = tokens[*token_index].pos.start;
      (*token_index)++;
      cs_node * expression = cs_parse_conditions(tokens, token_index);
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
      neg->content.branch._unop.center = expression;
      return neg; }

    case CS_TT_INT: {
      cs_node * num = cs_node_create(CS_NT_INT);
      num->pos = tokens[*token_index].pos;
      num->content.datum.type = CS_VT_INT;
      num->content.datum.value._int = tokens[*token_index].value._int;
      (*token_index)++;
      return num; }
    
    case CS_TT_FLT: {
      cs_node * num = cs_node_create(CS_NT_FLT);
      num->pos = tokens[*token_index].pos;
      num->content.datum.type = CS_VT_FLOAT;
      num->content.datum.value._float = tokens[*token_index].value._float;
      (*token_index)++;
      return num; }
    
    case CS_TT_STR: {
      cs_node * str = cs_node_create(CS_NT_STR);
      str->pos = tokens[*token_index].pos;
      str->content.datum.type = CS_VT_WCS;
      str->content.datum.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;
      return str; }
    
    case CS_TT_ID: {
      cs_node * node = cs_node_create(CS_NT_ID);
      node->pos = tokens[*token_index].pos;
      node->content.datum.type = CS_VT_WCS;
      node->content.datum.value._wcs = tokens[*token_index].value._wcs;
      (*token_index)++;

      // Function Call
      if(tokens[*token_index].type == CS_TT_LPAR)
      {
        cs_node * fncall = cs_node_create(CS_NT_CALL);
        fncall->pos.start = node->pos.start;
        fncall->content.branch._binop.left = node;

        cs_node * params = cs_node_create(CS_NT_LIST);
        params->pos.start = tokens[*token_index].pos.start;
        (*token_index)++;
        
        cs_node ** sequence = cs_parse_sequence(
          tokens, token_index,
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
        params->content.datum.type = CS_VT_VOIDPP;
        params->content.datum.value._p = (void**)sequence;
        params->pos.end = tokens[*token_index].pos.end;
        (*token_index)++;

        fncall->pos.end = params->pos.end;
        fncall->content.branch._binop.right = params;
        node = fncall;
      }
      
      return node; }
    
    case CS_TT_LSQB: {
      cs_node * list = cs_node_create(CS_NT_LIST);
      list->pos.start = tokens[*token_index].pos.start;
      // [
      (*token_index)++;
      // ...
      cs_node ** sequence = cs_parse_sequence(
        tokens, token_index,
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
      list->content.datum.type = CS_VT_VOIDPP;
      list->content.datum.value._p = (void**)sequence;
      return list; }
    
    case CS_TT_LPAR: {
      (*token_index)++;
      cs_node * expression = cs_parse_conditional_operation(tokens, token_index); // FIXME: Magic!
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
        error.type = CS_ET_EXPECTED_TOKEN;
        error.pos = tokens[*token_index].pos;
        error.info.type = CS_VT_INT;
        error.info.value._int = (int)CS_TT_RPAR;
        error.__line__ = __LINE__;
        strcpy(error.__file__, __FILE__);
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
      error.type = CS_ET_UNEXPECTED_TOKEN;
      error.pos = tokens[*token_index].pos;
      error.info.type = CS_VT_INT;
      error.info.value._int = (int)(tokens[*token_index].type);
      error.__line__ = __LINE__;
      strcpy(error.__file__, __FILE__);
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
cs_node * cs_parse_id(cs_token * tokens, size_t * token_index)
{
  if(tokens[*token_index].type != CS_TT_ID)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.info.value._int = CS_TT_ID;
    error.pos = tokens[*token_index].pos;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
  id->content.datum.value._wcs = tokens[*token_index].value._wcs;
  (*token_index)++;
  return id;
}
#pragma endregion Parse Identifier

#pragma region Parse Return Statement
cs_node * cs_parse_return(cs_token * tokens, size_t * token_index)
{
  cs_node * returnnode = cs_node_create(CS_NT_RETURN);
  returnnode->pos.start = tokens[*token_index].pos.start;

  // return
  if(tokens[*token_index].type != CS_TT_RETURN)
  {
    error.type = CS_ET_EXPECTED_TOKEN;
    error.pos = tokens[*token_index].pos;
    error.info.type = CS_VT_INT;
    error.info.value._int = (int)CS_TT_RETURN;
    error.__line__ = __LINE__;
    strcpy(error.__file__, __FILE__);
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
    value = cs_parse_conditions(tokens, token_index);
    if(value == NULL) WERR(L"malloc");
  }
  else
  {
    // wprintf(L"Warning: Implicit return value of INT:0\n");
    value = cs_node_create(CS_NT_INT);
    value->pos = (cs_position){tokens[*token_index].pos.start, tokens[*token_index].pos.start};
    value->content.datum.type = CS_VT_INT; // FIXME: datum.type is obsolete?
    value->content.datum.value._int = 0;
  }
  
  returnnode->pos.end = value->pos.end;
  returnnode->content.branch._unop.center = value;
  
  return returnnode;
}
#pragma endregion Parse Return Statement

#pragma endregion --- Parser ---

#pragma region main
int main(int argc, char * argv[])
{
  // Prints a bar to distinguish new output from old output in the terminal.
  wprintf(CS_COLOR_SUCCESS L"\33[7m\33[K\33[27m\33[0m\n");

  if(argc < 1 || access(argv[1], R_OK))
  {
    WERR(L"file not found");
  }
  
  #ifdef CS_DO_READ
    // Read file.
    cs_file file = {0};
    cs_file_read(&file, argv[1]);
  #endif
  
  #ifdef CS_DO_LEX
    // Lex text.
    cs_token * tokens = cs_lex_wcs(file);
    if(tokens == NULL)
    {
      cs_error_display(file);
      wprintf(L"\n");
      return 1;
    }
    cs_tokens_display(tokens);
  #endif

  #ifdef CS_DO_PARSE
    // Parse tokens.
    wprintf(L"\n");
    
    size_t token_index = 0;
    cs_node * ast = cs_node_create(CS_NT_STATEMENTS);
    ast->pos.start = tokens[token_index].pos.start;
    cs_node ** sequence = cs_parse_sequence(
      tokens, &token_index,
      &cs_parse_statement, CS_TT_NEW, CS_TT_EOF
    );
    if(sequence == NULL)
    {
      #ifdef CS_DEBUG_LOG_FREE
        wprintf(L"\n");
      #endif
      cs_error_display(file);
      wprintf(L"\n");
      cs_node_free(ast);
      return 1;
    }
    ast->pos.end = tokens[token_index].pos.end;
    ast->content.datum.type = CS_VT_VOIDPP;
    ast->content.datum.value._p = (void**)sequence;
  
    wchar_t * node_as_wcs = cs_node_to_wcs(ast);
    wprintf(node_as_wcs);
    free(node_as_wcs);
    wprintf(L"\n");
  #endif

  // cs_node ** inspect_nodes = (cs_node**)ast->content.datum.value._p;
  // wprintf(L"%d*%.2f [%ls]", inspect_nodes[0]->content.branch._binop.left->content.datum.value._int, inspect_nodes[0]->content.branch._binop.right->content.datum.value._float, cs_node_type_to_wcs(inspect_nodes[0]->content.branch._binop.right->type));

  #ifdef CS_DEBUG_LOG_FREE
    wprintf(L"\n");
  #endif

  #ifdef CS_DO_PARSE
    cs_node_free(ast);
  #endif
  #ifdef CS_DO_LEX
    cs_tokens_free(tokens);
  #endif
  #ifdef CS_DO_READ
    cs_file_free(file);
  #endif
  
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
#pragma endregion main