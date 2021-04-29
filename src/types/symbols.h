/*
symbols.h - Une
Updated 2021-04-29
*/

#ifndef UNE_SYMBOLS_H
#define UNE_SYMBOLS_H

#include "../primitive.h"
#include "../tools.h"
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

#endif /* !UNE_SYMBOLS_H */
