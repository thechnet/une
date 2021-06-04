/*
context.c - Une
Updated 2021-06-04
*/

#include "context.h"

#pragma region une_context_free
void une_context_free(une_context *context)
{
  free(context->name);

  #if defined(UNE_DO_INTERPRET)
    if (context->variables != NULL) {
      // 2)
      for (size_t i=0; i<context->variables_count; i++) {
        une_variable_free(context->variables[i]);
      }
      free(context->variables);
    }
    if (context->functions != NULL) {
      // 3)
      for (size_t i=0; i<context->functions_count; i++) {
        une_function_free(context->functions[i]);
      }
      free(context->functions);
    }
  #endif
  
  free(context);
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
    wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Context\n", __FILE__, __FUNCTION__, __LINE__);
  #endif
}
#pragma endregion une_context_free

#pragma region une_context_create
une_context *une_context_create(wchar_t *name, size_t variables_size, size_t functions_size)
{
  une_context *context = rmalloc(sizeof(*context));
  *context = (une_context){
    .name = wcs_dup(name),
    .parent = NULL,
    .variables = rmalloc(variables_size*sizeof(*context->variables)),
    .variables_size = variables_size,
    .variables_count = 0,
    .functions = rmalloc(functions_size*sizeof(*context->functions)),
    .functions_size = functions_size,
    .functions_count = 0,
  };
  return context;
}
#pragma endregion une_context_create

#pragma region une_variable_create
une_variable *une_variable_create(
  une_variable **variables,
  size_t *variables_count,
  size_t *variables_size,
  wchar_t *name
)
{
  if (*variables_count >= *variables_size) {
    *variables_size *= 2;
    une_variable *_variables = rrealloc(*variables, *variables_size*sizeof(*_variables));
    *variables = _variables;
  }
  une_variable *var = &((*variables)[*variables_count]);
  var->name = wcs_dup(name);
  var->content = une_result_create(UNE_RT_VOID);
  (*variables_count)++;
  return var;
}
#pragma endregion une_variable_create

#pragma region une_variable_find
une_variable *une_variable_find(une_variable *variables, size_t variables_count, wchar_t *name)
{
  for (size_t i=0; i<variables_count; i++) {
    if (wcscmp(variables[i].name, name) == 0) return &variables[i];
  }
  return NULL;
}
#pragma endregion une_variable_find

#pragma region une_variable_find_global
une_variable *une_variable_find_global(une_context *context, wchar_t *name)
{
  une_variable *var = NULL;
  while (var == NULL) {
    var = une_variable_find(context->variables, context->variables_count, name);
    if (context->parent == NULL) break;
    context = context->parent;
  }
  return var;
}
#pragma endregion une_variable_find_global

#pragma region une_variable_find_or_create
une_variable *une_variable_find_or_create(une_context *context, wchar_t *name)
{
  une_variable *variable = une_variable_find(
    context->variables,
    context->variables_count,
    name
  );

  if (variable != NULL) return variable;
  
  return une_variable_create(
    &context->variables,
    &context->variables_count,
    &context->variables_size,
    name
  );
}
#pragma endregion une_variable_find_or_create

#pragma region une_variable_find_or_create_global
une_variable *une_variable_find_or_create_global(une_context *context, wchar_t *name)
{
  une_variable *variable = une_variable_find_global(context, name);

  if (variable != NULL) return variable;
  
  return une_variable_create(
    &context->variables,
    &context->variables_count,
    &context->variables_size,
    name
  );
}
#pragma endregion une_variable_find_or_create_global

#pragma region une_function_create
une_function *une_function_create(
  une_function **functions,
  size_t *functions_count,
  size_t *functions_size,
  wchar_t *name
)
{
  if (*functions_count >= *functions_size) {
    *functions_size *= 2;
    une_function *_functions = rrealloc(*functions, *functions_size*sizeof(*_functions));
    *functions = _functions;
  }
  une_function *var = &((*functions)[*functions_count]);
  var->name = wcs_dup(name);
  var->params_count = 0;
  var->params = NULL;
  var->body = NULL;
  (*functions_count)++;
  return var;
}
#pragma endregion une_function_create

#pragma region une_function_find
une_function *une_function_find(une_function *functions, size_t functions_count, wchar_t *name)
{
  for (size_t i=0; i<functions_count; i++) {
    if (wcscmp(functions[i].name, name) == 0) return &functions[i];
  }
  return NULL;
}
#pragma endregion une_function_find

#pragma region une_function_find_global
une_function *une_function_find_global(une_context *context, wchar_t *name)
{
  une_function *fn = NULL;
  while (fn == NULL) {
    fn = une_function_find(context->functions, context->functions_count, name);
    if (context->parent == NULL) break;
    context = context->parent;
  }
  return fn;
}
#pragma endregion une_function_find_global
