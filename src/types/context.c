/*
context.c - Une
Updated 2021-05-10
*/

#include "context.h"

#pragma region une_context_free
// FIXME: Complete?
void une_context_free(une_context *context, bool is_function_context)
{
  // 1)
  if (context->name != NULL) {
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
      wprintf(UNE_COLOR_HINT L"%hs:%hs:%d: " UNE_COLOR_NEUTRAL L"name\n", __FILE__, __FUNCTION__, __LINE__);
    #endif
    free(context->name);
  }
  
  #if defined(UNE_DO_READ)
    if (context->text != NULL) {
      #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d: " UNE_COLOR_NEUTRAL L"text\n", __FILE__, __FUNCTION__, __LINE__);
      #endif
      free(context->text);
    }
  #endif

  #if defined(UNE_DO_INTERPRET)
    if (context->ast != NULL || is_function_context) {
      // 2)
      for (size_t i=0; i<context->variables_count; i++) {
        une_variable_free(context->variables[i]);
      }
      free(context->variables);
      // 3)
      for (size_t i=0; i<context->functions_count; i++) {
        une_function_free(context->functions[i]);
      }
      free(context->functions);
    }
  #endif

  if (context->tokens != NULL) {
    // 4)
    #if defined(UNE_DO_PARSE)
      une_node_free(context->ast, false);
    #endif
    // 5)
    #if defined(UNE_DO_LEX)
      une_tokens_free(context->tokens);
    #endif
  }
  
  // 6)
  // It's possible for une_error to contain pointers to allocated memory
  // that's used to provide details for an error message.
  // An example of this is the UNE_ET_GET (see une_interpret_get).
  une_error_free(context->error);
  
  free(context);
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_LOG_FREE)
    wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" Context\n", __FILE__, __FUNCTION__, __LINE__);
  #endif
}
#pragma endregion une_context_free

#pragma region une_context_create
une_context *une_context_create(void)
{
  une_context *context = rmalloc(sizeof(*context));
  context->parent = NULL;
  context->name = NULL;
  context->text = NULL;
  context->error = (une_error){
    .type = UNE_ET_NO_ERROR,
    .pos = (une_position){0, 1},
    .values[0]._wcs = NULL,
    .values[1]._wcs = NULL,
  };
  context->tokens = NULL;
  context->token_index = 0;
  context->ast = NULL;
  context->variables = NULL;
  context->variables_size = 0;
  context->variables_count = 0;
  context->functions = NULL;
  context->functions_size = 0;
  context->functions_count = 0;
  context->should_return = false;
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
