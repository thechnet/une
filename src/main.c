/*
main.c - Une
Updated 2021-06-09
*/

/* Import public Une interface. */
#include "une.h"
int une_alloc_count = 0;

int main(int argc, char *argv[])
{
  if (argc < 2)
    /* FIXME: Expose macros like ERR to the public? */
    ERR("No input file.");
  
  une_result result = une_run(true, argv[1], NULL);
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
