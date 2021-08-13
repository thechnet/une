/*
void.c - Une
Modified 2021-08-09
*/

/* Header-specific includes. */
#include "void.h"

/*
Print a text representation to file.
*/
void une_datatype_void_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_VOID);
  fwprintf(file, L"VOID");
}

/*
Check for truth.
*/
une_int une_datatype_void_is_true(une_result result)
{
  assert(result.type == UNE_RT_VOID);
  return 0;
}
