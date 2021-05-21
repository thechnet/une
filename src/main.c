/*
main.c - Une
Updated 2021-05-21
*/

#include "une.h"

#if defined(UNE_DEBUG) && defined(UNE_DEBUG_MALLOC_COUNTER)
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
    #if defined(UNE_DO_READ)
      context->text = str_to_wcs(argv[2]);
    #endif
  }
  // Run File
  else {
    if (argc < 2) WERR("No input file.");
    context->name = str_to_wcs(argv[1]);
    #if defined(UNE_DO_READ)
      context->text = file_read(argv[1]);
    #endif
  }
  
  #if defined(UNE_DO_LEX)
    #if defined(UNE_DO_READ)
      context->tokens = une_lex_wcs(context->text, &context->error);
    #else
      context->tokens = une_lex_file(context->name, &context->error);
    #endif
    if (context->tokens == NULL) {
      une_error_display(context->error, context->text, context->name);
      wprintf(L"\n");
    }
    #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
      une_tokens_display(context->tokens);
      wprintf(L"\n");
    #endif
  #endif

  #if defined(UNE_DO_PARSE)
    if (context->tokens != NULL) {
      context->ast = une_parse(
        context->tokens,
        &context->token_index,
        &context->error
      );
      if (context->ast == NULL) {
        une_error_display(context->error, context->text, context->name);
        une_node_free(context->ast, false);
        wprintf(L"\n");
      }
      #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_NODES)
        else {
          wchar_t *node_as_wcs = une_node_to_wcs(context->ast);
          wprintf(node_as_wcs);
          free(node_as_wcs);
          wprintf(L"\n\n"); // node_as_wcs does not add a newline.
        }
      #endif
    }
  #endif
  
  int final = 0;
  #if defined(UNE_DO_INTERPRET)
    if (context->ast != NULL) {
      context->variables_size = UNE_SIZE_SMALL; // FIXME: SIZE
      context->variables = rmalloc(context->variables_size*sizeof(*context->variables));
      context->functions_size = UNE_SIZE_SMALL; // FIXME: SIZE
      context->functions = rmalloc(context->functions_size*sizeof(*context->functions));
      une_result result = une_interpret(context->ast, context);
      if (result.type == UNE_RT_ERROR) {
        une_error_display(context->error, context->text, context->name);
        wprintf(L"\n");
      }
      #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_RESULT)
        else if (result.type != UNE_RT_VOID) {
          wchar_t *return_as_wcs = une_result_to_wcs(result);
          wprintf(L"%ls", return_as_wcs);
          free(return_as_wcs);
        }
        wprintf(L"\n");
      #endif
      if (result.type == UNE_RT_INT) final = result.value._int;
      une_result_free(result);
    }
  #endif
  
  une_context_free(context, false);

  #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_MEMORY_REPORT)
    #if defined(UNE_DEBUG) && defined(UNE_DEBUG_MALLOC_COUNTER )
      if (malloc_counter != 0) {
        wprintf(UNE_COLOR_FAIL L"%d memory location(s) not freed.\33[0m", malloc_counter);
      } else {
        wprintf(UNE_COLOR_SUCCESS L"All memory locations freed.\33[0m");
      }
    #else
      wprintf(L"\n\33[97mEnd of Program.\33[0m");
    #endif
    wprintf(L"\n");
  #endif
  
  return final;
}
