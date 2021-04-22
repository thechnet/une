/*
symbols.h - Une
Updated 2021-04-17
*/

#ifndef UNE_SYMBOLS_H
#define UNE_SYMBOLS_H

#include "../primitive.h"
#include "result.h"
#include "node.h"

#pragma region une_variable
typedef struct _une_variable {
  wchar_t *name;
  une_result content;
} une_variable;
#pragma endregion une_variable

#pragma region une_function
typedef struct _une_function {
  wchar_t *name;
  une_node *params;
  une_node *body;
} une_function;
#pragma endregion une_function

void une_variable_free(une_variable variable);
void une_function_free(une_function function);

#endif /* !UNE_SYMBOLS_H */