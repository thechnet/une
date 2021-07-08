/*
main.c - Une
Modified 2021-07-08
*/

/* Import public Une interface. */
#include "une.h"

/* Implementation-specific includes. */
#include <string.h>
#include "tools.h"

/* Implementation-specific macros. */
#define UNE_E(msg) L"\33[31m\33[7m" msg L"\n\33[0m"
#define UNE_S_INPUT L"Missing input file or string."
#define UNE_C_INPUT 1

int main(int argc, char *argv[])
{
  if (argc < 2) {
    wprintf(UNE_E(UNE_S_INPUT));
    return UNE_C_INPUT;
  }
  
  bool read_from_file = true;
  
  if (strcmp(argv[1],UNE_STDIN_SWITCH) == 0) {
    if (argc < 3) {
      wprintf(UNE_E(UNE_S_INPUT));
      return UNE_C_INPUT;
    }
    read_from_file = false;
  }
  
  une_result result;
  
  if (read_from_file)
    result = une_run(read_from_file, argv[1], NULL);
  else {
    wchar_t *wcs = une_str_to_wcs(argv[2]);
    result = une_run(read_from_file, NULL, wcs);
    free(wcs);
  }
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
  if (UNE_RESULT_TYPE_IS_DATA_TYPE(result.type)) {
    putwc(L'\n', stdout);
    une_result_represent(result);
    putwc(L'\n', stdout);
  }
  #endif
  int final = result.type == UNE_RT_INT ? (int)result.value._int : 0;
  une_result_free(result);
  
  return final;
}
