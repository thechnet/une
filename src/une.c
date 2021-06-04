/*
une.c - Une
Modified 2021-06-04
*/

#include "une.h"

#include "types/token.h"
#include "types/node.h"
#include "types/lexer_state.h"
#include "types/parser_state.h"
#include "types/interpreter_state.h"
#include "types/error.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#pragma region une_run
une_result une_run(
  bool read_from_file,
  char *path,
  wchar_t *text
)
{
  une_error error = une_error_create();
  
  une_lexer_state ls = une_lexer_state_create(read_from_file, path, text);
  #ifdef UNE_DO_LEX
  une_token *tokens = une_lex(&error, &ls);
  #else
  une_token *tokens = NULL;
  #endif
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
    une_tokens_display(tokens);
    wprintf(L"\n\n");
  #endif
  
  une_parser_state ps = une_parser_state_create(tokens);
  une_node *ast = NULL;
  #ifdef UNE_DO_PARSE
  if (tokens != NULL) {
    ast = une_parse(&error, &ps);
  }
  #endif
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_NODES)
    wchar_t *node_as_wcs = une_node_to_wcs(ast);
    wprintf(node_as_wcs);
    free(node_as_wcs);
    wprintf(L"\n\n"); // node_as_wcs does not add a newline.
  #endif
  
  une_context *context = une_context_create(L"FIXME:", UNE_SIZE_SMALL, UNE_SIZE_SMALL); // FIXME: SIZE
  une_interpreter_state is = une_interpreter_state_create(context);
  une_result result = une_result_create(UNE_RT_ERROR);
  #ifdef UNE_DO_INTERPRET
  if (ast != NULL) {
    result = une_interpret(&error, &is, ast);
  }
  #endif
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
    wchar_t *return_as_wcs = une_result_to_wcs(result);
    wprintf(L"\n%ls\n\n", return_as_wcs);
    free(return_as_wcs);
  #endif
  
  if (result.type == UNE_RT_ERROR) {
    une_error_display(&error, &ls, &ps, &is);
  }
  
  une_interpreter_state_free(is); // FIXME:
  une_context_free(context);
  
  une_node_free(ast, false);
  une_parser_state_free(ps); // FIXME:
  
  une_tokens_free(tokens);
  une_lexer_state_free(ls); // FIXME:
  
  une_error_free(error);
  
  return result;
}
#pragma endregion une_run
