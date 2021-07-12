/*
context.c - Une
Modified 2021-07-11
*/

/* Header-specific includes. */
#include "context.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Allocates, initializes, and returns a pointer to a une_context struct.
*/
une_context *une_context_create(wchar_t *name, size_t variables_size, size_t functions_size)
{
  /* Allocate une_context. */
  une_context *context = malloc(sizeof(*context));
  
  /* Initialize une_context. */
  *context = (une_context){
    .name = wcsdup(name),
    .parent = NULL,
    .variables = malloc(variables_size*sizeof(*context->variables)),
    .variables_size = variables_size,
    .variables_count = 0,
    .functions = malloc(functions_size*sizeof(*context->functions)),
    .functions_size = functions_size,
    .functions_count = 0,
  };
  
  return context;
}

/*
Frees a une_context struct and its owned members.
*/
void une_context_free(une_context *context)
{
  /* Free une_variable buffer. */
  if (context->variables != NULL) {
    for (size_t i=0; i<context->variables_count; i++)
      une_variable_free(context->variables[i]);
    free(context->variables);
  }
  
  /* Free une_function buffer. */
  if (context->functions != NULL) {
    for (size_t i=0; i<context->functions_count; i++)
      une_function_free(context->functions[i]);
    free(context->functions);
  }
  
  /* Free context name. */
  free(context->name);
  
  une_context *parent = context->parent;
  free(context);
  if (parent != NULL)
    une_context_free(parent);
  
  LOGFREE(L"une_context", L"", 0);
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
  }
  
  /* Initialize une_variable. */
  une_variable *var = &((context->variables)[context->variables_count]);
  (context->variables_count)++;
  *var = (une_variable){
    .name = wcsdup(name),
    .content = une_result_create(UNE_RT_VOID) /* Don'use __UNE_RT_none__ because this will be freed using une_result_free. */
  };
  
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

/*
Initializes a new une_function in a une_context's function buffer.
*/
__une_function_itf(une_function_create)
{
  /* Ensure sufficient space in une_function buffer. */
  if (context->functions_count >= context->functions_size) {
    context->functions_size *= 2;
    context->functions = realloc(context->functions, context->functions_size*sizeof(*context->functions));
  }
  
  /* Initialize une_function. */
  une_function *var = &((context->functions)[context->functions_count]);
  (context->functions_count)++;
  *var = (une_function){
    .name = wcsdup(name),
    .params_count = 0,
    .params = NULL,
    .body = NULL
  };
  
  return var;
}

/*
Returns a pointer to a une_function in a une_context's function buffer or NULL.
*/
__une_function_itf(une_function_find)
{
  /* Find une_function. */
  for (size_t i=0; i<context->functions_count; i++)
    if (wcscmp(context->functions[i].name, name) == 0)
      return &context->functions[i];
  
  /* Return NULL if no match was found. */
  return NULL;
}

/*
Returns a pointer to a une_function in a une_context's function buffer and its parents or NULL.
*/
__une_function_itf(une_function_find_global)
{
  /* Return NULL by default. */
  une_function *fn = NULL;
  
  /* Find une_function. */
  while (fn == NULL) {
    fn = une_function_find(context, name);
    if (context->parent == NULL)
      break;
    context = context->parent;
  }
  
  return fn;
}
