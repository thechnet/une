/*
interpreter_state.c - Une
Modified 2023-05-13
*/

/* Header-specific includes. */
#include "interpreter_state.h"
#include "../tools.h"

/* Implementation-specific includes. */
#include "../datatypes/object.h"

une_interpreter_state *une_is;

/*
Initialize a une_interpreter_state struct.
*/
une_interpreter_state une_interpreter_state_create(void)
{
  return (une_interpreter_state){
    .context = une_context_create(NULL, (une_position){0}),
    .should_return = false,
    .should_exit = false,
    .this_contestant = une_result_create(UNE_RT_VOID),
    .this = une_result_create(UNE_RT_VOID),
    .holding = (une_holding){
      .buffer = NULL,
      .size = 0,
      .count = 0
    }
  };
}

/*
Free all members of a une_interpreter_state struct.
*/
void une_interpreter_state_free(une_interpreter_state *is)
{
  /* Context. */
  une_context_free_children(NULL, is->context);
  
  /* 'this' contestant. */
  une_result_free(is->this_contestant);
}

/*
Strip the current holding, replacing it with a new one.
*/
une_holding une_interpreter_state_holding_strip(une_interpreter_state *is)
{
  /* Preserve current holding. */
  une_holding old = is->holding;
  
  /* Replace holding with new one. */
  une_result *buffer = malloc(UNE_SIZE_HOLDING*sizeof(*buffer));
  verify(buffer);
  for (size_t i=0; i<UNE_SIZE_HOLDING; i++)
    buffer[i] = une_result_create(UNE_RT_none__);
  is->holding = (une_holding){
    .buffer = buffer,
    .size = UNE_SIZE_HOLDING,
    .count = 0
  };
  
  /* Return old holding. */
  return old;
}

/*
Drop all held results.
*/
void une_interpreter_state_holding_reinstate(une_interpreter_state *is, une_holding old)
{
  /* Empty current holding's buffer. */
  une_interpreter_state_holding_purge(is);
  free(is->holding.buffer);
  
  /* Reinstate old holding. */
  is->holding = old;
}

/*
Hold a result in the interpreter state.
*/
une_result *une_interpreter_state_holding_add(une_interpreter_state *is, une_result result)
{
  /* Ensure buffer is big enough. */
  if (is->holding.count >= is->holding.size) {
    is->holding.size *= 2;
    is->holding.buffer = realloc(is->holding.buffer, is->holding.size*sizeof(*is->holding.buffer));
    verify(is->holding.buffer);
    for (size_t i=is->holding.count; i<is->holding.size; i++)
      is->holding.buffer[i] = une_result_create(UNE_RT_none__);
  }
  
  /* Hold result. */
  is->holding.buffer[is->holding.count++] = result;
  
  /* Return pointer to held result. */
  return is->holding.buffer+is->holding.count-1;
}

/*
Free the contents of a holding, without freeing the holding itself.
*/
void une_interpreter_state_holding_purge(une_interpreter_state *is)
{
  for (size_t i=0; i<is->holding.count; i++) {
    une_result_free(is->holding.buffer[i]);
    is->holding.buffer[i] = une_result_create(UNE_RT_none__);
  }
  is->holding.count = 0;
}

/*
Check if a result matches the interpreter state's 'this'.
*/
bool une_result_is_reference_to_foreign_object(une_interpreter_state *is, une_result subject)
{
  if (subject.type != UNE_RT_REFERENCE || subject.reference.type != UNE_FT_SINGLE)
    return false;
  une_result *referenced = (une_result*)subject.reference.root;
  assert(referenced);
  if (referenced->type != UNE_RT_OBJECT)
    return false;
  une_object *object = (une_object*)referenced->value._vp;
  assert(object);
  return object->owner != is->context;
}
