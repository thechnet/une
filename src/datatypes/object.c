/*
object.c - Une
Modified 2023-02-22
*/

/* Header-specific includes. */
#include "object.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
Print a text representation to file.
*/
void une_datatype_object_represent(FILE *file, une_result result)
{
  assert(result.type == UNE_RT_OBJECT);
  
  /* Extract object struct. */
  une_object *object = (une_object*)result.value._vp;
  
  fwprintf(file, L"{");
  UNE_FOR_OBJECT_MEMBER(i, object) {
    if (!object->members[i]->name)
      continue;
    fwprintf(file, L"%ls: ", object->members[i]->name);
    if (object->members[i]->content.type == UNE_RT_STR)
      putwc(L'"', file);
    une_result_represent(file, object->members[i]->content);
    if (object->members[i]->content.type == UNE_RT_STR)
      putwc(L'"', file);
    if (i < object->members_length-1)
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
  
  /* Extract object struct. */
  une_object *object = (une_object*)result.value._vp;
  
  UNE_FOR_OBJECT_MEMBER(i, object)
    if (object->members[0]->name)
      return 1;
  return 0;
}

/*
Check if subject is equal to comparison.
*/
une_int une_datatype_object_is_equal(une_result subject, une_result comparison)
{
  assert(subject.type == UNE_RT_OBJECT);
  if (comparison.type != UNE_RT_OBJECT)
    return 0;
  
  /* Extract object structs. */
  une_object *subject_object = (une_object*)subject.value._vp;
  une_object *comparison_object = (une_object*)comparison.value._vp;
  
  /* Compare sizes. */
  if (subject_object->members_length != comparison_object->members_length)
    return 0;
  
  /* Compare associations. */
  UNE_FOR_OBJECT_MEMBER(i, subject_object) {
    wchar_t *name = subject_object->members[i]->name;
    if (!name)
      continue;
    if (!une_datatype_object_member_exists(comparison, name))
      return 0;
    une_result member = une_datatype_object_get_member(comparison, name);
    if (!une_results_are_equal(subject_object->members[i]->content, member))
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
  
  /* Extract object struct. */
  une_object *object = (une_object*)target.value._vp;
  
  /* Check members. */
  UNE_FOR_OBJECT_MEMBER(i, object)
    if (object->members[i]->name && !wcscmp(object->members[i]->name, member))
      return 1;
  return 0;
}

/*
Add a member.
*/
une_result une_datatype_object_add_member(une_result *target, wchar_t *member)
{
  assert(target->type == UNE_RT_OBJECT);
  
  /* Extract object struct. */
  une_object *object = (une_object*)target->value._vp;
  
  /* Find or create slot. */
  size_t slot = 0;
  UNE_FOR_OBJECT_MEMBER(i, object) {
    if (!object->members[i]->name)
      break;
    slot++;
  }
  if (slot == object->members_length) {
    /* Update length. */
    object->members_length++;
    /* Grow member buffer. */
    object->members = realloc(object->members, (object->members_length+1)*sizeof(*object->members));
    verify(object->members);
    /* Initialize new members. */
    for (size_t i=slot; i<object->members_length; i++)
      object->members[i] = une_association_create();
    /* Name requested member. */
    object->members[slot]->name = wcsdup(member);
    verify(object->members[slot]->name);
  }
  
  /* Return reference. */
  une_result reference = une_result_create(UNE_RT_GENERIC_REFERENCE);
  reference.value._vp = (void*)&object->members[slot]->content;
  return reference;
}

/*
Get the value at member.
*/
une_result une_datatype_object_get_member(une_result target, wchar_t *member)
{
  assert(target.type == UNE_RT_OBJECT);
  
  /* Extract object struct. */
  une_object *object = (une_object*)target.value._vp;
  
  /* Check members. */
  une_result result = une_result_create(UNE_RT_none__);
  UNE_FOR_OBJECT_MEMBER(i, object)
    if (object->members[i]->name && !wcscmp(object->members[i]->name, member)) {
      assert(result.type == UNE_RT_none__);
      result = une_result_copy(object->members[i]->content);
    }
  assert(result.type != UNE_RT_none__);
  return result;
}

/*
Seek the value at member.
*/
une_result une_datatype_object_seek_member(une_result *target, wchar_t *member)
{
  assert(target->type == UNE_RT_OBJECT);
  
  /* Extract object struct. */
  une_object *object = (une_object*)target->value._vp;
  
  /* Check members. */
  une_result reference = une_result_create(UNE_RT_GENERIC_REFERENCE);
  UNE_FOR_OBJECT_MEMBER(i, object)
    if (object->members[i]->name && !wcscmp(object->members[i]->name, member)) {
      assert(!reference.value._vp);
      reference.value._vp = (void*)&object->members[i]->content;
    }
  return reference;
}

/*
Create a duplicate.
*/
une_result une_datatype_object_copy(une_result original)
{
  assert(original.type == UNE_RT_OBJECT);
  
  /* Extract object struct. */
  une_object *original_object = (une_object*)original.value._vp;
  
  une_object *copy_object = malloc(sizeof(*copy_object));
  verify(copy_object);
  copy_object->members_length = original_object->members_length;
  copy_object->members = malloc(copy_object->members_length*sizeof(*copy_object->members));
  verify(copy_object->members);
  UNE_FOR_OBJECT_MEMBER(i, original_object) {
    copy_object->members[i] = malloc(sizeof(*copy_object->members[i]));
    verify(copy_object->members[i]);
    if (original_object->members[i]->name) {
      copy_object->members[i]->name = wcsdup(original_object->members[i]->name);
      verify(copy_object->members[i]->name);
    } else {
      copy_object->members[i]->name = NULL;
    }
    copy_object->members[i]->content = une_result_copy(original_object->members[i]->content);
  }
  une_result copy = une_result_create(UNE_RT_OBJECT);
  copy.value._vp = (void*)copy_object;
  return copy;
}

/*
Free all members.
*/
void une_datatype_object_free_members(une_result result)
{
  assert(result.type == UNE_RT_OBJECT);
  
  /* Extract object struct. */
  une_object *object = (une_object*)result.value._vp;
  
  UNE_FOR_OBJECT_MEMBER(i, object)
    une_association_free(object->members[i]);
  
  free(object->members);
  free(object);
}
