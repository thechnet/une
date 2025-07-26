/*
tools.h - Une
Modified 2025-07-26
*/

#ifndef UNE_TOOLS_H
#define UNE_TOOLS_H

/* Header-specific includes. */
#include "common.h"
#include "struct/result.h"
#include <math.h>

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

/*
une_flt literal wrapper.
*/
#define UNE_NEW_FLT(flt_) flt_ /* flt_##L if une_flt is a long double. */

extern une_flt (*une_flt_nextafter)(une_flt, une_flt);
extern une_flt (*une_flt_pow)(une_flt, une_flt);
extern une_flt (*une_flt_abs)(une_flt);
extern une_flt (*une_flt_floor)(une_flt);
extern une_flt (*une_flt_mod)(une_flt, une_flt);

bool une_wcs_to_une_int(wchar_t *wcs, une_int *dest);
bool une_wcs_to_une_flt(wchar_t *wcs, une_flt *dest);

wchar_t *une_str_to_wcs(char *str);
char *une_wcs_to_str(wchar_t *wcs);

wchar_t *une_flt_to_wcs(une_flt flt);

char *une_resolve_path(char *path);

bool une_file_exists(char *path);
bool une_file_or_folder_exists(char *path);
wchar_t *une_file_read(char *path, bool normalize_line_endings, size_t convert_tabs_to_spaces);
bool une_file_extension_matches(char *path, char *extension);

wchar_t *une_get_working_directory(void);
bool une_set_working_directory(wchar_t *path);

void une_sleep_ms(int ms);

bool une_play_wav(wchar_t *path);

int une_out_of_memory(void);

bool une_flts_equal(une_flt a, une_flt b);

une_int une_min(une_int num, une_int min);
une_int une_max(une_int num, une_int max);
une_int une_clamp(une_int num, une_int min, une_int max);

une_range une_range_from_relative_index(une_result index, size_t scope);
une_range une_range_from_relative_indices(une_result begin, une_result end, size_t scope);

bool une_position_is_valid(une_position position);
une_position une_position_between(une_position first, une_position last);
une_position une_position_set_start(une_position subject, une_position begin);

size_t une_wcs_find_start_of_current_line(wchar_t *wcs, size_t starting_index);
size_t une_wcs_find_end_of_current_line(wchar_t *wcs, size_t starting_index);
size_t une_wcs_skip_whitespace(wchar_t *wcs, size_t starting_index);

#ifdef _WIN32
void une_win_vt_proc(bool enable);
#endif

#endif /* !UNE_TOOLS_H */
