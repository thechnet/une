/*
main.c - Une
Updated 2021-06-04
*/

#include "une.h"

#include "tools.h"

#if defined(UNE_DEBUG) && defined(UNE_DEBUG_MALLOC_COUNTER)
  int malloc_counter = 0;
#endif

int main(int argc, char *argv[])
{
  #if defined(UNE_DEBUG) && defined(UNE_DRAW_BAR)
    // Prints a bar to distinguish new output from old output in the terminal.
    wprintf(UNE_COLOR_SUCCESS L"\33[7m\33[K\33[27m\33[0m\n");
  #endif
  
  if (argc < 2) WERR("No input file.");
  
  une_result result = une_run(true, argv[1], NULL);
  int final = result.type == UNE_RT_INT ? result.value._int : 0;
  une_result_free(result);
  
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_MEMORY_REPORT)
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_MALLOC_COUNTER )
      if (malloc_counter != 0) {
        wprintf(UNE_COLOR_FAIL L"%d memory location(s) not freed.\33[0m", malloc_counter);
      } else {
        wprintf(UNE_COLOR_SUCCESS L"All memory locations freed.\33[0m");
      }
    #else
      wprintf(L"\n\33[97mEnd of Program.\33[0m");
    #endif
    wprintf(L"\n");
  #endif
  
  return final;
}
