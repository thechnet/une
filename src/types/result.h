/*
result.h - Une
Modified 2021-07-05
*/

#ifndef UNE_RESULT_H
#define UNE_RESULT_H

/* Header-specific includes. */
#include "../primitive.h"

/*
Type of une_result.
*/
typedef enum _une_result_type {
  __UNE_RT_none__,
  #define UNE_R_BGN_MARKER_RESULT_TYPES UNE_RT_VOID
  UNE_RT_VOID,
  UNE_RT_ERROR,
  UNE_RT_CONTINUE,
  UNE_RT_BREAK,
  #define UNE_R_END_MARKER_RESULT_TYPES UNE_RT_BREAK
  UNE_RT_SIZE,
  #define UNE_R_BGN_DATA_RESULT_TYPES UNE_RT_INT
  UNE_RT_INT,
  UNE_RT_FLT,
  UNE_RT_STR,
  UNE_RT_LIST,
  #define UNE_R_END_DATA_RESULT_TYPES UNE_RT_LIST
  __UNE_RT_max__,
} une_result_type;

/*
Holds data resulting from interpretation.
*/
typedef struct _une_result {
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
  size_t listsize = listname[0].value._int

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
  (type > __UNE_RT_none__ && type < __UNE_RT_max__)

/*
Condition to check whether une_result_type is data type.
*/
#define UNE_RESULT_TYPE_IS_DATA_TYPE(type)\
  (type >= UNE_R_BGN_DATA_RESULT_TYPES && type <= UNE_R_END_DATA_RESULT_TYPES)

/*
Verify une_result_type.
*/
#define UNE_VERIFY_RESULT_TYPE(type)\
  if (!UNE_RESULT_TYPE_IS_VALID(type))\
    ERR(L"Invalid une_result_type %lld.", type);

une_result une_result_create(une_result_type type);
une_result une_result_copy(une_result src);
void une_result_free(une_result result);

une_result *une_result_list_create(size_t size);

const wchar_t *une_result_type_to_wcs(une_result_type result_type);
void une_result_represent(une_result result);

une_int une_result_is_true(une_result result);
une_int une_results_are_equal(une_result left, une_result right);

une_result une_result_lists_add(une_result left, une_result right);
une_result une_result_strs_add (une_result left, une_result right);

une_result une_result_list_mul(une_result list, une_int count);
une_result une_result_str_mul (une_result str, une_int count);

#endif /* !UNE_RESULT_H */
