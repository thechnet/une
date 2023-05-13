/*
main.c - Une
Modified 2023-05-13
*/

/* Implementation-specific includes. */
#include <string.h>
#include <signal.h>
#include "une.h"
#include "tools.h"
#include "datatypes/datatypes.h"

#define UNE_MAIN_ERROR 1

void main_interactive(void);

int main(int argc, char *argv[])
{
  #ifdef _WIN32
  /* Manually enable Virtual Terminal Processing for the Windows console. */
  une_win_vt_proc(true);
  #endif
  
  /* Warnings */
  #ifdef UNE_DEBUG
  #ifdef UNE_DEBUG_MEMDBG
  warn("UNE_DEBUG_MEMDBG enabled.");
  #endif
  #ifdef UNE_DEBUG_WATCHDOG
  warn("UNE_DEBUG_WATCHDOG enabled.");
  #endif
  #ifdef UNE_DEBUG_SIZES
  warn("UNE_DEBUG_SIZES enabled.");
  #endif
  #ifdef UNE_DEBUG_REPORT
  warn("UNE_DEBUG_REPORT enabled.");
  #endif
  #ifdef UNE_NO_LEX
  warn("UNE_NO_LEX enabled.");
  #endif
  #ifdef UNE_NO_PARSE
  warn("UNE_NO_PARSE enabled.");
  #endif
  #ifdef UNE_NO_INTERPRET
  warn("UNE_NO_INTERPRET enabled.");
  #endif
  #endif
  une_result result;
  bool read_from_file;
  bool main_error = false;
  
  if (argc < 2) {
    main_error = true;
    goto end;
  }
  
  if (strcmp(argv[1], UNE_SWITCH_INTERACTIVE) == 0) {
    main_interactive();
    return 0;
  }
  
  if (strcmp(argv[1], UNE_SWITCH_SCRIPT) == 0) {
    if (argc < 3) {
      main_error = true;
      goto end;
    }
    read_from_file = false;
  } else
    read_from_file = true;
  
  if (read_from_file)
    result = une_run(read_from_file, argv[1], NULL, NULL, NULL);
  else {
    wchar_t *wcs = une_str_to_wcs(argv[2]);
    if (wcs == NULL) {
      main_error = true;
      goto end;
    }
    result = une_run(read_from_file, NULL, wcs, NULL, NULL);
    free(wcs);
  }
  
  end:
  /* main Error. */
  if (main_error) {
    wprintf(RESET UNE_HEADER L"\n" UNE_ERROR_USAGE L"\n", argv[0]);
    result = (une_result){
      .type = UNE_RT_ERROR,
      .value._int = UNE_MAIN_ERROR
    };
  }
  
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
  if (result.type != UNE_RT_ERROR) {
    assert(UNE_RESULT_TYPE_IS_DATA_TYPE(result.type));
    wprintf(UNE_COLOR_RESULT_TYPE L"%ls" RESET ": ", une_result_type_to_wcs(result.type));
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
    final = UNE_MAIN_ERROR;
  else
    final = 0;
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
  
  #ifdef _WIN32
  /* Manually disable Virtual Terminal Processing for the Windows console. */
  une_win_vt_proc(false);
  #endif
  
  return final;
}

/*
*** Interactive mode.
*/

volatile sig_atomic_t sigint_fired = false;

void main_sigint_fired(int signal)
{
  sigint_fired = true;
}

void main_interactive(void)
{
  signal(SIGINT, &main_sigint_fired);
  une_interpreter_state is = une_interpreter_state_create();
  wchar_t *stmts = malloc(UNE_SIZE_FGETWS_BUFFER*sizeof(*stmts));
  verify(stmts);
  bool did_exit = false;
  
  fputws(RESET UNE_HEADER L"\n" UNE_INTERACTIVE_INFO L"\n", stdout);
  
  while (!sigint_fired && !did_exit) {
    fputws(UNE_INTERACTIVE_PREFIX, stdout);
    if (!fgetws(stmts, UNE_SIZE_FGETWS_BUFFER, stdin))
      break;
    /* Directives. */
    if (!wcscmp(stmts, L"#clear\n")) {
      fputws(L"\033[H\033[J", stdout);
      continue;
    } else if (!wcscmp(stmts, L"#header\n")) {
      fputws(RESET UNE_HEADER L"\n" UNE_INTERACTIVE_INFO L"\n", stdout);
      continue;
    } else if (!wcscmp(stmts, L"#symbols\n")) {
      for (size_t i=0; i<is.context->variables_count; i++) {
        fputws(is.context->variables[i]->name, stdout);
        if (is.context->variables[i]->content.type == UNE_RT_FUNCTION) {
          une_function *fn = (une_function*)is.context->variables[i]->content.value._vp;
          fputwc(L'(', stdout);
          if (fn->params_count) {
            fwprintf(stdout, L"%ls", fn->params[0]);
            for (size_t j=1; j<fn->params_count; j++)
              fwprintf(stdout, L", %ls", fn->params[j]);
          }
          fputwc(L')', stdout);
        }
        fputwc(L'\n', stdout);
      }
      continue;
    }
    size_t len = wcslen(stmts);
    stmts[--len] = L'\0'; /* Remove trailing newline. */
    une_result result = une_run(false, NULL, stmts, &did_exit, une_is);
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
  une_interpreter_state_free(&is);
}
