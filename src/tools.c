/*
tools.c - Une
Modified 2025-07-26
*/

/* Header-specific includes. */
#include "tools.h"

/* Implementation-specific includes. */
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#undef PATH_MAX
#define PATH_MAX MAX_PATH /* MinGW defines both of these for compatibility. */
#else
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#endif
#include "lexer.h"

/*
une_flt helpers.
*/
une_flt (*une_flt_nextafter)(une_flt, une_flt) = nextafter;
une_flt (*une_flt_pow)(une_flt, une_flt) = pow;
une_flt (*une_flt_abs)(une_flt) = fabs;
une_flt (*une_flt_floor)(une_flt) = floor;
une_flt (*une_flt_mod)(une_flt, une_flt) = fmod;

/*
Convert a wchar_t string into a une_int integer.
*/
bool une_wcs_to_une_int(wchar_t *wcs, une_int *dest)
{
	assert(wcs);
	une_lexer_state ls = une_lexer_state_create();
	ls.text = wcs;
	ls.text_length = wcslen(ls.text);
	une_error error = une_error_create();
	une_token token = une_lex_number(&error, &ls, true);
	if (token.kind != UNE_TK_INT)
		return false;
	*dest = token.value._int;
	return true;
}

/*
Convert a wchar_t string into a une_flt floating pointer number.
*/
bool une_wcs_to_une_flt(wchar_t *wcs, une_flt *dest)
{
	assert(wcs);
	une_lexer_state ls = une_lexer_state_create();
	ls.text = wcs;
	ls.text_length = wcslen(ls.text);
	une_error error = une_error_create();
	une_token token = une_lex_number(&error, &ls, true);
	if (token.kind == UNE_TK_FLT)
		*dest = token.value._flt;
	else if (token.kind == UNE_TK_INT)
		*dest = (une_flt)token.value._int;
	else
		return false;
	return true;
}

/*
Create a wchar_t string from a char string.
*/
wchar_t *une_str_to_wcs(char *str)
{
	size_t size = strlen(str)+1 /* NUL. */;
	wchar_t *wcs = malloc(size*sizeof(*wcs));
	verify(wcs);
	mbstowcs(wcs, str, size);
	if (errno == EILSEQ) {
		free(wcs);
		return NULL;
	}
	return wcs;
}

/*
Create a char string from a wchar_t string.
*/
char *une_wcs_to_str(wchar_t *wcs)
{
	size_t size = wcslen(wcs)+1 /* NUL. */;
	char *str = malloc(size*sizeof(*str));
	verify(str);
	wcstombs(str, wcs, size);
	if (errno == EILSEQ) {
		free(str);
		return NULL;
	}
	return str;
}

/*
Create a wchar_t string representation of a une_flt.
*/
wchar_t *une_flt_to_wcs(une_flt flt)
{
	/* Try to strip imprecise digits. */
	wchar_t *wcs = malloc(UNE_SIZE_NUMBER_AS_STRING*sizeof(*wcs));
	verify(wcs);
	swprintf(wcs, UNE_SIZE_NUMBER_AS_STRING, UNE_PRINTF_UNE_FLT, UNE_FLT_PRECISION, flt);
	
	/* Strip trailing zeros. */
	size_t wcs_length = wcslen(wcs);
	for (size_t i=wcs_length-1; i>wcs_length-UNE_FLT_PRECISION; i--) {
		if (wcs[i] != L'0')
			break;
		wcs[i] = L'\0';
	}
	
	return wcs;
}

/*
Get the absolute path.
*/
char *une_resolve_path(char *path)
{
	size_t size = PATH_MAX;
	char *resolved_path = malloc(size * sizeof(*resolved_path));
	verify(resolved_path);
	
	#ifdef _WIN32
	DWORD count;
	while (true) {
		count = GetFullPathNameA(path, (DWORD)size, resolved_path, NULL);
		if (count == 0) {
			free(resolved_path);
			return NULL;
		}
		if (count < size)
			break;
		size = count + 1 /* Just to be sure... */;
		resolved_path = realloc(resolved_path, size * sizeof(*resolved_path));
		verify(resolved_path);
	}
	
	#else
	
	if (!realpath(path, resolved_path)) {
		free(resolved_path);
		return NULL;
	}
	#endif
	
	return resolved_path;
}

/*
Check if a file exists.
*/
bool une_file_exists(char *path)
{
	#ifdef _WIN32
	DWORD attribs = GetFileAttributesA(path);
	return attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY);
	#else
	struct stat path_stat;
	int does_not_exist = stat(path, &path_stat);
	int is_directory = S_ISDIR(path_stat.st_mode);
	return !does_not_exist && !is_directory;
	#endif
}

/*
Check if a file or folder exists.
*/
bool une_file_or_folder_exists(char *path)
{
	#ifdef _WIN32
	DWORD attribs = GetFileAttributesA(path);
	return attribs != INVALID_FILE_ATTRIBUTES;
	#else
	struct stat path_stat;
	int does_not_exist = stat(path, &path_stat);
	return !does_not_exist;
	#endif
}

