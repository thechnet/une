/*
modules.c - Une
Modified 2023-11-18
*/

/* Header-specific includes. */
#include "modules.h"

/* Implementation-specific includes. */
#include "../tools.h"

static void une_module_clear(une_module *module)
{
	assert(module);

	if (module->id == 0)
		return;
	
	if (module->path)
		free(module->path);
	
	if (module->source)
		free(module->source);
	
	if (module->tokens)
		une_tokens_free(module->tokens);
	
	module->id = 0;
}

une_modules une_modules_create(void)
{
	size_t size = UNE_SIZE_MODULES;

	une_module *buffer = malloc(size*sizeof(*buffer));
	verify(buffer);
	for (size_t i=0; i<size; i++)
		buffer[i] = (une_module){ .id = 0 };
	
	return (une_modules){
		.size = size,
		.buffer = buffer,
		.next_unused_id = 0
	};
}

une_module *une_modules_add_module(une_modules *modules)
{	
	assert(modules);
	assert(modules->buffer);
	
	size_t index;
	for (index=0; index<modules->size; index++) {
		if (modules->buffer[index].id == 0)
			break;
	}
	if (index == modules->size) {
		modules->size *= 2;
		modules->buffer = realloc(modules->buffer, modules->size*sizeof(*modules->buffer));
		verify(modules->buffer);
		for (size_t i=index; i<modules->size; i++)
			modules->buffer[i] = (une_module){ .id = 0 };
	}
	
	modules->buffer[index] = (une_module){
		.id = modules->next_unused_id++
	};

	return modules->buffer+index;
}

une_module *une_modules_get_module_by_id(une_modules modules, size_t id)
{
	assert(modules.buffer);

	for (size_t i=0; i<modules.size; i++) {
		if (modules.buffer[i].id == id)
			return modules.buffer+i;
	}

	return NULL;
}

void une_modules_free(une_modules *modules)
{
	assert(modules);

	assert(modules->buffer);
	for (size_t i=0; i<modules->size; i++)
		une_module_clear(modules->buffer+i);
	free(modules->buffer);

	modules->buffer = NULL;
	modules->size = 0;
	modules->next_unused_id = 0;
}
