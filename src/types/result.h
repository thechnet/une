/*
result.h - Une
Modified 2023-02-11
*/

#ifndef UNE_RESULT_H
#define UNE_RESULT_H

/* Header-specific includes. */
#include "../primitive.h"

/*
Type of une_result.
*/
typedef enum une_result_type_ {
  UNE_RT_none__,
  UNE_RT_ERROR,
  #define UNE_R_BGN_DATA_RESULT_TYPES UNE_RT_VOID
  UNE_RT_VOID,
  UNE_RT_INT,
  UNE_RT_FLT,
  UNE_RT_STR,
  UNE_RT_LIST,
  UNE_RT_OBJECT,
  UNE_RT_FUNCTION,
  UNE_RT_BUILTIN,
  #define UNE_R_END_DATA_RESULT_TYPES UNE_RT_BUILTIN
  UNE_RT_CONTINUE,
  UNE_RT_BREAK,
  UNE_RT_SIZE,
  UNE_RT_GENERIC_REFERENCE,
  UNE_RT_STR_ELEMENT_REFERENCE,
  UNE_RT_max__,
} une_result_type;

/*
Holds data resulting from interpretation.
*/
typedef struct une_result_ {
  une_result_type type;
  une_value value;
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
Condition to check whether une_result_type is valid.
*/
#define UNE_RESULT_TYPE_IS_VALID(type)\
  (type > UNE_RT_none__ && type < UNE_RT_max__)

/*
Condition to check whether une_result_type is data type.
*/
#define UNE_RESULT_TYPE_IS_DATA_TYPE(type)\
  (type >= UNE_R_BGN_DATA_RESULT_TYPES && type <= UNE_R_END_DATA_RESULT_TYPES)

une_result une_result_create(une_result_type type);
une_result une_result_copy(une_result original);
void une_result_free(une_result result);

une_result *une_result_list_create(size_t size);

#ifdef UNE_DEBUG
const wchar_t *une_result_type_to_wcs(une_result_type result_type);
#endif /* UNE_DEBUG */
void une_result_represent(FILE *file, une_result result);

une_int une_result_is_true(une_result result);
une_int une_results_are_equal(une_result left, une_result right);

#endif /* !UNE_RESULT_H */
