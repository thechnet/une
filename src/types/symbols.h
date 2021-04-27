/*
symbols.h - Une
Updated 2021-04-24
*/

#ifndef UNE_SYMBOLS_H
#define UNE_SYMBOLS_H

#include "../primitive.h"
#include "result.h"
#include "node.h"

#pragma region une_variable
typedef struct _une_variable {
  wchar_t *name; /* DOC: Making all these items nodes allows us to know where
                     variables were last changed */
  une_result content;
} une_variable;
#pragma endregion une_variable

#pragma region une_function
typedef struct _une_function {
  wchar_t *name; /* DOC: Making all these items nodes allows us to know where
                    functions were defined. */
  size_t params_count;
  wchar_t **params;
  une_node *body;
} une_function;
#pragma endregion une_function

void une_variable_free(une_variable variable);
void une_function_free(une_function function);
une_variable *une_variable_find(une_variable *variables, size_t variables_count, wchar_t *name);
une_function *une_function_find(une_function *functions, size_t functions_count, wchar_t *name);
une_variable *une_variable_create(
  une_variable **variables,
  size_t *variables_count,
  size_t *variables_size,
  wchar_t *name
);
une_function *une_function_create(
  une_function **functions,
  size_t *functions_count,
  size_t *functions_size,
  wchar_t *name
);

#endif /* !UNE_SYMBOLS_H */
