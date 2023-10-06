/*
tools.h - Une
Modified 2023-10-06
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

/* Header-specific includes. */
#include "primitive.h"
#include "types/result.h"
#include <math.h>

/*
une_flt-compatible INFINITY.
*/
#undef INFINITY
#define INFINITY __builtin_inf()

/*
A range, produced by une_range_from_relative_indices.
*/
typedef struct une_range_ {
  bool valid;
  size_t first;
  size_t guard;
  size_t length;
} une_range;

/*
*** Interface.
*/

/*
Verify an allocation.
*/
#define verify(memory) \
  ((void)((memory) != NULL || une_out_of_memory()))

bool une_wcs_to_une_int(wchar_t *wcs, une_int *dest);
bool une_wcs_to_une_flt(wchar_t *wcs, une_flt *dest);

wchar_t *une_str_to_wcs(char *str);
char *une_wcs_to_str(wchar_t *wcs);

wchar_t *une_flt_to_wcs(une_flt flt);

bool une_file_exists(char *path);
bool une_file_or_folder_exists(char *path);
wchar_t *une_file_read(char *path);

void une_sleep_ms(int ms);

int une_out_of_memory(void);

bool une_flts_equal(une_flt a, une_flt b);

une_int une_min(une_int num, une_int min);
une_int une_max(une_int num, une_int max);
une_int une_clamp(une_int num, une_int min, une_int max);

une_range une_range_from_relative_index(une_result index, size_t scope);
une_range une_range_from_relative_indices(une_result begin, une_result end, size_t scope);

une_position une_position_between(une_position first, une_position last);
une_position une_position_set_start(une_position subject, une_position begin);

#ifdef _WIN32
void une_win_vt_proc(bool enable);
#endif

#endif /* !UNE_TOOLS_H */
