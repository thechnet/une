/*
primitive.h - Une
Modified 2021-07-15
*/

#ifndef UNE_PRIMITIVE_H
#define UNE_PRIMITIVE_H

/*
*** Options.
*/
#define UNE_DEBUG_MEMDBG
#define UNE_DEBUG_SIZE 1
#define UNE_DEBUG_RETURN_ERROR_TYPE

// #define UNE_NO_LEX
// #define UNE_NO_PARSE
// #define UNE_NO_INTERPRET

// #define UNE_DISPLAY_TOKENS
// #define UNE_DISPLAY_NODES
#define UNE_DISPLAY_RESULT
#define UNE_DEBUG_DISPLAY_EXTENDED_ERROR

// #define UNE_DEBUG_LOG_INTERPRET
// #define UNE_DEBUG_LOG_PARSE
// #define UNE_DEBUG_LOG_FREE

/* TO TOGGLE ESCAPE SEQUENCES, SEE ESCSEQ.H. */

/* Universal includes. */
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>

#define ESCSEQ_WIDE
#include "util/escseq.h"

#define LOGGING_WIDE
#define LOGGING_ID "une"
#include "util/logging.h"

#if defined(UNE_DEBUG) && defined(UNE_DEBUG_MEMDBG)
#define MEMDBG_ENABLE
#endif
#include "util/memdbg.h"

/*
*** Constants.
*/
#define UNE_DEFAULT_CONTEXT_NAME L"<stdin>"
#define UNE_STDIN_SWITCH "-s"
#define UNE_FOPEN_RFLAGS "r,ccs=UTF-8"
#define UNE_FOPEN_WFLAGS "w,ccs=UTF-8"

/* Sizes. */
#define UNE_SIZE_NODE_AS_WCS 32767 /* (Debug) Representing. */
#define UNE_SIZE_TOKEN_AS_WCS 4096 /* (Debug) Representing. */
#define UNE_SIZE_FGETWS_BUFFER 32767 /* une_builtin_input. */
#if !defined(UNE_DEBUG) || !defined(UNE_DEBUG_SIZE)
#define UNE_SIZE_NUM_LEN 32 /* Lexing, representing. */
#define UNE_SIZE_STR_LEN 4096 /* Lexing. */
#define UNE_SIZE_ID_LEN 32 /* Lexing. */
#define UNE_SIZE_VARIABLE_BUF 32 /* Context. */
#define UNE_SIZE_FUNCTION_BUF 16 /* Context. */
#define UNE_SIZE_TOKEN_BUF 4096 /* Lexing. */
#define UNE_SIZE_SEQUENCE 256 /* Parsing. */
#define UNE_SIZE_FILE_BUFFER 4096 /* une_file_read. */
#define UNE_SIZE_BIF_SPLIT_TKS 16 /* une_buitlin_split. */
#define UNE_SIZE_EXPECTED_TRACEBACK_DEPTH 8 /* une_error_display. */
#else
#define UNE_SIZE_NUM_LEN UNE_DEBUG_SIZE
#define UNE_SIZE_STR_LEN UNE_DEBUG_SIZE
#define UNE_SIZE_ID_LEN UNE_DEBUG_SIZE
#define UNE_SIZE_VARIABLE_BUF UNE_DEBUG_SIZE
#define UNE_SIZE_FUNCTION_BUF UNE_DEBUG_SIZE
#define UNE_SIZE_TOKEN_BUF UNE_DEBUG_SIZE
#define UNE_SIZE_SEQUENCE UNE_DEBUG_SIZE
#define UNE_SIZE_FILE_BUFFER UNE_DEBUG_SIZE
#define UNE_SIZE_BIF_SPLIT_TKS UNE_DEBUG_SIZE
#define UNE_SIZE_EXPECTED_TRACEBACK_DEPTH UNE_DEBUG_SIZE
#endif

/* Output Color Escape Sequences. */
#define UNE_COLOR_SUCCESS FGGREEN
#define UNE_COLOR_FAIL BOLD FGRED
#define UNE_COLOR_HINT FGBLACK
#define UNE_COLOR_TOKEN_TYPE FGYELLOW
#define UNE_COLOR_TOKEN_VALUE FGGREEN
#define UNE_COLOR_NODE_BRANCH_TYPE FGCYAN
#define UNE_COLOR_NODE_DATUM_TYPE FGMAGENTA
#define UNE_COLOR_NODE_DATUM_VALUE BOLD FGMAGENTA
#define UNE_COLOR_RESULT_TYPE FGBLUE

/*
*** Types.
*/
typedef int64_t une_int;
typedef double une_flt;

typedef struct _une_position {
  size_t start;
  size_t end;
} une_position;

typedef union _une_value {
  une_int _int;
  une_flt _flt;
  wchar_t *_wcs;
  void **_vpp;
  void *_vp;
} une_value;

/*
*** Portability.
*/
#if defined(__TINYC__)
#define swprintf(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

/*
*** Tools.
*/
#define UNE_VERIFY_NOT_REACHED assert(false)

/* Internal Error Response and Logging Tools. */
#if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
#define LOGFREE(obj, str, num) out(L"%ls ('%ls', %lld)", obj, str, (une_int)num)
#else
#define LOGFREE(...)
#endif
#if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_INTERPRET)
#define LOGINTERPRET(str) out(L"interpret: %ls", str);
#else
#define LOGINTERPRET(...)
#endif
#if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
#define LOGPARSE(object, nt) out(L"%ls [%ls]", object, nt);
#else
#define LOGPARSE(...)
#endif

/*
*** Miscellaneous.
*/
#ifndef UNE_DEBUG
#define __une_static static
#else
#define __une_static
#endif

#endif /* !UNE_PRIMITIVE_H */
