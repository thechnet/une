/*
symbols.c - Une
Updated 2021-04-29
*/

#include "symbols.h"

#pragma region une_variable_free
void une_variable_free(une_variable variable)
{
  free(variable.name);
  une_result_free(variable.content);
  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Variable: %ls\n", __FILE__, __FUNCTION__, __LINE__, variable.name);
  #endif
}
#pragma endregion une_variable_free

#pragma region une_function_free
void une_function_free(une_function function)
{
  free(function.name);
  for(size_t i=0; i<function.params_count; i++) free(function.params[i]);
  free(function.params);
  une_node_free(function.body, true);
  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Function: %ls\n", __FILE__, __FUNCTION__, __LINE__, function.name);
  #endif
}
#pragma endregion une_function_free
