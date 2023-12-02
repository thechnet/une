/*
engine.c - Une
Modified 2023-11-26
*/

/* Header-specific includes. */
#include "engine.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "../lexer.h"
#include "../parser.h"
#include "../interpreter.h"

/*
Globals.
*/

une_engine *felix = NULL; /* FIXME: This name is a placeholder to make search-and-replace easier. */

/*
Interface.
*/

une_engine une_engine_create_engine(void)
{
	return (une_engine){
		.error = une_error_create(),
		.is = une_interpreter_state_create(NULL)
	};
}

void une_engine_select_engine(une_engine *engine)
{
	felix = engine;
}

void une_engine_free(void)
{
	une_interpreter_state_free(&felix->is);
	felix = NULL;
}

void une_engine_prepare_for_next_module(void)
{
	felix->error.type = UNE_ET_none__;
	felix->is.should_return = false;
	felix->is.should_exit = false;
}

une_module *une_engine_new_module_from_file_or_wcs(char *path, wchar_t *wcs)
{
	UNE_VERIFY_ENGINE;
	bool originates_from_file = path != NULL;
	if (originates_from_file)
		assert(!wcs);
	else
		assert(!path && wcs);

	char *stored_path = NULL;
	wchar_t *source = NULL;
	if (originates_from_file) {
		#ifdef DEBUG_USE_ABSOLUTE_MODULE_PATHS
		stored_path = une_resolve_path(path);
		#else
		stored_path = strdup(path);
		verify(stored_path);
		#endif
		if (!stored_path || !une_file_exists(stored_path))
			felix->error = UNE_ERROR_SET(UNE_ET_FILE, (une_position){0});
		else
			source = une_file_read(stored_path, true, UNE_TAB_WIDTH);
	} else {
		source = wcsdup(wcs);
	}

	une_module *module = une_modules_add_module(&felix->is.modules);
	assert(module);

	module->originates_from_file = originates_from_file;
	module->path = stored_path;
	module->source = source;

	if (source) {
		une_lexer_state ls = une_lexer_state_create();
		ls.tokens = malloc(UNE_SIZE_TOKEN_BUF*sizeof(*ls.tokens));
		verify(ls.tokens);
		ls.text = module->source;
		ls.text_length = wcslen(ls.text);
		#ifndef UNE_NO_LEX
		une_lex(&felix->error, &ls);
		#endif
		module->tokens = ls.tokens;
	}

	#if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
	wprintf(L"\n");
	une_tokens_display(module->tokens);
	wprintf(L"\n\n");
	#endif

	return module;
}

une_callable *une_engine_parse_module(une_module *module)
{
	UNE_VERIFY_ENGINE;
	assert(module);

	assert(module->tokens);

	une_parser_state ps = une_parser_state_create();
	ps.module_id = module->id;
	une_node *ast = NULL;
	#ifndef UNE_NO_PARSE
	ast = une_parse(&felix->error, &ps, module->tokens);
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

	une_callable *callable = NULL;
	if (ast) {
		callable = une_callables_add_callable(&felix->is.callables);
		assert(callable);

		callable->module_id = module->id;
		callable->body = ast;
		callable->borrows_body_strings = true;
	}

	return callable;
}

une_result une_engine_interpret_file_or_wcs_with_position(char *path, wchar_t *wcs, une_position current_context_exit_position)
{
	une_engine_prepare_for_next_module();

	une_module *module = une_engine_new_module_from_file_or_wcs(path, wcs);
	
	une_context *parent = felix->is.context;
	parent->exit_position = current_context_exit_position;
	felix->is.context = une_context_create_transparent();
	felix->is.context->parent = parent;
	felix->is.context->module_id = module->id;

	if (!module->tokens)
		return une_result_create(UNE_RT_ERROR);

	une_callable *callable = une_engine_parse_module(module);
	if (!callable)
		return une_result_create(UNE_RT_ERROR);
	
	felix->is.context->callable_id = callable->id;

	une_result result = une_result_create(UNE_RT_VOID);
	#ifndef UNE_NO_INTERPRET
	result = une_interpret(callable->body);
	felix->is.should_return = false;
	#endif
	#ifdef UNE_DEBUG_LOG_INTERPRET
	success("interpret done.");
	#endif
	if (result.type == UNE_RT_ERROR)
		return une_result_create(UNE_RT_ERROR);

	une_context_free_children(parent, felix->is.context);
	felix->is.context = parent;
	
	return result;
}

une_result une_engine_interpret_file_or_wcs(char *path, wchar_t *wcs)
{
	return une_engine_interpret_file_or_wcs_with_position(path, wcs, (une_position){0});
}

void une_engine_print_error(void)
{
	assert(felix->error.type != UNE_ET_none__);
	wprintf(L"[91mError: %ls\n[0m", une_error_type_to_wcs(felix->error.type));
}