/*
Open a UTF-8 file at 'path' and return its text contents as wchar_t string.
*/
wchar_t *une_file_read(char *path, bool normalize_line_endings, size_t convert_tabs_to_spaces)
{
	if (path == NULL)
		return NULL;
	
	FILE *stream = fopen(path, UNE_FOPEN_RFLAGS);
	if (stream == NULL)
		return NULL;
	
	size_t content_size = UNE_SIZE_FILE_BUFFER;
	wchar_t *content = malloc(content_size * sizeof(*content));
	verify(content);
	
	size_t index = 0;
	wint_t current_character; /* DOC: Can represent any Unicode character + WEOF (!).
							 This is important when using fgetwc(), as otherwise,
							 WEOF will overflow and be indistinguishable from an
							 actual character. */
	
	while (true) {
		current_character = fgetwc(stream);
		
		if (normalize_line_endings && current_character == L'\r')
			continue;
		
		size_t insertion_length = 1;
		if (convert_tabs_to_spaces > 0 && current_character == L'\t')
			insertion_length = convert_tabs_to_spaces;

		if (index + insertion_length > content_size) {
			content_size *= 2;
			content = realloc(content, content_size * sizeof(*content));
			verify(content);
		}
		
		if (convert_tabs_to_spaces > 0 && current_character == L'\t') {
			for (size_t i=0; i<convert_tabs_to_spaces; i++)
				content[index+i] = L' ';
		} else if (current_character == WEOF) {
			content[index] = L'\0';
			break;
		} else {
			content[index] = (wchar_t)current_character;
		}
		
		index += insertion_length;
	}

	fclose(stream);
	
	return content;
}

/*
Check if a file extension matches a given path.
*/
bool une_file_extension_matches(char *path, char *extension)
{
	assert(path);
	assert(extension);
	
	size_t path_length = strlen(path);
	size_t extension_length = strlen(extension);
	if (extension_length >= path_length)
		return false;
	
	char *path_extension_lower = malloc((extension_length+1)*sizeof(*path_extension_lower));
	verify(path_extension_lower);
	for (size_t i=0; i<=extension_length; i++)
		path_extension_lower[i] = (char)tolower(path[path_length-extension_length+i]);
	
	char *extension_lower = malloc((extension_length+1)*sizeof(*extension_lower));
	verify(extension_lower);
	for (size_t i=0; i<=extension_length; i++)
		extension_lower[i] = (char)tolower(extension[i]);
	
	int strcmp_return_code = strcmp(path_extension_lower, extension_lower);
	free(path_extension_lower);
	free(extension_lower);
	return strcmp_return_code == 0;
}

/*
Get the working directory.
*/
wchar_t *une_get_working_directory(void)
{
	#ifdef _WIN32
	DWORD size = GetCurrentDirectoryW(0, NULL); /* Determine required buffer size. */
	wchar_t *path = malloc(size*sizeof(*path));
	verify(path);
	DWORD count = GetCurrentDirectoryW(size, path);
	if (count == 0) {
		free(path);
		return NULL;
	}
	assert(count + 1 /* \0. */ == size);
	
	#else
	
	size_t size = PATH_MAX;
	char *path_narrow = malloc(size * sizeof(*path_narrow));
	verify(path_narrow);
	while (!getcwd(path_narrow, size-1)) {
		if (errno != ERANGE) {
			free(path_narrow);
			return NULL;
		}
		size *= 2;
		path_narrow = realloc(path_narrow, size * sizeof(*path_narrow));
		verify(path_narrow);
	}
	wchar_t *path = une_str_to_wcs(path_narrow);
	free(path_narrow);
	#endif
	
	return path;
}

/*
Set the working directory.
*/
bool une_set_working_directory(wchar_t *path)
{
	#ifdef _WIN32
	return SetCurrentDirectoryW(path) != 0;
	#else
	
	char *path_narrow = une_wcs_to_str(path);
	if (!path_narrow)
		return false;
	int code = chdir(path_narrow);
	free(path_narrow);
	return code == 0;
	#endif
}

/*
Halt execution for the specified number of miliseconds.
*/
void une_sleep_ms(int ms)
{
	#ifdef _WIN32
	Sleep((DWORD)ms);
	#else
	struct timespec ts = {
		.tv_sec = ms / 1000,
		.tv_nsec = ms % 1000 * 1000000
	};
	nanosleep(&ts, NULL);
	#endif
}

/*
Play a WAV file.
*/
bool une_play_wav(wchar_t *path)
{
	#ifdef _WIN32
	return PlaySoundW(path, NULL, SND_FILENAME|SND_NODEFAULT|SND_ASYNC);
	#else
	return false;
	#endif
}

/*
Print a message and abort.
*/
int une_out_of_memory(void)
{
	wprintf(UNE_ERROR_OUT_OF_MEMORY L"\n");
	fflush(stdout);
	abort();
}

/*
Compares two une_flt for equality.
*/
bool une_flts_equal(une_flt a, une_flt b)
{
	une_flt epsilon = une_flt_nextafter(UNE_NEW_FLT(0.0), UNE_NEW_FLT(1.0));
	return une_flt_abs(a - b) < epsilon;
}

