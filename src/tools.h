/*
tools.h - Une
Modified 2021-07-15
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

/* Header-specific includes. */
#include "primitive.h"

/*
*** Interface.
*/

bool une_wcs_to_une_int(wchar_t *wcs, une_int *dest);
bool une_wcs_to_une_flt(wchar_t *wcs, une_flt *dest);

wchar_t *une_str_to_wcs(char *str);
char *une_wcs_to_str(wchar_t *wcs);

bool une_file_exists(char *path);
bool une_file_or_folder_exists(char *path);
wchar_t *une_file_read(char *path);

#endif /* !UNE_TOOLS_H */
