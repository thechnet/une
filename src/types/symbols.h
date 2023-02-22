/*
symbols.h - Une
Modified 2023-02-22
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
*** Interface.
*/

void une_variable_free(une_association *variable);

#endif /* !UNE_SYMBOLS_H */
