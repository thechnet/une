/*
reference.h - Une
Modified 2023-12-10
*/

#ifndef UNE_REFERENCE_H
#define UNE_REFERENCE_H

/* Header-specific includes. */
#include "../common.h"

/*
Kind of une_reference.
*/
typedef enum une_reference_kind_ {
	UNE_FK_none__,
	UNE_FK_SINGLE,
	UNE_FK_LISTVIEW,
	UNE_FK_STRVIEW,
	UNE_FK_max__,
} une_reference_kind;

/*
Refers to data.
*/
typedef struct une_reference_ {
	une_reference_kind kind;
	void *root;
	size_t width;
} une_reference;

/*
*** Interface.
*/

/*
Condition to check whether une_reference_kind is valid.
*/
#define UNE_REFERENCE_KIND_IS_VALID(kind)\
	(kind > UNE_FK_none__ && kind < UNE_FK_max__)

#endif /* !UNE_REFERENCE_H */
