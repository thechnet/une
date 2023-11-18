/*
une.c - Une
Modified 2023-11-18
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
	
	/* Read file. */
	wchar_t *content = NULL;
	if (read_from_file) {
		char *resolved_path = une_resolve_path(path);
		if (!resolved_path || !une_file_exists(resolved_path))
			error = UNE_ERROR_SET(UNE_ET_FILE, (une_position){0});
		path = resolved_path;
		content = une_file_read(path);
	} else {
		content = text;
	}
	
	/* Lex. */
	une_lexer_state ls = une_lexer_state_create();
	if (content) {
		ls.tokens = malloc(UNE_SIZE_TOKEN_BUF*sizeof(*ls.tokens));
		verify(ls.tokens);
		ls.name = path;
		ls.text = content;
		ls.text_length = wcslen(ls.text);
		#ifndef UNE_NO_LEX
		une_lex(&error, &ls);
		#endif
	}
	
	#if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
	une_tokens_display(ls.tokens);
	wprintf(L"\n\n");
	#endif
	
	/* Parse. */
	une_node *ast = NULL;
	#ifndef UNE_NO_PARSE
	une_parser_state ps = une_parser_state_create();
	if (ls.tokens != NULL)
		ast = une_parse(&error, &ps, ls.tokens);
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
	if (existing_interpreter_state) {
		une_is = existing_interpreter_state;
	} else {
		_is = une_interpreter_state_create(read_from_file ? path : NULL);
		une_is = &_is;
	}
	
	une_result result = une_result_create(UNE_RT_ERROR);
	#ifndef UNE_NO_INTERPRET
	if (ast != NULL)
		result = une_interpret(&error, ast);
	#else
	result = une_result_create(UNE_RT_VOID);
	#endif
	if (did_exit != NULL)
		*did_exit = une_is->should_exit;
	une_interpreter_state_reset_flags(une_is);
	#ifdef UNE_DEBUG_LOG_INTERPRET
	success("interpret done.");
	#endif
	/* Wrap up. */
	int error_cases = 0;
	#ifndef UNE_NO_LEX
	if (ls.tokens == NULL)
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
		une_error_display(&error, &ls);
	}
	if (!existing_interpreter_state)
		une_interpreter_state_free(une_is);
	une_node_free(ast, false);
	une_tokens_free(ls.tokens);
	if (read_from_file) {
		if (content)
			free(content);
		if (path)
			free(path);
	}
	
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
Run a Une program without error handling, 'return' propagation, or managing incoming memory.
*/
une_result une_run_bare(une_error *error, char *path, wchar_t *text)
{
	/* Prepare. */
	*error = une_error_create();
	
	/* Read file. */
	wchar_t *content = NULL;
	if (path) {
		char *resolved_path = une_resolve_path(path);
		if (!resolved_path || !une_file_exists(resolved_path))
			*error = UNE_ERROR_SET(UNE_ET_FILE, (une_position){0});
		path = resolved_path;
		content = une_file_read(path);
	} else {
		content = text;
	}
	
	/* Lex. */
	une_lexer_state ls = une_lexer_state_create();
	ls.tokens = malloc(UNE_SIZE_TOKEN_BUF*sizeof(*ls.tokens));
	verify(ls.tokens);
	ls.text = content;
	ls.text_length = wcslen(ls.text);
	une_lex(error, &ls);
	if (!ls.tokens)
		return une_result_create(UNE_RT_ERROR);
	
	/* Parse. */
	une_parser_state ps = une_parser_state_create();
	une_node *ast = une_parse(error, &ps, ls.tokens);
	if (!ast) {
		une_tokens_free(ls.tokens);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Interpret. */
	if (!une_is->context)
		*une_is = une_interpreter_state_create(path);
	une_result result = une_interpret(error, ast);
	
	/* Finalize. */
	une_is->should_return = false;
	une_node_free(ast, false);
	une_tokens_free(ls.tokens);
	if (path) {
		free(content);
		free(path);
	}
	
	return result;
}
