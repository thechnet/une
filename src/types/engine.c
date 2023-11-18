/*
engine.c - Une
Modified 2023-11-18
*/

/* Header-specific includes. */
#include "engine.h"

/* Implementation-specific includes. */
#include "../tools.h"
#include "../lexer.h"
#include "../parser.h"
#include "../interpreter.h"

/*
API.
*/

une_engine une_engine_create(void)
{
	return (une_engine){
		.error = une_error_create(),
		.is = une_interpreter_state_create(NULL)
	};
}

une_module *une_engine_new_module_from_file_or_wcs(une_engine *engine, char *path, wchar_t *wcs)
{
	assert(engine);
	bool is_file = path != NULL;
	if (is_file) {
		assert(!wcs);
	} else {
		assert(!path);
		assert(wcs);
	}

	char *absolute_path = NULL;
	wchar_t *source = wcs;
	if (is_file) {
		absolute_path = une_resolve_path(path);
		if (!absolute_path || !une_file_exists(absolute_path))
			engine->error = UNE_ERROR_SET(UNE_ET_FILE, (une_position){0});
		source = une_file_read(absolute_path);
	}

	une_module *module = une_modules_add_module(&engine->is.modules);
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
		une_lex(&engine->error, &ls);
		#endif
		module->tokens = ls.tokens;
	}

	#if defined(UNE_DEBUG) && defined(UNE_DISPLAY_TOKENS)
	une_tokens_display(module->tokens);
	wprintf(L"\n\n");
	#endif

	return module;
}

une_callable *une_engine_parse_module(une_engine *engine, une_module *module)
{
	assert(engine);
	assert(module);

	assert(module->tokens);

	une_parser_state ps = une_parser_state_create();
	une_node *ast = NULL;
	#ifndef UNE_NO_PARSE
	ast = une_parse(&engine->error, &ps, module->tokens);
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
		callable = une_callables_add_callable(&engine->is.callables);
		assert(callable);

		callable->module_id = module->id;
		callable->definition_file = module->path;
		callable->definition_point = (une_position){0};
		callable->params_count = 0;
		callable->params = NULL;
		callable->body = ast;
	}

	return callable;
}

une_result une_engine_interpret_file_or_wcs(une_engine *engine, char *path, wchar_t *wcs)
{
	une_module *module = une_engine_new_module_from_file_or_wcs(engine, path, wcs);
	if (!module->tokens)
		return une_result_create(UNE_RT_ERROR);

	une_callable *callable = une_engine_parse_module(engine, module);
	if (!module)
		return une_result_create(UNE_RT_ERROR);

	une_context *parent = engine->is.context;
	engine->is.context = une_context_create_marker(module->path, (une_position){0}, NULL, NULL, (une_position){0});
	engine->is.context->parent = parent;

	une_interpreter_state_reset_flags(&engine->is);

	une_result result = une_result_create(UNE_RT_VOID);
	#ifndef UNE_NO_INTERPRET
	result = une_interpret(&engine->error, callable->body);
	#endif
	#ifdef UNE_DEBUG_LOG_INTERPRET
	success("interpret done.");
	#endif
	if (result.type == UNE_RT_ERROR)
		return une_result_create(UNE_RT_ERROR);

	une_context_free_children(parent, engine->is.context);
	engine->is.context = parent;
	
	return result;
}

void une_engine_print_error(une_engine *engine)
{
	wprintf(L"Error\n");
}

void une_engine_free(une_engine engine)
{
	une_interpreter_state_free(&engine.is);
}
