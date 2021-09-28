/*
context.h - Une
Modified 2021-09-28
*/

#ifndef UNE_CONTEXT_H
#define UNE_CONTEXT_H

/* Header-specific includes. */
#include "../primitive.h"
#include "symbols.h"

/*
Holds information that changes depending on the execution context.
*/
typedef struct _une_context {
  struct _une_context *parent;
  ptrdiff_t function;
  size_t variables_size;
  size_t variables_count;
  une_variable *variables;
} une_context;

/*
*** Interface.
*/

/*
Variable interface function template.
*/
#define __une_variable_itf(__id) une_variable *(__id)(une_context *context, wchar_t *name) 

une_context *une_context_create(ptrdiff_t function, size_t variables_size);
void une_context_free_children(une_context *parent, une_context *youngest_child);
void une_context_free(une_context *context);

__une_variable_itf(une_variable_create);
__une_variable_itf(une_variable_find);
__une_variable_itf(une_variable_find_global);
__une_variable_itf(une_variable_find_or_create);
__une_variable_itf(une_variable_find_or_create_global);

#endif /* !UNE_CONTEXT_H */
