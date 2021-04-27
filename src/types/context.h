/*
context.h - Une
Updated 2021-04-24
*/

#ifndef UNE_CONTEXT_H
#define UNE_CONTEXT_H

#include "../primitive.h"
#include "error.h"
#include "token.h"
#include "node.h"
#include "symbols.h"

#pragma region une_context
typedef struct _une_context {
  wchar_t *name;
  wchar_t *text;
  une_token *tokens;
  size_t token_index;
  une_node *ast;
  une_error error;
  struct _une_context *parent;
  une_variable *variables;
  size_t variables_size;
  size_t variables_count;
  une_function *functions;
  size_t functions_size;
  size_t functions_count;
  bool should_return;
} une_context;
#pragma endregion une_context

void une_context_free(une_context *context);
une_context *une_context_create(void);

#endif /* !UNE_CONTEXT_H */
