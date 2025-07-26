/*
common.h - Une
Modified 2025-07-26
*/

#ifndef UNE_PRIMITIVE_H
#define UNE_PRIMITIVE_H

/* Ensure UNE_USES_UCRT is defined if we're using the UCRT. */
#if defined(_UCRT) && !defined(UNE_USES_UCRT)
#error Enable UNE_USES_UCRT when using the UCRT.
#endif

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

#include "deprecated/memdbg.h"

/*
*** Constants.
*/
#define UNE_VERSION UNE_VERSION_MAJOR L"." UNE_VERSION_MINOR L"." UNE_VERSION_PATCH
#define UNE_MODULE_NAME_PLACEHOLDER "<string>"
#define UNE_SWITCH_SCRIPT "-s"
#define UNE_SWITCH_INTERACTIVE "-i"
#define UNE_INTERACTIVE_PREFIX L">>> "
#define UNE_HEADER L"Une " UNE_VERSION L" (" UNE_VERSION_HASH L")"
#define UNE_INTERACTIVE_INFO L"Use \"exit\" or CTRL + C to exit."
#define UNE_FOPEN_RFLAGS "r,ccs=UTF-8"
#define UNE_FOPEN_WFLAGS "w,ccs=UTF-8"
#define UNE_FOPEN_AFLAGS "a,ccs=UTF-8"
#define UNE_DBG_SIZES_SIZE 1
#define UNE_DBG_REPORT_FILE_RETURN "une_report_return.txt"
#define UNE_DBG_REPORT_FILE_STATUS "une_report_status.txt"
#define UNE_PRINTF_UNE_FLT L"%.*lf"
#define UNE_PRINTF_UNE_INT L"%lld"
#define UNE_ERROR_OUT_OF_MEMORY L"Out of memory."
#define UNE_ERROR_USAGE L"Usage: %hs {<script>|" UNE_SWITCH_SCRIPT L" <string>|" UNE_SWITCH_INTERACTIVE L"}"
#define UNE_ERROR_STREAM stderr
#define UNE_DBG_LOGINTERPRET_INDENT L"|   "
#define UNE_DBG_LOGINTERPRET_OFFSET 10
#define UNE_DBG_LOGPARSE_INDENT L"| "
#define UNE_DBG_LOGPARSE_OFFSET 15
#define UNE_FLT_PRECISION 10 /* See https://www.gnu.org/software/libc/manual/html_node/Floating-Point-Parameters.html#index-DBL_005fDIG. Also, test.py! */
#define UNE_TAB_WIDTH 4
#define UNE_TRACEBACK_EXTRACT_PREFIX L"  "

/* Sizes. */
#define UNE_SIZE_NODE_AS_WCS 128000 /* (Debug) Representing. */
#define UNE_SIZE_TOKEN_AS_WCS 4096 /* (Debug) Representing. */
#define UNE_SIZE_FGETWS_BUFFER 32767 /* une_native_fn_input. */
#define UNE_SIZE_NUMBER_AS_STRING (48+UNE_FLT_PRECISION) /* une_int and une_flt as strings. */
#if !defined(UNE_DEBUG) || !defined(UNE_DBG_SIZES)
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
#define UNE_SIZE_NUM_LEN UNE_DBG_SIZES_SIZE
#define UNE_SIZE_STR_LEN UNE_DBG_SIZES_SIZE
#define UNE_SIZE_NAME_LEN UNE_DBG_SIZES_SIZE
#define UNE_SIZE_VARIABLE_BUF UNE_DBG_SIZES_SIZE
#define UNE_SIZE_FUNCTION_BUF UNE_DBG_SIZES_SIZE
#define UNE_SIZE_TOKEN_BUF UNE_DBG_SIZES_SIZE
#define UNE_SIZE_SEQUENCE UNE_DBG_SIZES_SIZE
#define UNE_SIZE_FILE_BUFFER UNE_DBG_SIZES_SIZE
#define UNE_SIZE_BIF_SPLIT_TKS UNE_DBG_SIZES_SIZE
#define UNE_SIZE_EXPECTED_TRACEBACK_DEPTH UNE_DBG_SIZES_SIZE
#define UNE_SIZE_HOLDING UNE_DBG_SIZES_SIZE
#define UNE_SIZE_CALLABLES UNE_DBG_SIZES_SIZE
#define UNE_SIZE_MODULES UNE_DBG_SIZES_SIZE
#endif

