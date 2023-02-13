/*
symbols.h - Une
Modified 2023-02-13
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
typedef struct une_association_ {
  wchar_t *name;
  une_result content;
} une_association;

/*
Holds the information for a function.
*/
typedef struct une_function_ {
  char *definition_file;
  une_position definition_point;
  size_t params_count;
  wchar_t **params;
  une_node *body;
} une_function;

/*
*** Interface.
*/

void une_variable_free(une_association variable);
void une_function_free(une_function *function);

#endif /* !UNE_SYMBOLS_H */
