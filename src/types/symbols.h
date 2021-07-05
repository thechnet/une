/*
symbols.h - Une
Modified 2021-07-05
*/

#ifndef UNE_SYMBOLS_H
#define UNE_SYMBOLS_H

/* Header-specific includes. */
#include "../primitive.h"
#include "result.h"
#include "node.h"

/*
Holds the information for a variable.
*/
typedef struct _une_variable {
  wchar_t *name;
  une_result content;
} une_variable;

/*
Holds the information for a function.
*/
typedef struct _une_function {
  wchar_t *name;
  size_t params_count;
  wchar_t **params;
  une_node *body;
} une_function;

/*
*** Interface.
*/

void une_variable_free(une_variable variable);
void une_function_free(une_function function);

#endif /* !UNE_SYMBOLS_H */
