/*
main.c - Une
Modified 2023-11-19
*/

/* Header-specific includes. */
#include "main.h"

/* Implementation-specific includes. */
#include <signal.h>
#include "tools.h"
#include "types/engine.h"
#include "datatypes/datatypes.h"

/*
Implementation.
*/

volatile sig_atomic_t sigint_fired = 0;

int main(int argc, char *argv[])
{
	/* Enable Virtual Terminal Processing for the Windows console. */
	#ifdef _WIN32
	une_win_vt_proc(true);
	#endif
	
	/* Display warnings. */
	#ifdef UNE_DEBUG
	#ifdef UNE_DEBUG_MEMDBG
	wprintf(UNE_COLOR_WARN L"UNE_DEBUG_MEMDBG enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_DEBUG_SIZES
	wprintf(UNE_COLOR_WARN L"UNE_DEBUG_SIZES enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_DEBUG_REPORT
	wprintf(UNE_COLOR_WARN L"UNE_DEBUG_REPORT enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_NO_LEX
	wprintf(UNE_COLOR_WARN L"UNE_NO_LEX enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_NO_PARSE
	wprintf(UNE_COLOR_WARN L"UNE_NO_PARSE enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_NO_INTERPRET
	wprintf(UNE_COLOR_WARN L"UNE_NO_INTERPRET enabled.\n" UNE_COLOR_RESET);
	#endif
	#endif

	/* Check command line. */
	wchar_t *script_string = NULL;
	enum une_main_action action = SHOW_USAGE;
	if (argc == 2 && !strcmp(argv[1], UNE_SWITCH_INTERACTIVE)) {
		action = ENTER_INTERACTIVE_MODE;
	} else if (argc >= 2 && !strcmp(argv[1], UNE_SWITCH_SCRIPT)) {
		script_string = argc == 3 ? une_str_to_wcs(argv[2]) : NULL;
		if (script_string)
			action = RUN_SCRIPT_STRING;
		else
			action = SHOW_USAGE;
	} else if (argc == 2) {
		action = RUN_SCRIPT_FILE;
	}
	
	/* Perform action. */

	une_result result;

	if (action == SHOW_USAGE) {
		result = (une_result){
			.type = UNE_RT_ERROR,
			.value._int = EXIT_FAILURE
		};
		print_usage(argv[0]);
	} else {
		une_engine engine = une_engine_create_engine();
		une_engine_select_engine(&engine);
		
		if (action == RUN_SCRIPT_FILE) {
			result = une_engine_interpret_file_or_wcs(argv[1], NULL);
		} else if (action == RUN_SCRIPT_STRING) {
			result = une_engine_interpret_file_or_wcs(NULL, script_string);
			free(script_string);
		} else {
			assert(action == ENTER_INTERACTIVE_MODE);
			result = (une_result){
				.type = UNE_RT_INT,
				.value._int = EXIT_SUCCESS
			};
			interactive();
		}

		if (result.type == UNE_RT_ERROR) {
			result.value._int = (une_int)felix->error.type+(une_int)UNE_R_END_DATA_RESULT_TYPES;
			une_engine_print_error();
		}
		
		une_engine_free();
	}

	#if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
	if (result.type != UNE_RT_ERROR) {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(result.type));
		wprintf(UNE_COLOR_RESULT_TYPE L"%ls" UNE_COLOR_RESET ": ", une_result_type_to_wcs(result.type));
		une_result_represent(stdout, result);
		putwc(L'\n', stdout);
	}
	#endif
	
	#if defined(UNE_DEBUG) && defined(UNE_DEBUG_REPORT)
	FILE *report_return = fopen(UNE_DEBUG_REPORT_FILE_RETURN, UNE_FOPEN_WFLAGS);
	assert(report_return != NULL);
	if (UNE_RESULT_TYPE_IS_DATA_TYPE(result.type))
		une_result_represent(report_return, result);
	else if (result.type == UNE_RT_ERROR)
		fwprintf(report_return, UNE_PRINTF_UNE_INT, result.value._int);
	fclose(report_return);
	#endif /* UNE_DEBUG_REPORT */
	
	int final;
	if (result.type == UNE_RT_INT)
		final = (int)result.value._int;
	else if (result.type == UNE_RT_ERROR)
		final = EXIT_FAILURE;
	else
		final = EXIT_SUCCESS;
	une_result_free(result);
	
	#if defined(UNE_DEBUG) && defined(UNE_DEBUG_REPORT)
	#ifdef UNE_DEBUG_MEMDBG
	extern int64_t memdbg_allocations_count;
	extern int64_t memdbg_alert_count;
	#endif /* UNE_DEBUG_MEMDBG */
	FILE *report_status = fopen(UNE_DEBUG_REPORT_FILE_STATUS, UNE_FOPEN_WFLAGS);
	assert(report_status != NULL);
	fwprintf(
		report_status,
		L"result_type:%d\n"
		#ifdef UNE_DEBUG_MEMDBG
		L"alloc_count:%d\n"
		L"alert_count:%d\n"
		#endif /* UNE_DEBUG_MEMDBG */
		, (int)result.type
		#ifdef UNE_DEBUG_MEMDBG
		, memdbg_allocations_count-1 /* FILE *report_status */,
		memdbg_alert_count
		#endif /* UNE_DEBUG_MEMDBG */
	);
	fclose(report_status);
	#endif /* UNE_DEBUG_REPORT */

	/* Disable Virtual Terminal Processing for the Windows console. */
	#ifdef _WIN32
	une_win_vt_proc(false);
	#endif
	
	return final;
}

void interactive_sigint_handler(int signal)
{
	sigint_fired = true;
}

void interactive(void)
{
	signal(SIGINT, &interactive_sigint_handler);

	wchar_t *stmts = malloc(UNE_SIZE_FGETWS_BUFFER*sizeof(*stmts));
	verify(stmts);

	fputws(UNE_COLOR_RESET UNE_HEADER L"\n" UNE_INTERACTIVE_INFO L"\n", stdout);
	
	while (!sigint_fired && !felix->is.should_exit) {
		fputws(UNE_INTERACTIVE_PREFIX, stdout);
		
		if (!fgetws(stmts, UNE_SIZE_FGETWS_BUFFER, stdin))
			break;
		
		/* Directives. */
		if (!wcscmp(stmts, L"#clear\n")) {
			fputws(L"\033[H\033[J", stdout);
			continue;
		} else if (!wcscmp(stmts, L"#header\n")) {
			fputws(UNE_COLOR_RESET UNE_HEADER L"\n" UNE_INTERACTIVE_INFO L"\n", stdout);
			continue;
		} else if (!wcscmp(stmts, L"#symbols\n")) {
			for (size_t i=0; i<felix->is.context->variables_count; i++) {
				fputws(felix->is.context->variables[i]->name, stdout);
				if (felix->is.context->variables[i]->content.type == UNE_RT_FUNCTION) {
					une_callable *callable = une_callables_get_callable_by_id(felix->is.callables, felix->is.context->variables[i]->content.value._id);
					fputwc(L'(', stdout);
					if (callable->params_count) {
						fwprintf(stdout, L"%ls", callable->params[0]);
						for (size_t j=1; j<callable->params_count; j++)
							fwprintf(stdout, L", %ls", callable->params[j]);
					}
					fwprintf(stdout, L")", callable->id);
				}
				fputwc(L'\n', stdout);
			}
			continue;
		}

		size_t len = wcslen(stmts);
		stmts[--len] = L'\0'; /* Remove trailing newline. */
		une_result result = une_engine_interpret_file_or_wcs(NULL, stmts);
		if (result.type != UNE_RT_VOID && result.type != UNE_RT_ERROR) {
			if (UNE_RESULT_TYPE_IS_DATA_TYPE(result.type)) {
				une_result_represent(stdout, result);
			} else {
				assert(UNE_DATATYPE_FOR_RESULT(result).represent != NULL);
				UNE_DATATYPE_FOR_RESULT(result).represent(stdout, result);
			}
			fputwc(L'\n', stdout);
		}
		une_result_free(result);
	}

	free(stmts);
}

void print_usage(char *executable_path)
{
	wprintf(UNE_COLOR_RESET UNE_HEADER L"\n" UNE_ERROR_USAGE L"\n", executable_path);
}
