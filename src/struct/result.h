/*
result.h - Une
Modified 2023-12-10
*/

#ifndef UNE_RESULT_H
#define UNE_RESULT_H

/* Header-specific includes. */
#include "../common.h"
#include "reference.h"

/*
Kind of une_result.
*/
typedef enum une_result_kind_ {
	UNE_RK_none__,
	UNE_RK_ERROR,
	#define UNE_R_BGN_DATA_RESULT_KINDS UNE_RK_VOID
	UNE_RK_VOID,
	UNE_RK_INT,
	UNE_RK_FLT,
	UNE_RK_STR,
	UNE_RK_LIST,
	UNE_RK_OBJECT,
	UNE_RK_FUNCTION,
	UNE_RK_NATIVE,
	#define UNE_R_END_DATA_RESULT_KINDS UNE_RK_NATIVE
	UNE_RK_CONTINUE,
	UNE_RK_BREAK,
	UNE_RK_SIZE,
	UNE_RK_REFERENCE,
	UNE_RK_max__,
} une_result_kind;

/*
Holds data resulting from interpretation.
*/
typedef struct une_result_ {
	une_result_kind kind;
	union {
		une_value value;
		une_reference reference;
	};
} une_result;

/*
*** Interface.
*/

/*
Unpack a une_result list into its name and size.
*/
#define UNE_UNPACK_RESULT_LIST(listresult, listname, listsize)\
	une_result *listname = (une_result*)listresult.value._vp;\
	assert(listname != NULL);\
	size_t listsize = (size_t)listname[0].value._int

/*
Unpack a une_result string into its string pointer and size.
*/
#define UNE_UNPACK_RESULT_STR(strresult, strname, strsize)\
	wchar_t *strname = strresult.value._wcs;\
	size_t strsize = wcslen(strname)

/*
Iterate over every item in a une_result list.
*/
#define UNE_FOR_RESULT_LIST_ITEM(var, size)\
	for (size_t var=1; var<=size; var++)

/*
Iterate over every index in a une_result list.
*/
#define UNE_FOR_RESULT_LIST_INDEX(var, size)\
	for (size_t var=0; var<=size; var++)

/*
Condition to check whether une_result_kind is valid.
*/
#define UNE_RESULT_KIND_IS_VALID(kind)\
	(kind > UNE_RK_none__ && kind < UNE_RK_max__)

/*
Condition to check whether une_result_kind is data kind.
*/
#define UNE_RESULT_KIND_IS_TYPE(kind)\
	(kind >= UNE_R_BGN_DATA_RESULT_KINDS && kind <= UNE_R_END_DATA_RESULT_KINDS)

une_result une_result_create(une_result_kind kind);
une_result une_result_copy(une_result original);
void une_result_free(une_result result);

une_result *une_result_list_create(size_t size);

#ifdef UNE_DEBUG
const wchar_t *une_result_kind_to_wcs(une_result_kind result_kind);
#endif /* UNE_DEBUG */
void une_result_represent(FILE *file, une_result result);

une_int une_result_is_true(une_result result);
une_int une_result_equ_result(une_result left, une_result right);
une_int une_result_neq_result(une_result left, une_result right);
une_int une_result_gtr_result(une_result left, une_result right);
une_int une_result_geq_result(une_result left, une_result right);
une_int une_result_lss_result(une_result left, une_result right);
une_int une_result_leq_result(une_result left, une_result right);

bool une_result_equals_result(une_result left, une_result right);

une_result une_result_dereference(une_result result);

une_result une_result_wrap_in_list(une_result result);

#endif /* !UNE_RESULT_H */
