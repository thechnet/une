/*
main.c - Une
Updated 2021-05-03
*/

#include "une.h"

#ifdef UNE_DEBUG_MALLOC_COUNTER
  int malloc_counter = 0;
#endif

int main(int argc, char *argv[])
{
  #if defined(UNE_DEBUG) && defined(UNE_DRAW_BAR)
    // Prints a bar to distinguish new output from old output in the terminal.
    wprintf(UNE_COLOR_SUCCESS L"\33[7m\33[K\33[27m\33[0m\n");
  #endif

  une_context *context = une_context_create();

  // Run Command Line
  if (argc > 2 && strcmp(argv[1], "do") == 0) {
    context->name = rmalloc((wcslen(L"<string>")+1)*sizeof(*context->name));
    wcscpy(context->name, L"<string>");
    #ifdef UNE_DO_READ
      context->text = str_to_wcs(argv[2]);
    #endif
  }
  // Run File
  else {
    if (argc < 2) WERR("No input file.");
    context->name = str_to_wcs(argv[1]);
    #ifdef UNE_DO_READ
      context->text = file_read(argv[1]);
    #endif
  }
  
  #ifdef UNE_DO_LEX
    #ifdef UNE_DO_READ
      context->tokens = une_lex_wcs(context->text, &context->error);
    #else
      context->tokens = une_lex_file(context->name, &context->error);
    #endif
    if (context->tokens == NULL) {
      une_error_display(context->error, context->text, context->name);
      wprintf(L"\n");
      return 1;
    }
    #ifdef UNE_DISPLAY_TOKENS
      une_tokens_display(context->tokens);
      wprintf(L"\n");
    #endif
  #endif

  #ifdef UNE_DO_PARSE
    context->ast = une_parse(
      context->tokens,
      &context->token_index,
      &context->error
    );
    if (context->ast == NULL) {
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(UNE_COLOR_HINT L"%hs:%hs:%d:" UNE_COLOR_NEUTRAL L" \n", __FILE__, __FUNCTION__, __LINE__);
      #endif
      une_error_display(context->error, context->text, context->name);
      wprintf(L"\n");
      une_node_free(context->ast, false);
      return 1;
    }
    #ifdef UNE_DISPLAY_NODES
      wchar_t *node_as_wcs = une_node_to_wcs(context->ast);
      wprintf(node_as_wcs);
      free(node_as_wcs);
      wprintf(L"\n");
    #endif
  #endif
  
  int final = 0;
  #ifdef UNE_DO_INTERPRET
    context->variables_size = UNE_SIZE_SMALL; // FIXME: SIZE
    context->variables = rmalloc(context->variables_size*sizeof(*context->variables));
    context->functions_size = UNE_SIZE_SMALL; // FIXME: SIZE
    context->functions = rmalloc(context->functions_size*sizeof(*context->functions));
    une_result result = une_interpret(context->ast, context);
    if (result.type == UNE_RT_ERROR) {
      wprintf(L"\n");
      une_error_display(context->error, context->text, context->name);
    }
    #ifdef UNE_DISPLAY_RESULT
      else {
        wchar_t *return_as_wcs = une_result_to_wcs(result);
        wprintf(L"\n%ls\n", return_as_wcs);
        free(return_as_wcs);
      }
    #endif
    if (result.type == UNE_RT_INT) final = result.value._int;
    une_result_free(result);
  #endif
  
  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(L"\n");
  #endif

  une_context_free(context);

  #ifdef UNE_DISPLAY_MEMORY_REPORT
    #ifdef UNE_DEBUG_MALLOC_COUNTER 
      if (malloc_counter != 0) {
        wprintf(UNE_COLOR_FAIL L"\n%d memory location(s) not freed.\33[0m", malloc_counter);
      } else {
        wprintf(UNE_COLOR_SUCCESS L"\nAll memory locations freed.\33[0m");
      }
    #else
      wprintf(L"\n\33[97mMemory freed.\33[0m");
    #endif
    wprintf(L"\n");
  #endif
  
  
  return final;
}
