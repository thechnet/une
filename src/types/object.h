/*
object.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TYPES_OBJECT_H
#define UNE_TYPES_OBJECT_H

/* Header-specific includes. */
#include "../common.h"
#include "../struct/result.h"
#include "../struct/association.h"
#include "../struct/context.h"

void une_type_object_represent(FILE *file, une_result result);

une_int une_type_object_is_true(une_result result);
une_int une_type_object_is_equal(une_result subject, une_result comparison);

bool une_type_object_is_valid_element(une_result element);

bool une_type_object_member_exists(une_result subject, wchar_t *name);
une_result une_type_object_refer_to_member(une_result subject, wchar_t *name);

une_result une_type_object_copy(une_result result);
void une_type_object_free_members(une_result result);

/*
Object.
*/

#define UNE_FOR_OBJECT_MEMBER(iterator_, object_ptr_) \
	for (size_t iterator_=0; iterator_<object_ptr_->members_length; iterator_++)

typedef struct une_object_ {
	une_association **members;
	size_t members_length;
	une_context *owner;
} une_object;

#endif /* UNE_TYPES_OBJECT_H */
