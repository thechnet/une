/*
context.c - Une
Updated 2021-04-28
*/

#include "context.h"

#pragma region une_context_free
// FIXME: Complete?
void une_context_free(une_context *context)
{
  // 1)
  if (context->name != NULL) free(context->name);
  #ifdef UNE_DO_READ
    if (context->text != NULL) free(context->text);
  #endif

  #ifdef UNE_DO_INTERPRET
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
  #endif

  // 4)
  #ifdef UNE_DO_PARSE
    une_node_free(context->ast, false);
  #endif
  // 5)
  #ifdef UNE_DO_LEX
    une_tokens_free(context->tokens);
  #endif
  // 6)
  // It's possible for une_error to contain pointers to allocated memory
  // that's used to provide details for an error message.
  // An example of this is the UNE_ET_GET (see une_interpret_get).
  
  une_error_free(context->error);
  
  free(context);
  #ifdef UNE_DEBUG_LOG_FREE
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