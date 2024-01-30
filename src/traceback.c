/*
traceback.c - Une
Modified 2024-01-30
*/

/* Header-specific includes. */
#include "traceback.h"

/* Implementation-specific includes. */
#include "struct/context.h"
#include "struct/error.h"
#include "struct/engine.h"
#include "tools.h"
#include "struct/callable.h"

/*
Print a trace.
*/
void une_traceback_print_trace(une_context *context)
{
	assert(context);
	une_module *module = une_modules_get_module_by_id(felix->is.modules, context->module_id);
	assert(module);

	/* File not found. */
	if (!module->source)
		return;
	
	une_position position = context->exit_position;
	if (!une_position_is_valid(position))
		position = felix->error.pos;

	/* Print position. */

	char *module_name = module->path;
	if (!module_name)
		module_name = UNE_MODULE_NAME_PLACEHOLDER;

	fwprintf(UNE_ERROR_STREAM, UNE_COLOR_TRACEBACK_LOCATION L"File \"%hs\", line %zu", module_name, position.line);
	if (context->label)
		fwprintf(UNE_ERROR_STREAM, L", in %ls", context->label);
	fwprintf(UNE_ERROR_STREAM, ":\n" UNE_COLOR_RESET);

	/* Print extract. */

	#if defined(UNE_DEBUG) && defined(UNE_DBG_DISPLAY_EXTENDED_ERROR)
	fwprintf(UNE_ERROR_STREAM, UNE_COLOR_HINT L"    Position: %zu-%zu" UNE_COLOR_RESET L"\n", position.start, position.end);
	#endif

	size_t index = une_wcs_find_start_of_current_line(module->source, position.start);
	index = une_wcs_skip_whitespace(module->source, index);
	
	while (index < position.end) {
		size_t length_of_line = une_wcs_find_end_of_current_line(module->source, index) - index;

		fwprintf(UNE_ERROR_STREAM, UNE_COLOR_RESET UNE_TRACEBACK_EXTRACT_PREFIX L"%.*ls\n", length_of_line, module->source+index);
		
		size_t underline_offset = position.start > index ? position.start-index : 0;
		size_t underline_length = (position.end < index + length_of_line ? position.end-index : length_of_line) - underline_offset;
		if (
			index + underline_offset + underline_length == position.end - 1 && /* Underline ends on this line. */
			(module->source[position.end - 1] == L'\n' || module->source[position.end - 1] == L'\0') /* Position ends on a newline or the end of the source. */
		)
			underline_length++;
		
		fputws(UNE_COLOR_POSITION UNE_TRACEBACK_EXTRACT_PREFIX, UNE_ERROR_STREAM);
		for (size_t i=0; i<underline_offset; i++)
			fputwc(L' ', UNE_ERROR_STREAM);
		for (size_t i=0; i<underline_length; i++)
			fputwc(L'~', UNE_ERROR_STREAM);
		fputws(UNE_COLOR_RESET "\n", UNE_ERROR_STREAM);
		index += length_of_line + 1;
	}
}

/*
Print a traceback.
*/
void une_traceback_print(void)
{
	une_context **lineage;
	size_t lineage_length = une_context_get_lineage(felix->is.context, &lineage);

	for (size_t i=0; i<lineage_length; i++) {
		une_context *context = lineage[lineage_length-1-i];

		#if defined(UNE_DEBUG) && defined(UNE_DBG_DISPLAY_EXTENDED_ERROR)
		fwprintf(UNE_ERROR_STREAM, UNE_COLOR_HINT L"--- Generation: %zu" UNE_COLOR_RESET L"\n", i);
		if (context->is_transparent)
			fputws(UNE_COLOR_HINT L"    Transparent" UNE_COLOR_RESET L"\n", UNE_ERROR_STREAM);
		#endif

		if (context->module_id) /* The only context without a module is the root context. */
			une_traceback_print_trace(context);
		#if defined(UNE_DEBUG) && defined(UNE_DBG_DISPLAY_EXTENDED_ERROR)
		else {
			fputws(UNE_COLOR_HINT L"    Root context (no callable)" UNE_COLOR_RESET L"\n", UNE_ERROR_STREAM);
		}
		#endif
	}

	free(lineage);
}
