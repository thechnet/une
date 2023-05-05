/*
reference.h - Une
Modified 2023-05-05
*/

#ifndef UNE_REFERENCE_H
#define UNE_REFERENCE_H

/* Header-specific includes. */
#include "../primitive.h"

/*
Type of une_reference.
*/
typedef enum une_reference_type_ {
  UNE_FT_none__,
  UNE_FT_SINGLE,
  UNE_FT_LISTVIEW,
  UNE_FT_STRVIEW,
  UNE_FT_max__,
} une_reference_type;

/*
Refers to data.
*/
typedef struct une_reference_ {
  une_reference_type type;
  void *root;
  size_t width;
} une_reference;

/*
*** Interface.
*/

/*
Condition to check whether une_reference_type is valid.
*/
#define UNE_REFERENCE_TYPE_IS_VALID(type)\
  (type > UNE_FT_none__ && type < UNE_FT_max__)

#endif /* !UNE_REFERENCE_H */
