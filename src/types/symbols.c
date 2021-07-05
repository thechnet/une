/*
symbols.c - Une
Modified 2021-07-05
*/

/* Header-specific includes. */
#include "symbols.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Free all members of a une_variable.
*/
void une_variable_free(une_variable variable)
{
  LOGFREE(L"une_variable", variable.name, 0);
  une_free(variable.name);
  une_result_free(variable.content);
}

/*
Free all members of a une_function.
*/
void une_function_free(une_function function)
{
  LOGFREE(L"une_function", function.name, 0);
  une_free(function.name);
  for (size_t i=0; i<function.params_count; i++)
    une_free(function.params[i]);
  une_free(function.params);
  une_node_free(function.body, true);
}
