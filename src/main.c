/*
main.c - Une
Modified 2021-08-14
*/

/* Import public Une interface. */
#include "une.h"

/* Implementation-specific includes. */
#include <string.h>
#include "tools.h"

#define UNE_MAIN_ERROR 1

int main(int argc, char *argv[])
{
  /* Warnings */
  #ifdef UNE_DEBUG_MEMDBG
  warn("UNE_DEBUG_MEMDBG enabled.");
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
  
  une_result result;
  bool read_from_file;
  bool main_error = false;
  
  if (argc < 2) {
    main_error = true;
    goto end;
  }
  
  if (strcmp(argv[1], UNE_STDIN_SWITCH) == 0) {
    if (argc < 3) {
      main_error = true;
      goto end;
    }
    read_from_file = false;
  } else
    read_from_file = true;
  
  if (read_from_file)
    result = une_run(read_from_file, argv[1], NULL);
  else {
    wchar_t *wcs = une_str_to_wcs(argv[2]);
    if (wcs == NULL) {
      main_error = true;
      goto end;
    }
    result = une_run(read_from_file, NULL, wcs);
    free(wcs);
  }
  
  end:
  /* main Error. */
  if (main_error) {
    wprintf(RESET L"Usage: %hs {<script>|-s <string>})\n", argv[0]);
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
  
  #ifdef UNE_DEBUG_REPORT
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
  
  #ifdef UNE_DEBUG_REPORT
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
  
  return final;
}
