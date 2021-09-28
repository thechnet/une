/*
context.c - Une
Modified 2021-09-28
*/

/* Header-specific includes. */
#include "context.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Allocates, initializes, and returns a pointer to a une_context struct.
*/
une_context *une_context_create(ptrdiff_t function, size_t variables_size)
{
  /* Allocate une_context. */
  une_context *context = malloc(sizeof(*context));
  verify(context);
  
  /* Initialize une_context. */
  *context = (une_context){
    .parent = NULL,
    .function = function,
    .variables = malloc(variables_size*sizeof(*context->variables)),
    .variables_size = variables_size,
    .variables_count = 0
  };
  verify(context->variables);
  
  return context;
}

/*
Frees all une_contexts, starting at youngest_child and up to, but not including, parent.
*/
void une_context_free_children(une_context *parent, une_context *youngest_child)
{
  une_context *context = youngest_child;
  while (context != parent) {
    une_context *older_context = context->parent;
    une_context_free(context);
    context = older_context;
  }
}

/*
Frees a une_context and its owned members.
*/
void une_context_free(une_context *context)
{
  /* Free une_variable buffer. */
  if (context->variables != NULL) {
    for (size_t i=0; i<context->variables_count; i++)
      une_variable_free(context->variables[i]);
    free(context->variables);
  }
  
  free(context);
}

/*
Initializes a new une_variable in a une_context's variable buffer.
*/
__une_variable_itf(une_variable_create)
{
  /* Ensure sufficient space in une_variable buffer. */
  if (context->variables_count >= context->variables_size) {
    context->variables_size *= 2;
    context->variables = realloc(context->variables, context->variables_size*sizeof(*context->variables));
    verify(context->variables);
  }
  
  /* Initialize une_variable. */
  une_variable *var = &((context->variables)[context->variables_count]);
  (context->variables_count)++;
  *var = (une_variable){
    .name = wcsdup(name),
    .content = une_result_create(UNE_RT_VOID) /* Don'use __UNE_RT_none__ because this will be freed using une_result_free. */
  };
  verify(var->name);
  
  return var;
}

/*
Returns a pointer to a une_variable in a une_context's variable buffer or NULL.
*/
__une_variable_itf(une_variable_find)
{
  /* Find une_variable. */
  for (size_t i=0; i<context->variables_count; i++)
    if (wcscmp(context->variables[i].name, name) == 0)
      return &context->variables[i];

  /* Return NULL if no match was found. */
  return NULL;
}

/*
Returns a pointer to a une_variable in a une_context's variable buffer and its parents or NULL.
*/
__une_variable_itf(une_variable_find_global)
{
  /* Return NULL by default. */
  une_variable *var = NULL;
  
  /* Find une_variable. */
  while (var == NULL) {
    var = une_variable_find(context, name);
    if (context->parent == NULL)
      break;
    context = context->parent;
  }
  
  return var;
}

/*
Returns a pointer to a une_variable in a une_context's variable buffer or creates and initializes it.
*/
__une_variable_itf(une_variable_find_or_create)
{
  /* Find une_variable. */
  une_variable *variable = une_variable_find(context, name);
  if (variable != NULL)
    return variable;
  
  /* une_variable doesn't exist yet, create it. */
  return une_variable_create(context, name);
}

/*
Returns a pointer to a une_variable in a une_context's variable buffer and its parents or creates and initializes it.
*/
__une_variable_itf(une_variable_find_or_create_global)
{
  /* Find une_variable. */
  une_variable *variable = une_variable_find_global(context, name);
  if (variable != NULL)
    return variable;
  
  /* une_variable doesn't exist yet, create it. */
  return une_variable_create(context, name);
}
