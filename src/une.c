/*
une.c - Une
Modified 2023-05-13
*/

/* Header-specific includes. */
#include "une.h"

/* Implementation-specific includes. */
#include "primitive.h"
#include "types/token.h"
#include "types/node.h"
#include "types/lexer_state.h"
#include "types/parser_state.h"
#include "types/error.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "tools.h"

/*
Run a Une program.
*/
une_result une_run(bool read_from_file, char *path, wchar_t *text, bool *did_exit, une_interpreter_state *existing_interpreter_state)
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
  
  /* Get interpreter state. */
  une_interpreter_state _is;
  une_interpreter_state *is;
  if (existing_interpreter_state) {
    is = existing_interpreter_state;
  } else {
    _is = une_interpreter_state_create();
    is = &_is;
  }
  une_is = is;
  
  une_result result = une_result_create(UNE_RT_ERROR);
  #ifndef UNE_NO_INTERPRET
  if (ast != NULL)
    result = une_interpret(&error, is, ast);
  #else
  result = une_result_create(UNE_RT_VOID);
  #endif
  if (did_exit != NULL)
    *did_exit = is->should_exit;
  is->should_return = false;
  is->should_exit = false;
  #ifdef UNE_DEBUG_LOG_INTERPRET
  success("interpret done.");
  #endif
  /* Wrap up. */
  int error_cases = 0;
  #ifndef UNE_NO_LEX
  if (tokens == NULL)
    error_cases++;
  #endif
  #ifndef UNE_NO_PARSE
  if (ast == NULL)
    error_cases++;
  #endif
  #ifndef UNE_NO_INTERPRET
  if (result.type == UNE_RT_ERROR)
    error_cases++;
  #endif
  if (error_cases > 0) {
    assert(error.type != UNE_ET_none__);
    une_error_display(&error, &ls, is);
  }
  if (!existing_interpreter_state)
    une_interpreter_state_free(is);
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

/*
Run a Une program, without handling errors or freeing memory.
*/
une_result une_run_bare(une_error *error, une_interpreter_state *is, char *path, wchar_t *text)
{
  /* Prepare. */
  *error = une_error_create();
  
  /* Lex. */
  une_lexer_state ls = une_lexer_state_create(path ? true : false, path, text);
  une_token *tokens = une_lex(error, &ls);
  if (!tokens)
    return une_result_create(UNE_RT_ERROR);
  
  /* Parse. */
  une_parser_state ps = une_parser_state_create();
  une_node *ast = une_parse(error, &ps, tokens);
  if (!ast) {
    une_tokens_free(tokens);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Interpret. */
  if (!is->context)
    *is = une_interpreter_state_create();
  une_result result = une_interpret(error, is, ast);
  
  /* Finalize. */
  une_node_free(ast, false);
  une_tokens_free(tokens);
  
  return result;
}
