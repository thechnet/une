/*
main.c - Une
Updated 2021-07-05
*/

/* Import public Une interface. */
#include "une.h"
int une_alloc_count = 0;

/* Implementation-specific macros. */
#define UNE_E(msg) L"\33[31m\33[7m" msg L"\n\33[0m"
#define UNE_S_FILE L"No input file."
#define UNE_I_FILE 1
#define UNE_RUN_FILE

#ifndef UNE_RUN_FILE
#include "tools.h"
#endif

int main(int argc, char *argv[])
{
  if (argc < 2) {
    wprintf(UNE_E(UNE_S_FILE));
    return UNE_I_FILE;
  }
  
  #ifdef UNE_RUN_FILE
  une_result result = une_run(true, argv[1], NULL);
  #else
  une_result result = une_run(false, NULL, L"return 1");
  #endif
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
  if (UNE_RESULT_TYPE_IS_DATA_TYPE(result.type)) {
    une_result_represent(result);
    putwc(L'\n', stdout);
  }
  #endif
  int final = result.type == UNE_RT_INT ? (int)result.value._int : 0;
  une_result_free(result);
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_ALLOC_COUNTER)
  if (une_alloc_count != 0)
    wprintf(UNE_COLOR_FAIL L"%d memory location(s) not freed.\33[0m\n", une_alloc_count);
  else
    wprintf(UNE_COLOR_SUCCESS L"All memory locations freed.\33[0m\n");
  #endif
  
  return final;
}
