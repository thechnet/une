/*
tools.h - Une
Updated 2021-06-13
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

/* Header-specific includes. */
#include "primitive.h"

/*
*** Interface.
*/

void *une_malloc(size_t size);
void *une_realloc(void *memory, size_t new_size);
void une_free(void *memory);

une_int wcs_to_une_int(wchar_t *str);
une_flt wcs_to_une_flt(wchar_t *str);

wchar_t *str_to_wcs(char *str);

char    *str_dup(char    *src);
wchar_t *wcs_dup(wchar_t *src);

#endif /* !UNE_TOOLS_H */
