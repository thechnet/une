/*
context.c - Une
Updated 2021-04-24
*/

#include "context.h"

#pragma region une_context_free
// FIXME: Complete?
void une_context_free(une_context *context)
{
  // 1)
  free(context->name);
  #ifdef UNE_DO_READ
    free(context->text);
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
    une_node_free(context->ast);
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
    wprintf(L"Context\n");
  #endif
}
#pragma endregion une_context_free

#pragma region une_context_create
une_context *une_context_create(void)
{
  une_context *context = malloc(sizeof(*context));
  if (context == NULL) WERR(L"Out of memory.");
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
  return context;
}
#pragma endregion une_context_create