/* Output Color Escape Sequences. */
#define UNE_COLOR_RESET L"\33[0m"
#define UNE_COLOR_POSITION L"\33[1m\33[32m"
#define UNE_COLOR_FAIL L"\33[1m\33[31m"
#define UNE_COLOR_HINT L"\33[90m"
#define UNE_COLOR_TOKEN_KIND L"\33[33m"
#define UNE_COLOR_TOKEN_VALUE L"\33[32m"
#define UNE_COLOR_NODE_BRANCH_KIND L"\33[36m"
#define UNE_COLOR_NODE_DATUM_KIND L"\33[35m"
#define UNE_COLOR_NODE_DATUM_VALUE L"\33[1m\33[35m"
#define UNE_COLOR_RESULT_KIND L"\33[34m"
#define UNE_COLOR_TRACEBACK_LOCATION L"\33[1m"
#define UNE_COLOR_WARN L"\33[1m\33[33m"

/*
*** Types.
*/
typedef int64_t une_int;
typedef uint64_t une_uint;
typedef double une_flt;

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
#ifdef __TINYC__
#define swprintf(dest, size, format, ...) swprintf((dest), (format), ##__VA_ARGS__)
#endif

/*
*** Logging.
*/
#if defined(UNE_DEBUG) && defined(UNE_DBG_LOG_INTERPRET)
#define LOGINTERPRET_BEGIN(node) \
	static int indent = 0; \
	wprintf(L"%d-%d\33[%dG", node->pos.start, node->pos.end, UNE_DBG_LOGINTERPRET_OFFSET); \
	for (int i=0; i<indent; i++) \
		wprintf(UNE_DBG_LOGINTERPRET_INDENT); \
	wprintf(L"%ls\n", une_node_kind_to_wcs(node->kind)); \
	indent++
#define LOGINTERPRET_END(node) \
	indent--; \
	wprintf(L"\33[%dG", UNE_DBG_LOGINTERPRET_OFFSET); \
	for (int i=0; i<indent; i++) \
		wprintf(UNE_DBG_LOGINTERPRET_INDENT); \
	wprintf(L"/\n", une_node_kind_to_wcs(node->kind))
#else
#define LOGINTERPRET_BEGIN(...)
#define LOGINTERPRET_END(...)
#endif
#if defined(UNE_DEBUG) && defined(UNE_DBG_LOG_PARSE)
#define LOGPARSE_BEGIN() \
	do { \
		wchar_t *tk_ = une_token_to_wcs(now(&ps->in)); \
		wprintf(L"%ls\33[%dG", tk_, UNE_DBG_LOGPARSE_OFFSET); \
		free(tk_); \
		for (int i=0; i<une_logparse_indent; i++) \
			wprintf(UNE_DBG_LOGPARSE_INDENT); \
		une_logparse_indent++; \
		wprintf(L"%hs\n", __func__ + 10); \
	} while (0)
#define LOGPARSE_END(call) \
	do { \
		une_node *result__ = call; \
		wchar_t *tk_ = une_token_to_wcs(now(&ps->in)); \
		wprintf(L"%ls\33[%dG\33[%dm", tk_, UNE_DBG_LOGPARSE_OFFSET, result__ ? 90 : 31); \
		free(tk_); \
		une_logparse_indent--; \
		for (int i=0; i<une_logparse_indent; i++) \
			wprintf(UNE_DBG_LOGPARSE_INDENT); \
		wprintf(L"\33[9m%hs\33[0m\n", __func__ + 10); \
		return result__; \
	} while (0)
#define LOGPARSE_COMMENT(comment) \
	do { \
		wchar_t *tk_ = une_token_to_wcs(now(&ps->in)); \
		wprintf(L"%ls\33[%dG", tk_, UNE_DBG_LOGPARSE_OFFSET); \
		free(tk_); \
		for (int i=0; i<une_logparse_indent; i++) \
			wprintf(UNE_DBG_LOGPARSE_INDENT); \
		wprintf(L"(%ls)\n", comment); \
	} while (0)
#else
#define LOGPARSE_BEGIN()
#define LOGPARSE_END(call) return call
#define LOGPARSE_COMMENT(...)
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
