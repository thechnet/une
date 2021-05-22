/*
main.c - Une
Updated 2021-05-22
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

  une_instance instance = une_instance_create();
  
  if (argc < 2) WERR("No input file.");
  instance.is.context->name = str_to_wcs(argv[1]);
  #if defined(UNE_DO_READ)
    instance.ls.wcs = file_read(argv[1]);
  #endif
  
  #if defined(UNE_DO_LEX)
    instance.ps.tokens = une_lex_wcs(&instance);
    if (instance.ps.tokens == NULL) {
      une_error_display(instance.error, instance.ls, instance.is.context->name);
      wprintf(L"\n");
    }
    #if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
      une_tokens_display(context->tokens);
      wprintf(L"\n");
    #endif
  #endif

  #if defined(UNE_DO_PARSE)
    une_node *ast = NULL;
    if (instance.ps.tokens != NULL) {
      ast = une_parse(&instance);
      if (ast == NULL) {
        une_error_display(instance.error, instance.ls, instance.is.context->name);
        une_node_free(ast, false);
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
    if (ast != NULL) {
      instance.is.context->variables_size = UNE_SIZE_SMALL; // FIXME: SIZE
      instance.is.context->variables = rmalloc(
        instance.is.context->variables_size*sizeof(*instance.is.context->variables)
      );
      instance.is.context->functions_size = UNE_SIZE_SMALL; // FIXME: SIZE
      instance.is.context->functions = rmalloc(
        instance.is.context->functions_size*sizeof(*instance.is.context->functions)
      );
      une_result result = une_interpret(&instance, ast);
      instance.is.should_return = false;
      if (result.type == UNE_RT_ERROR) {
        une_error_display(instance.error, instance.ls, instance.is.context->name);
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
  
  une_node_free(ast, false);
  une_instance_free(instance);

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
