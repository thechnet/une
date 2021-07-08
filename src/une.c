/*
une.c - Une
Modified 2021-07-08
*/

/* Header-specific includes. */
#include "une.h"

/* Implementation-specific includes. */
#include "primitive.h"
#include "types/token.h"
#include "types/node.h"
#include "types/lexer_state.h"
#include "types/parser_state.h"
#include "types/interpreter_state.h"
#include "types/error.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "tools.h"

/*
Run a Une program.
*/
une_result une_run(bool read_from_file, char *path, wchar_t *text)
{
  /* Setup. */
  une_error error = une_error_create();
  
  /* Lex. */
  une_lexer_state ls = une_lexer_state_create(read_from_file, path, text);
  #ifndef UNE_NO_LEX
  une_token *tokens = une_lex(&error, &ls);
  #else
  une_token *tokens = NULL;
  #endif
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
  une_tokens_display(tokens);
  wprintf(L"\n\n");
  #endif
  
  /* Parse. */
  une_parser_state ps = une_parser_state_create();
  une_node *ast = NULL;
  #ifndef UNE_NO_PARSE
  if (tokens != NULL)
    ast = une_parse(&error, &ps, tokens);
  #endif
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_NODES)
  wchar_t *node_as_wcs = une_node_to_wcs(ast);
  wprintf(node_as_wcs);
  free(node_as_wcs);
  wprintf(L"\n\n"); /* node_as_wcs does not add a newline. */
  #endif
  
  /* Interpret. */
  wchar_t *context_name = ls.read_from_file ? une_str_to_wcs(path) : UNE_DEFAULT_CONTEXT_NAME;
  une_context *context = une_context_create(context_name, UNE_SIZE_VARIABLE_BUF, UNE_SIZE_FUNCTION_BUF);
  if (ls.read_from_file)
    free(context_name);
  une_interpreter_state is = une_interpreter_state_create(context);
  une_result result = une_result_create(UNE_RT_VOID);
  #ifndef UNE_NO_INTERPRET
  if (ast != NULL)
    result = une_interpret(&error, &is, ast);
  #endif
  
  /* Wrap up. */
  if (tokens == NULL || ast == NULL || result.type == UNE_RT_ERROR) {
    if (error.type != __UNE_ET_none__)
      une_error_display(&error, &ls, &is);
    else
      fail(L"Expected error, but error undefined. Check UNE_NO_*.");
  }
  une_context_free(context);
  une_node_free(ast, false);
  une_tokens_free(tokens);
  return result;
}
