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
	bool is_file = path != NULL;
	if (is_file) {
		assert(!wcs);
	} else {
		assert(!path);
		assert(wcs);
	}

	char *absolute_path = NULL;
	wchar_t *source = NULL;
	if (is_file) {
		absolute_path = une_resolve_path(path);
		if (!absolute_path || !une_file_exists(absolute_path))
			felix->error = UNE_ERROR_SET(UNE_ET_FILE, (une_position){0});
		else
			source = une_file_read(absolute_path, true, UNE_TAB_WIDTH);
	} else {
		source = wcsdup(wcs);
	}

	une_module *module = une_modules_add_module(&felix->is.modules);
	assert(module);

	module->is_file = is_file;
	module->path = absolute_path;
	module->source = source;

	if (source) {
		une_lexer_state ls = une_lexer_state_create();
		ls.tokens = malloc(UNE_SIZE_TOKEN_BUF*sizeof(*ls.tokens));
		verify(ls.tokens);
		ls.name = module->path;
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
		callable->is_module = true;
	}

	return callable;
}

une_result une_engine_interpret_file_or_wcs_with_position(char *path, wchar_t *wcs, une_position creation_position)
{
	une_engine_prepare_for_next_module();

	une_module *module = une_engine_new_module_from_file_or_wcs(path, wcs);
	if (!module->tokens)
		return une_result_create(UNE_RT_ERROR);

	une_callable *callable = une_engine_parse_module(module);
	if (!callable)
		return une_result_create(UNE_RT_ERROR);

	une_context *parent = felix->is.context;
	felix->is.context = une_context_create_transparent();
	felix->is.context->parent = parent;
	une_callable *parent_callable = une_callables_get_callable_by_id(felix->is.callables, parent->callable_id);
	if (parent_callable) { /* The root context, created as part of the engine, does not have a callable. */
		felix->is.context->creation_module_id = parent_callable->module_id;
		felix->is.context->creation_position = creation_position;
	}
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
