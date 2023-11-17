/*
association.h - Une
Modified 2023-11-17
*/

#ifndef UNE_ASSOCIATION_H
#define UNE_ASSOCIATION_H

/* Header-specific includes. */
#include "../primitive.h"
#include "result.h"
#include "node.h"

/*
Associate a result with a given name.
*/
typedef struct une_association_ {
	wchar_t *name;
	une_result content;
} une_association;

/*
*** Interface.
*/

une_association *une_association_create(void);
void une_association_free(une_association *association);

#endif /* !UNE_ASSOCIATION_H */
