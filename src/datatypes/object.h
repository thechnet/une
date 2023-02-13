/*
object.h - Une
Modified 2023-02-11
*/

#ifndef UNE_DATATYPES_OBJECT_H
#define UNE_DATATYPES_OBJECT_H

/* Header-specific includes. */
#include "../primitive.h"
#include "../types/result.h"

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

#endif /* UNE_DATATYPES_OBJECT_H */