/*
Clamps a une_int to a minimum.
*/
une_int une_min(une_int num, une_int min)
{
	if (num < min)
		return min;
	return num;
}

/*
Clamps a une_int to a maximum.
*/
une_int une_max(une_int num, une_int max)
{
	if (num > max)
		return max;
	return num;
}

/*
Clamps a une_int between a minimum and a maximum.
*/
une_int une_clamp(une_int num, une_int min, une_int max)
{
	if (num < min)
		return min;
	if (num > max)
		return max;
	return num;
}

/*
Get absolute index from relative one.
*/
une_range une_range_from_relative_index(une_result index, size_t scope)
{
	assert(index.kind == UNE_RK_INT);
	
	/* Unpack index. */
	une_int relative_index = index.value._int;
	
	/* Map relative index to absolute one. */
	une_int absolute_index = relative_index >= 0 ? relative_index : (une_int)scope + relative_index;
	
	/* Clamp index to scope. */
	bool valid = false;
	if (absolute_index < 0)
		absolute_index = 0;
	else if (absolute_index >= (une_int)scope)
		absolute_index = (une_int)scope - 1;
	else
		valid = true;
	
	return (une_range){
		.valid = valid,
		.first = (size_t)absolute_index
	};
}

/*
Get range from relative indices.
*/
une_range une_range_from_relative_indices(une_result begin, une_result end, size_t scope)
{
	assert(begin.kind == UNE_RK_INT);
	assert(end.kind == UNE_RK_INT || end.kind == UNE_RK_VOID);
	
	/* Unpack results. */
	une_int begin_index = begin.value._int;
	une_int end_index = end.kind == UNE_RK_INT ? end.value._int : (une_int)scope;
	
	/* Map relative indices to absolute ones. */
	une_int absolute_begin = begin_index >= 0 ? begin_index : (une_int)scope + begin_index;
	une_int absolute_end = end_index >= 0 ? end_index : (une_int)scope + end_index;
	
	/* Clamp indices to scope. */
	bool valid = false;
	if (absolute_begin < 0)
		absolute_begin = 0;
	else if (absolute_end < 0)
		absolute_end = 0;
	else if (absolute_begin > (une_int)scope)
		absolute_begin = (une_int)scope;
	else if (absolute_end > (une_int)scope)
		absolute_end = (une_int)scope;
	else
		valid = true;
	
	/* Determine step and length. */
	/* TODO: In the future, I'd like to support backwards ranges.
	This would either be expressed through a .step member in une_range
	or via a negative .length (for usability purposes, preferably the first). */
	if (absolute_begin > absolute_end)
		absolute_begin = absolute_end;
	size_t length = (size_t)(absolute_end - absolute_begin);
	
	return (une_range){
		.valid = valid,
		.first = (size_t)absolute_begin,
		.guard = (size_t)absolute_end,
		.length = length
	};
}

/*
Check if a position is valid.
*/
bool une_position_is_valid(une_position position)
{
	return position.end - position.start > 0;
}

/*
Merge a start and end une_position.
*/
une_position une_position_between(une_position first, une_position last)
{
	return (une_position){
		.start = first.start,
		.end = last.end,
		.line = first.line
	};
}

/*
A start and line to a une_position.
*/
une_position une_position_set_start(une_position subject, une_position begin)
{
	subject.start = begin.start;
	subject.line = begin.line;
	return subject;
}

/*
Find start of current line.
*/
size_t une_wcs_find_start_of_current_line(wchar_t *wcs, size_t starting_index)
{
	assert(wcs);
	assert(starting_index <= wcslen(wcs));

	size_t index = starting_index;
	while (index > 0 && wcs[index-1] != L'\n')
		index--;
	return index;
}

/*
Find end of current line.
*/
size_t une_wcs_find_end_of_current_line(wchar_t *wcs, size_t starting_index)
{
	assert(wcs);
	assert(starting_index <= wcslen(wcs));

	size_t index = starting_index;
	while (wcs[index] != L'\n' && wcs[index] != L'\0')
		index++;
	return index;
}

/*
Find first non-whitespace character in string.
*/
size_t une_wcs_skip_whitespace(wchar_t *wcs, size_t starting_index)
{
	assert(wcs);
	assert(starting_index <= wcslen(wcs));

	size_t index = starting_index;
	while (wcs[index] == L' ')
		index++;
	return index;
}

#ifdef _WIN32
/*
Enable Virtual Terminal Processing for the Windows console.
*/
void une_win_vt_proc(bool enable)
{
	static DWORD mode = 0;
	static bool mode_defined = false;
	HANDLE stdout_ = GetStdHandle(STD_OUTPUT_HANDLE);
	if (enable) {
		if (!mode_defined) {
			GetConsoleMode(stdout_, &mode);
			mode_defined = true;
		}
		SetConsoleMode(stdout_, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	} else {
		if (mode_defined)
			SetConsoleMode(stdout, mode);
	}
}
#endif
