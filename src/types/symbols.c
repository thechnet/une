/*
symbols.c - Une
Modified 2021-08-13
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
  free(variable.name);
  une_result_free(variable.content);
}

/*
Free all members of a une_function.
*/
void une_function_free(une_function *function)
{
  free(function->definition_file);
  for (size_t i=0; i<function->params_count; i++)
    free(function->params[i]);
  if (function->params != NULL)
    free(function->params);
  une_node_free(function->body, true);
  free(function);
}
