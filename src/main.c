/*
main.c - Une
Modified 2023-12-10
*/

/* Header-specific includes. */
#include "main.h"

/* Implementation-specific includes. */
#include "tools.h"
#include "struct/engine.h"
#include "types/types.h"

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
	#ifdef UNE_DBG_MEMDBG
	wprintf(UNE_COLOR_WARN L"UNE_DBG_MEMDBG enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_DBG_SIZES
	wprintf(UNE_COLOR_WARN L"UNE_DBG_SIZES enabled.\n" UNE_COLOR_RESET);
	#endif
	#ifdef UNE_DBG_REPORT
	wprintf(UNE_COLOR_WARN L"UNE_DBG_REPORT enabled.\n" UNE_COLOR_RESET);
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
			.kind = UNE_RK_ERROR,
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
				.kind = UNE_RK_INT,
				.value._int = EXIT_SUCCESS
			};
			interactive();
		}

		if (result.kind == UNE_RK_ERROR) {
			result.value._int = (une_int)felix->error.kind+(une_int)UNE_R_END_DATA_RESULT_KINDS;
			une_engine_print_error();
		}
		
		une_engine_free();
	}

	#if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
	if (result.kind != UNE_RK_ERROR) {
		assert(UNE_RESULT_KIND_IS_TYPE(result.kind));
		wprintf(UNE_COLOR_RESULT_KIND L"%ls" UNE_COLOR_RESET ": ", une_result_kind_to_wcs(result.kind));
		une_result_represent(stdout, result);
		putwc(L'\n', stdout);
	}
	#endif
	
	#if defined(UNE_DEBUG) && defined(UNE_DBG_REPORT)
	FILE *report_return = fopen(UNE_DBG_REPORT_FILE_RETURN, UNE_FOPEN_WFLAGS);
	assert(report_return != NULL);
	if (UNE_RESULT_KIND_IS_TYPE(result.kind))
		une_result_represent(report_return, result);
	else if (result.kind == UNE_RK_ERROR)
		fwprintf(report_return, UNE_PRINTF_UNE_INT, result.value._int);
	fclose(report_return);
	#endif /* UNE_DBG_REPORT */
	
	int final;
	if (result.kind == UNE_RK_INT)
		final = (int)result.value._int;
	else if (result.kind == UNE_RK_ERROR)
		final = EXIT_FAILURE;
	else
		final = EXIT_SUCCESS;
	une_result_free(result);
	
	#if defined(UNE_DEBUG) && defined(UNE_DBG_REPORT)
	#ifdef UNE_DBG_MEMDBG
	extern int64_t memdbg_allocations_count;
	extern int64_t memdbg_alert_count;
	#endif /* UNE_DBG_MEMDBG */
	FILE *report_status = fopen(UNE_DBG_REPORT_FILE_STATUS, UNE_FOPEN_WFLAGS);
	assert(report_status != NULL);
	fwprintf(
		report_status,
		L"result_kind:%d\n"
		#ifdef UNE_DBG_MEMDBG
		L"alloc_count:%d\n"
		L"alert_count:%d\n"
		#endif /* UNE_DBG_MEMDBG */
		, (int)result.kind
		#ifdef UNE_DBG_MEMDBG
		, memdbg_allocations_count-1 /* FILE *report_status */,
		memdbg_alert_count
		#endif /* UNE_DBG_MEMDBG */
	);
	fclose(report_status);
	#endif /* UNE_DBG_REPORT */

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
			fputws(UNE_COLOR_RESET L"\033[H\033[J" UNE_HEADER L"\n" UNE_INTERACTIVE_INFO L"\n", stdout);
			continue;
		} else if (!wcscmp(stmts, L"#inspect\n")) {
			/* Modules. */
			fputws(L"--- modules ---\n", stdout);
			for (size_t i=0; i<felix->is.modules.size; i++) {
				une_module module = felix->is.modules.buffer[i];
				if (module.id == 0)
					continue;
				fwprintf(stdout, L"m%zu ", module.id);
				if (module.originates_from_file)
					fwprintf(stdout, L"'%hs'", module.path);
				else
					fputws(UNE_MODULE_NAME_PLACEHOLDER L"", stdout);
				size_t end_of_first_line = une_wcs_find_end_of_current_line(module.source, 0);
				fwprintf(stdout, L" | %.*ls\n", end_of_first_line, module.source);
			}

			/* Callables. */
			fputws(L"--- callables ---\n", stdout);
			for (size_t i=0; i<felix->is.callables.size; i++) {
				une_callable callable = felix->is.callables.buffer[i];
				if (callable.id == 0)
					continue;
				fwprintf(stdout, L"c%zu in m%zu, %zu/%zu-%zu, with (", callable.id, callable.module_id, callable.position.line, callable.position.start, callable.position.end);
				if (callable.parameters.count) {
					fputws(callable.parameters.names[0], stdout);
					for (size_t j=1; j<callable.parameters.count; j++) {
						fwprintf(stdout, L", %ls", callable.parameters.names[j]);
					}
				}
				fputws(L")\n", stdout);
			}

			/* Symbols. */
			fputws(L"--- symbols ---\n", stdout);
			for (size_t i=0; i<felix->is.context->variables.count; i++) {
				fputws(felix->is.context->variables.buffer[i]->name, stdout);
				if (felix->is.context->variables.buffer[i]->content.kind == UNE_RK_FUNCTION) {
					une_callable *callable = une_callables_get_callable_by_id(felix->is.callables, felix->is.context->variables.buffer[i]->content.value._id);
					assert(callable);
					fwprintf(stdout, L" -> c%zu", callable->id);
				}
				fputwc(L'\n', stdout);
			}
			continue;
		}

		size_t len = wcslen(stmts);
		stmts[--len] = L'\0'; /* Remove trailing newline. */
		une_result result = une_engine_interpret_file_or_wcs(NULL, stmts);
		if (result.kind == UNE_RK_ERROR) {
			une_engine_print_error();
			une_engine_return_to_root_context();
		} else if (result.kind != UNE_RK_VOID) {
			if (UNE_RESULT_KIND_IS_TYPE(result.kind)) {
				une_result_represent(stdout, result);
			} else {
				assert(UNE_TYPE_FOR_RESULT(result).represent != NULL);
				UNE_TYPE_FOR_RESULT(result).represent(stdout, result);
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
