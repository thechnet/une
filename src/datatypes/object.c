/*
object.c - Une
Modified 2023-05-08
*/

/* Header-specific includes. */
#include "object.h"

/* Implementation-specific includes. */
#include "../tools.h"

/*
*** Helpers.
*/

static une_object *result_as_object_pointer(une_result subject)
{
  if (subject.type == UNE_RT_OBJECT)
    return (une_object*)subject.value._vp;
  assert(subject.type == UNE_RT_REFERENCE && subject.reference.type == UNE_FT_SINGLE);
  une_result *container = (une_result*)subject.reference.root;
  assert(container->type == UNE_RT_OBJECT);
  return (une_object*)container->value._vp;
}

/*
*** Interface.
*/

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
    une_result member = une_result_dereference(une_datatype_object_refer_to_member(comparison, name));
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
bool une_datatype_object_member_exists(une_result subject, wchar_t *name)
{
  une_object *object = result_as_object_pointer(subject);
  UNE_FOR_OBJECT_MEMBER(i, object)
    if (object->members[i]->name && !wcscmp(object->members[i]->name, name))
      return true;
  return false;
}

/*
Refer to a member.
*/
une_result une_datatype_object_refer_to_member(une_result subject, wchar_t *name)
{
  une_object *object = result_as_object_pointer(subject);
  une_result *root = NULL;
  UNE_FOR_OBJECT_MEMBER(i, object)
    if (object->members[i]->name && !wcscmp(object->members[i]->name, name)) {
      assert(!root);
      root = &object->members[i]->content;
    }
  assert(root);
  return (une_result){
    .type = UNE_RT_REFERENCE,
    .reference = (une_reference){
      .type = UNE_FT_SINGLE,
      .root = root
    }
  };
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
