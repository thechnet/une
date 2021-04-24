/*
symbols.c - Une
Updated 2021-04-24
*/

#include "symbols.h"

#pragma region une_variable_free
void une_variable_free(une_variable variable)
{
  free(variable.name);
  une_result_free(variable.content);
  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(L"Variable: %ls\n", variable.name);
  #endif
}
#pragma endregion une_variable_free

#pragma region une_function_free
void une_function_free(une_function function)
{
  free(function.name);
  for(size_t i=0; i<function.params_count; i++) free(function.params[i]);
  free(function.params);
  une_node_free(function.body);
  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(L"Function: %ls\n", function.name);
  #endif
}
#pragma endregion une_function_free

#pragma region une_variable_find
une_variable *une_variable_find(une_variable *variables, size_t variables_count, wchar_t *name)
{
  for (size_t i=0; i<variables_count; i++) {
    if (wcscmp(variables[i].name, name) == 0) return &variables[i];
  }
  return NULL;
}
#pragma endregion une_variable_find

#pragma region une_function_find
une_function *une_function_find(une_function *functions, size_t functions_count, wchar_t *name)
{
  for (size_t i=0; i<functions_count; i++) {
    if (wcscmp(functions[i].name, name) == 0) return &functions[i];
  }
  return NULL;
}
#pragma endregion une_function_find

#pragma region une_variable_create
une_variable *une_variable_create(
  une_variable **variables,
  size_t *variables_count,
  size_t *variables_size,
  wchar_t *name
)
{
  if (*variables_count >= *variables_size) {
    *variables_size *= 2;
    une_variable *_variables = realloc(*variables, *variables_size*sizeof(*_variables));
    if (_variables == NULL) WERR(L"Out of memory.");
    *variables = _variables;
  }
  une_variable *var = &((*variables)[*variables_count]);
  var->name = wcs_dup(name);
  var->content.type = UNE_RT_INT;
  var->content.value._int = 0;
  (*variables_count)++;
  return var;
}
#pragma endregion une_variable_create
