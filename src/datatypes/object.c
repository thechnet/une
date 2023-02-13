/*
object.c - Une
Modified 2023-02-12
*/

/* Header-specific includes. */
#include "object.h"

/* Implementation-specific includes. */
#include "../types/symbols.h"
#include "../tools.h"

/*
Print a text representation to file.
*/
void une_datatype_object_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_OBJECT);
  
  /* Get associations. */
  une_variable *associations = (une_variable*)result.value._vp;
  
  /* Get size. */
  assert(associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  fwprintf(file, L"{");
  for (size_t i=1; i<=size; i++) {
    if (!associations[i].name)
      continue;
    fwprintf(file, L"%ls: ", associations[i].name);
    if (associations[i].content.type == UNE_RT_STR)
      putwc(L'"', file);
    une_result_represent(file, associations[i].content);
    if (associations[i].content.type == UNE_RT_STR)
      putwc(L'"', file);
    if (i < size)
      fwprintf(file, L", ");
  }
  putwc(L'}', file);
}

/*
Check for truth.
*/
une_int une_datatype_object_is_true(une_result result)
{
  assert(result.type == UNE_RT_OBJECT);
  return ((une_variable*)result.value._vp)[0].content.value._int == 0 ? 0 : 1;
}

/*
Check if subject is equal to comparison.
*/
une_int une_datatype_object_is_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_OBJECT);
  if (comparison.type != UNE_RT_OBJECT)
    return 0;
  
  /* Get associations. */
  une_variable *subject_associations = (une_variable*)subject.value._vp;
  une_variable *comparison_associations = (une_variable*)comparison.value._vp;
  
  /* Get size. */
  assert(subject_associations[0].content.type == UNE_RT_SIZE);
  assert(comparison_associations[0].content.type == UNE_RT_SIZE);
  size_t subject_size = (size_t)subject_associations[0].content.value._int;
  size_t comparison_size = (size_t)comparison_associations[0].content.value._int;
  if (subject_size != comparison_size)
    return 0;
  
  /* Compare associations. */
  for (size_t i=1; i<=subject_size; i++) {
    wchar_t *name = subject_associations[i].name;
    if (!name)
      continue;
    if (!une_datatype_object_member_exists(comparison, name))
      return 0;
    une_result member = une_datatype_object_get_member(comparison, name);
    if (!une_results_are_equal(subject_associations[i].content, member))
      return 0;
  }
  return 1;
}

/*
Check if an element is valid.
*/
bool une_datatype_object_is_valid_element(une_result element)
{
  assert(UNE_RESULT_TYPE_IS_DATA_TYPE(element.type));
  return 1;
}

/*
Check if a member exists.
*/
bool une_datatype_object_member_exists(une_result target, wchar_t *member)
{
  assert(target.type == UNE_RT_OBJECT);
  
  /* Get associations. */
  une_variable *associations = (une_variable*)target.value._vp;
  
  /* Get size. */
  assert(!associations[0].name && associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  /* Check members. */
  for (size_t i=1; i<=size; i++)
    if (!wcscmp(associations[i].name, member))
      return 1;
  return 0;
}

/*
Add a member.
*/
une_result une_datatype_object_add_member(une_result *target, wchar_t *member)
{
  assert(target->type == UNE_RT_OBJECT);
  
  /* Get associations. */
  une_variable *associations = (une_variable*)target->value._vp;
  
  /* Get size. */
  assert(!associations[0].name && associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  /* Find or create slot. */
  size_t slot = 0;
  for (size_t i=1; i<=size; i++)
    if (!associations[i].name) {
      slot = i;
      break;
    }
  if (!slot) {
    slot = ++size;
    associations = realloc(associations, (size+1)*sizeof(*associations));
    verify(associations);
    associations[slot].name = wcsdup(member);
    verify(associations[slot].name);
    for (size_t i=slot; i<=size; i++) {
      associations[i].name = NULL;
      associations[i].content = une_result_create(UNE_RT_VOID);
    }
    assert(associations[0].content.type == UNE_RT_SIZE);
    associations[0].content.value._int = (une_int)size;
  }
  
  /* Return reference. */
  une_result reference = une_result_create(UNE_RT_GENERIC_REFERENCE);
  reference.value._vp = (void*)(&(associations[slot].content));
  return reference;
}

/*
Get the value at member.
*/
une_result une_datatype_object_get_member(une_result target, wchar_t *member)
{
  assert(target.type == UNE_RT_OBJECT);
  
  /* Get associations. */
  une_variable *associations = (une_variable*)target.value._vp;
  
  /* Get size. */
  assert(!associations[0].name && associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  /* Check members. */
  une_result result = une_result_create(UNE_RT_none__);
  for (size_t i=1; i<=size; i++)
    if (!wcscmp(associations[i].name, member)) {
      assert(result.type == UNE_RT_none__);
      result = une_result_copy(associations[i].content);
    }
  return result;
}

/*
Seek the value at member.
*/
une_result une_datatype_object_seek_member(une_result *target, wchar_t *member)
{
  assert(target->type == UNE_RT_OBJECT);
  
  /* Get associations. */
  une_variable *associations = (une_variable*)target->value._vp;
  
  /* Get size. */
  assert(!associations[0].name && associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  /* Check members. */
  une_result reference = une_result_create(UNE_RT_GENERIC_REFERENCE);
  for (size_t i=1; i<=size; i++)
    if (!wcscmp(associations[i].name, member)) {
      assert(!reference.value._vp);
      reference.value._vp = (void*)&associations[i].content;
    }
  return reference;
}

/*
Create a duplicate.
*/
une_result une_datatype_object_copy(une_result original)
{
  assert(original.type == UNE_RT_OBJECT);
  
  /* Get associations. */
  une_variable *associations = (une_variable*)original.value._vp;
  
  /* Get size. */
  assert(!associations[0].name && associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  une_variable *associations_copy = malloc((size+1)*sizeof(*associations_copy));
  verify(associations_copy);
  for (size_t i=0; i<=size; i++) {
    if (associations[i].name) {
      associations_copy[i].name = wcsdup(associations[i].name);
      verify(associations_copy[i].name);
    } else {
      associations_copy[i].name = NULL;
    }
    associations_copy[i].content = une_result_copy(associations[i].content);
  }
  une_result copy = une_result_create(UNE_RT_OBJECT);
  copy.value._vp = (void*)associations_copy;
  return copy;
}

/*
Free all members.
*/
void une_datatype_object_free_members(une_result result)
{
  assert(result.type == UNE_RT_OBJECT);
  
  une_variable *associations = (une_variable*)result.value._vp;
  
  assert(associations[0].content.type == UNE_RT_SIZE);
  size_t size = (size_t)associations[0].content.value._int;
  
  for (size_t i=0; i<=size; i++) {
    if (associations[i].name)
      free(associations[i].name);
    une_result_free(associations[i].content);
  }
  
  free(associations);
}
