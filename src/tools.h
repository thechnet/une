/*
tools.h - Une
Modified 2021-07-08
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

/* Header-specific includes. */
#include "primitive.h"

/*
*** Interface.
*/

une_int une_wcs_to_une_int(wchar_t *str);
une_flt une_wcs_to_une_flt(wchar_t *str);

wchar_t *une_str_to_wcs(char *str);
char *une_wcs_to_str(wchar_t *wcs);

char    *une_strdup(char    *src);
wchar_t *une_wcsdup(wchar_t *src);

wchar_t *une_file_read(char *path);

#endif /* !UNE_TOOLS_H */
