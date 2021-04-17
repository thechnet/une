/*
symbols.c - Une
Updated 2021-04-17
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
  une_node_free(function.params);
  une_node_free(function.body);
  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(L"Function: %ls\n", function.name);
  #endif
}
#pragma endregion une_function_free
