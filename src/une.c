/*
une.c - Une
Modified 2021-10-20
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
  une_token *tokens = NULL;
  #ifndef UNE_NO_LEX
  tokens = une_lex(&error, &ls);
  #endif
  
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
  une_tokens_display(tokens);
  wprintf(L"\n\n");
  #endif
  
  /* Parse. */
  une_node *ast = NULL;
  #ifndef UNE_NO_PARSE
  une_parser_state ps = une_parser_state_create();
  if (tokens != NULL)
    ast = une_parse(&error, &ps, tokens);
  #endif
  #ifdef UNE_DEBUG_LOG_PARSE
  success("parse done.");
  #endif
  
  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_NODES)
  wchar_t *node_as_wcs = une_node_to_wcs(ast);
  wprintf(node_as_wcs);
  free(node_as_wcs);
  wprintf(L"\n\n"); /* node_as_wcs does not add a newline. */
  #endif
  
  /* Interpret. */
  une_context *context = une_context_create(-1, UNE_SIZE_VARIABLE_BUF);
  
  une_interpreter_state is = une_interpreter_state_create(context, UNE_SIZE_FUNCTION_BUF);
  une_result result = une_result_create(UNE_RT_ERROR);
  #ifndef UNE_NO_INTERPRET
  if (ast != NULL)
    result = une_interpret(&error, &is, ast);
  #else
  result = une_result_create(UNE_RT_VOID);
  #endif
  #ifdef UNE_DEBUG_LOG_INTERPRET
  success("interpret done.");
  #endif
  /* Wrap up. */
  if (
    #ifndef UNE_NO_LEX
    tokens == NULL ||
    #endif
    #ifndef UNE_NO_PARSE
    ast == NULL ||
    #endif
    #ifndef UNE_NO_INTERPRET
    result.type == UNE_RT_ERROR ||
    #endif
    false
  ) {
    assert(error.type != __UNE_ET_none__);
    une_error_display(&error, &ls, &is);
  }
  une_interpreter_state_free(&is);
  
  une_context_free_children(NULL, is.context);
  une_node_free(ast, false);
  une_tokens_free(tokens);
  
  #if defined(UNE_DEBUG) && defined(UNE_DEBUG_REPORT)
  if (result.type == UNE_RT_ERROR) {
    une_result_free(result);
    return (une_result){
      .type = UNE_RT_ERROR,
      .value._int = (une_int)error.type+(une_int)UNE_R_END_DATA_RESULT_TYPES
    };
  }
  #endif /* UNE_DEBUG_REPORT */
  
  return result;
}
