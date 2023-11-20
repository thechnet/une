/*
primitive.h - Une
Modified 2023-11-20
*/

#ifndef UNE_PRIMITIVE_H
#define UNE_PRIMITIVE_H

/* Universal includes. */
#include "cmake.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <float.h>

#include "util/memdbg.h"

/*
*** Constants.
*/
#define UNE_VERSION UNE_VERSION_MAJOR L"." UNE_VERSION_MINOR L"." UNE_VERSION_PATCH
#define UNE_SOURCE_PLACEHOLDER "<string>"
#define UNE_SWITCH_SCRIPT "-s"
#define UNE_SWITCH_INTERACTIVE "-i"
#define UNE_INTERACTIVE_PREFIX L">>> "
#define UNE_HEADER L"Une " UNE_VERSION L" (" UNE_VERSION_HASH L")"
#define UNE_INTERACTIVE_INFO L"Use \"exit\" or CTRL + C to exit."
#define UNE_FOPEN_RFLAGS "r,ccs=UTF-8"
#define UNE_FOPEN_WFLAGS "w,ccs=UTF-8"
#define UNE_FOPEN_AFLAGS "a,ccs=UTF-8"
#define UNE_DEBUG_SIZES_SIZE 1
#define UNE_DEBUG_REPORT_FILE_RETURN "une_report_return.txt"
#define UNE_DEBUG_REPORT_FILE_STATUS "une_report_status.txt"
#define UNE_PRINTF_UNE_FLT L"%.*Lf"
#define UNE_PRINTF_UNE_INT L"%lld"
#define UNE_ERROR_OUT_OF_MEMORY L"Out of memory."
#define UNE_ERROR_USAGE L"Usage: %hs {<script>|" UNE_SWITCH_SCRIPT L" <string>|" UNE_SWITCH_INTERACTIVE L"}"
#define UNE_ERROR_STREAM stderr
#define UNE_DEBUG_LOGINTERPRET_INDENT L"|   "
#define UNE_DEBUG_LOGINTERPRET_OFFSET 10
#define UNE_FLT_PRECISION (LDBL_DIG-5) /* This is some aggressive rounding, but I think it's still precise enough. */

/* Sizes. */
#define UNE_SIZE_NODE_AS_WCS 128000 /* (Debug) Representing. */
#define UNE_SIZE_TOKEN_AS_WCS 4096 /* (Debug) Representing. */
#define UNE_SIZE_FGETWS_BUFFER 32767 /* une_builtin_input. */
#define UNE_SIZE_NUMBER_AS_STRING (48+UNE_FLT_PRECISION) /* une_int and une_flt as strings. */
#if !defined(UNE_DEBUG_SIZES)
#define UNE_SIZE_NUM_LEN 32 /* Lexing. */
#define UNE_SIZE_STR_LEN 4096 /* Lexing. */
#define UNE_SIZE_NAME_LEN 32 /* Lexing. */
#define UNE_SIZE_VARIABLE_BUF 32 /* Context. */
#define UNE_SIZE_FUNCTION_BUF 16 /* Context. */
#define UNE_SIZE_TOKEN_BUF 4096 /* Lexing. */
#define UNE_SIZE_SEQUENCE 256 /* Parsing. */
#define UNE_SIZE_FILE_BUFFER 4096 /* une_file_read. */
#define UNE_SIZE_BIF_SPLIT_TKS 16 /* une_buitlin_split. */
#define UNE_SIZE_EXPECTED_TRACEBACK_DEPTH 8 /* une_error_display. */
#define UNE_SIZE_HOLDING 4 /* Interpreter state. */
#define UNE_SIZE_CALLABLES 32
#define UNE_SIZE_MODULES 8
#else
#define UNE_SIZE_NUM_LEN UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_STR_LEN UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_NAME_LEN UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_VARIABLE_BUF UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_FUNCTION_BUF UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_TOKEN_BUF UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_SEQUENCE UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_FILE_BUFFER UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_BIF_SPLIT_TKS UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_EXPECTED_TRACEBACK_DEPTH UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_HOLDING UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_CALLABLES UNE_DEBUG_SIZES_SIZE
#define UNE_SIZE_MODULES UNE_DEBUG_SIZES_SIZE
#endif

/* Output Color Escape Sequences. */
#define UNE_COLOR_RESET L"\33[0m"
#define UNE_COLOR_POSITION L"\33[1m\33[32m"
#define UNE_COLOR_FAIL L"\33[1m\33[31m"
#define UNE_COLOR_HINT L"\33[90m"
#define UNE_COLOR_TOKEN_TYPE L"\33[33m"
#define UNE_COLOR_TOKEN_VALUE L"\33[32m"
#define UNE_COLOR_NODE_BRANCH_TYPE L"\33[36m"
#define UNE_COLOR_NODE_DATUM_TYPE L"\33[35m"
#define UNE_COLOR_NODE_DATUM_VALUE L"\33[1m\33[35m"
#define UNE_COLOR_RESULT_TYPE L"\33[34m"
#define UNE_COLOR_TRACEBACK_LOCATION L"\33[1m"
#define UNE_COLOR_WARN L"\33[1m\33[33m"

/*
*** Types.
*/
typedef int64_t une_int;
typedef uint64_t une_uint;
typedef long double une_flt;

typedef struct une_position_ {
	size_t start;
	size_t end;
	size_t line;
} une_position;

typedef union une_value_ {
	size_t _id;
	une_int _int;
	une_flt _flt;
	wchar_t *_wcs;
	void *_vp;
	void **_vpp;
} une_value;

/*
*** Portability.
*/
#if defined(__TINYC__)
#define swprintf(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

/*
*** Logging.
*/
#if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_INTERPRET)
#define LOGINTERPRET_BEGIN(node) \
	static int indent = 0; \
	wprintf(L"%d-%d\33[%dG", node->pos.start, node->pos.end, UNE_DEBUG_LOGINTERPRET_OFFSET); \
	for (int i=0; i<indent; i++) \
		wprintf(UNE_DEBUG_LOGINTERPRET_INDENT); \
	wprintf(L"%ls\n", une_node_type_to_wcs(node->type)); \
	indent++
#define LOGINTERPRET_END(node) \
	indent--; \
	wprintf(L"\33[%dG", UNE_DEBUG_LOGINTERPRET_OFFSET); \
	for (int i=0; i<indent; i++) \
		wprintf(UNE_DEBUG_LOGINTERPRET_INDENT); \
	wprintf(L"X\n", une_node_type_to_wcs(node->type))
#else
#define LOGINTERPRET_BEGIN(...)
#define LOGINTERPRET_END(...)
#endif
#if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_PARSE)
#define LOGPARSE(object_, node_) wprintf(L"P %hs %ls\n", __func__+10, object_, node_.type)
#else
#define LOGPARSE(...)
#endif

/*
*** Miscellaneous.
*/
#ifndef UNE_DEBUG
#define une_static__ static
#else
#define une_static__
#endif
#define UNE_INFINITY ((une_flt)INFINITY)

#endif /* !UNE_PRIMITIVE_H */
