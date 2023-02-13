/*
object.h - Une
Modified 2023-02-13
*/

#ifndef UNE_DATATYPES_OBJECT_H
#define UNE_DATATYPES_OBJECT_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"
#include "../types/symbols.h"

void une_datatype_object_represent(FILE *file, une_result result);

une_int une_datatype_object_is_true(une_result result);
une_int une_datatype_object_is_equal(une_result subject, une_result comparison);

bool une_datatype_object_is_valid_element(une_result element);

bool une_datatype_object_member_exists(une_result target, wchar_t *member);
une_result une_datatype_object_add_member(une_result *target, wchar_t *member);
une_result une_datatype_object_get_member(une_result target, wchar_t *member);
une_result une_datatype_object_seek_member(une_result *target, wchar_t *member);

une_result une_datatype_object_copy(une_result result);
void une_datatype_object_free_members(une_result result);

/*
Object.
*/

#define UNE_FOR_OBJECT_MEMBER(iterator_, object_ptr_) \
  for (size_t iterator_=0; iterator_<object_ptr_->members_length; iterator_++)

typedef struct une_object_ {
  une_association **members;
  size_t members_length;
} une_object;

#endif /* UNE_DATATYPES_OBJECT_H */
