/*
main.c - Une
Updated 2021-04-17
*/

/*
macOS
clear && clang test-2.c -o test-1.exe && ./test-1.exe test-2-program.txt

Windows
cls & gcc test-2.c -o test-2.exe && test-2.exe test-2-program.txt
cls & tcc test-2.c -o test-2.exe && test-2.exe test-2-program.txt
cls & clang test-2.c -o test-2.exe -Wno-deprecated && test-2.exe test-2-program.txt

Visual Studio Code Developer Prompt (Administrator)
cls & cl test-2.c /Fetest-2.exe /Fo"%temp%\cs.obj"

Planned for test-3:
- Rework node data and branches:
  - Every node has four 'une_value's.
  - The content of the values is determined by the node type.

Planned for this test:
- Finish interpretation.

Backlog:
- Memory Pool
  - Nodes are placed next to each other in a memory pool (node_index).
  - Branches are just pointers to indices of other nodes.
  UNSURE: Is this extra effort worth it?
- Memory index for nodes to simplify freeing memory.
  UNSURE: Freeing memory as we go isn't that bad.

CHECKLIST:
- Do all variables allocate their OWN memory?
- Is there a way to not always have to duplicate strings for results?
- Can atom include '(' highest_order_expression() ')'? (currently conditional_operation)
- Are we checking for NULL after every malloc or realloc?
- Do all nodes store correct positions?
- Allow Conditional Operation for Statements?
- TODO: Index ranges (0:5)
- TODO: Better SET and GET parser and interpreter implementations.

FIXME: Shared code between une_node_to_wcs and une_result_to_wcs.

Notes:
- The SET node returns the value the variable was set to. This potentially allows
  Python-like a = b = 1 stmts.
*/

// #define UNE_DEBUG_LOG_FREE

#include "une.h"

#ifdef UNE_DEBUG_MALLOC_COUNTER
  int malloc_counter = 0;
#endif

int main(int argc, char *argv[])
{
  // Prints a bar to distinguish new output from old output in the terminal.
  wprintf(UNE_COLOR_SUCCESS L"\33[7m\33[K\33[27m\33[0m\n");

  une_context *context = une_context_create();

  // Run Command Line
  if(argc > 2 && strcmp(argv[1], "do") == 0)
  {
    context->name = malloc((strlen("<string>")+1)*sizeof(*context->name));
    if(context->name == NULL) WERR(L"Out of memory.");
    strcpy(context->name, "<string>");
    #ifdef UNE_DO_READ
      context->text = str_to_wcs(argv[2]);
    #endif
  }
  // Run File
  else
  {
    if(argc < 1 || access(argv[1], R_OK)) WERR(L"File not found.");
    context->name = malloc((strlen(argv[1])+1)*sizeof(*context->name));
    if(context->name == NULL) WERR(L"Out of memory.");
    strcpy(context->name, argv[1]);
    #ifdef UNE_DO_READ
      context->text = file_read(argv[1]);
    #endif
  }
  
  #ifdef UNE_DO_LEX
    context->tokens = une_lex_wcs(context->text, &context->error);
    if(context->tokens == NULL)
    {
      une_error_display(context->error, context->text, context->name);
      wprintf(L"\n");
      return 1;
    }
    une_tokens_display(context->tokens);
  #endif

  #ifdef UNE_DO_PARSE
    wprintf(L"\n");
    context->ast = une_node_create(UNE_NT_STMTS);
    context->ast->pos.start = context->tokens[context->token_index].pos.start;
    une_node **sequence = une_parse_sequence(
      context->tokens, &context->token_index, &context->error,
      &une_parse_stmt, UNE_TT_NEW, UNE_TT_EOF
    );
    if(sequence == NULL)
    {
      #ifdef UNE_DEBUG_LOG_FREE
        wprintf(L"\n");
      #endif
      une_error_display(context->error, context->text, context->name);
      wprintf(L"\n");
      une_node_free(context->ast);
      return 1;
    }
    context->ast->pos.end = context->tokens[context->token_index].pos.end;
    context->ast->content.value._vpp = (void**)sequence;
  
    wchar_t *node_as_wcs = une_node_to_wcs(context->ast);
    wprintf(node_as_wcs);
    free(node_as_wcs);
    wprintf(L"\n");
  #endif
  
  int final = 0;
  
  #ifdef UNE_DO_INTERPRET
    context->variables_size = UNE_SIZE_SMALL; // FIXME: SIZE
    context->variables = malloc(context->variables_size*sizeof(*context->variables))
    if(context->variables == NULL) WERR(L"Out of memory.");
    context->functions_size = UNE_SIZE_SMALL; // FIXME: SIZE
    context->functions = malloc(context->functions_size*sizeof(*context->functions))
    if(context->functions == NULL) WERR(L"Out of memory.");
    une_result result = une_interpret(context->ast, context);
    if(result.type == UNE_RT_ERROR)
    {
      wprintf(L"\n");
      une_error_display(context->error, context->text, context->name);
    }
    else
    {
      wchar_t *return_as_wcs = une_result_to_wcs(result);
      wprintf(L"\n%ls\n", return_as_wcs);
      free(return_as_wcs);
    }
    if(result.type == UNE_RT_INT) final = result.value._int;
    une_result_free(result);
  #endif

  #ifdef UNE_DEBUG_LOG_FREE
    wprintf(L"\n");
  #endif

  une_context_free(context);
  
  #ifdef UNE_DEBUG_MALLOC_COUNTER
    if(malloc_counter != 0)
    {
      wprintf(UNE_COLOR_FAIL L"\n%d memory location(s) not freed.\33[0m", malloc_counter);
    }
    else
    {
      wprintf(UNE_COLOR_SUCCESS L"\nAll memory locations freed.\33[0m");
    }
  #else
    wprintf(L"\n\33[97mMemory freed.\33[0m");
  #endif
  
  wprintf(L"\n");
  
  return final;
}