/*
context.h - Une
Modified 2023-02-22
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
  char *entry_file;
  une_position entry_point;
  size_t variables_size;
  size_t variables_count;
  une_association **variables;
} une_context;

/*
*** Interface.
*/

/*
Variable interface function template.
*/
#define une_variable_itf__(id__) une_association *(id__)(une_context *context, wchar_t *name) 

une_context *une_context_create(char *entry_file, une_position entry_point);
une_context *une_context_get_oldest_parent(une_context *context);
void une_context_free_children(une_context *parent, une_context *youngest_child);
void une_context_free(une_context *context);

une_variable_itf__(une_variable_create);
une_variable_itf__(une_variable_find);
une_variable_itf__(une_variable_find_global);
une_variable_itf__(une_variable_find_or_create);
une_variable_itf__(une_variable_find_or_create_global);

#endif /* !UNE_CONTEXT_H */
