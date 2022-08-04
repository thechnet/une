/*
context.h - Une
Modified 2022-08-04
*/

#ifndef UNE_CONTEXT_H
#define UNE_CONTEXT_H

/* Header-specific includes. */
#include "../primitive.h"
#include "symbols.h"

/*
Holds information that changes depending on the execution context.
*/
typedef struct une_context_ {
  struct une_context_ *parent;
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
#define une_variable_itf__(id__) une_variable *(id__)(une_context *context, wchar_t *name) 

une_context *une_context_create(ptrdiff_t function, size_t variables_size);
void une_context_free_children(une_context *parent, une_context *youngest_child);
void une_context_free(une_context *context);

une_variable_itf__(une_variable_create);
une_variable_itf__(une_variable_find);
une_variable_itf__(une_variable_find_global);
une_variable_itf__(une_variable_find_or_create);
une_variable_itf__(une_variable_find_or_create_global);

#endif /* !UNE_CONTEXT_H */
