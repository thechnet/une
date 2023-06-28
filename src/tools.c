/*
tools.c - Une
Modified 2023-06-25
*/

/* FIXME: Because watchdog.h overrides sizeof we need to include windows.h here. */
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

/* Header-specific includes. */
#include "tools.h"

/* Implementation-specific includes. */
#include <string.h>
#include <time.h>
#include <errno.h>

/*
Convert a wchar_t string into a une_int integer.
*/
bool une_wcs_to_une_int(wchar_t *wcs, une_int *dest)
{
  wchar_t *wcs_end = wcs+wcslen(wcs);
  wchar_t *int_end;
  *dest = wcstoll(wcs, &int_end, 10);
  return int_end > wcs && int_end == wcs_end ? true : false;
}

/*
Convert a wchar_t string into a une_flt floating pointer number.
*/
bool une_wcs_to_une_flt(wchar_t *wcs, une_flt *dest)
{
  wchar_t *wcs_end = wcs+wcslen(wcs);
  wchar_t *flt_end;
  *dest = wcstod(wcs, &flt_end);
  return flt_end > wcs && flt_end == wcs_end ? true : false;
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
wchar_t *une_file_read(char *path)
{
  FILE *f = fopen(path, UNE_FOPEN_RFLAGS);
  if (f == NULL)
    return NULL;
  size_t text_size = UNE_SIZE_FILE_BUFFER;
  wchar_t *text = malloc(text_size*sizeof(*text));
  verify(text);
  size_t cursor = 0;
  wint_t c; /* DOC: Can represent any Unicode character + WEOF (!).
               This is important when using fgetwc(), as otherwise,
               WEOF will overflow and be indistinguishable from an
               actual character. */
  while (true) {
    c = fgetwc(f);
    if (c == L'\r')
      continue;
    if (cursor >= text_size-1) { /* NUL. */
      text_size *= 2;
      text = realloc(text, text_size *sizeof(*text));
      verify(text);
    }
    if (c == WEOF)
      break;
    text[cursor] = (wchar_t)c;
    cursor++;
  }
  fclose(f);
  text[cursor] = L'\0';
  return text;
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
  double epsilon = nextafter(0, 1);
  return fabs(a - b) < epsilon;
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
  assert(index.type == UNE_RT_INT);
  
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
  assert(begin.type == UNE_RT_INT);
  assert(end.type == UNE_RT_INT || end.type == UNE_RT_VOID);
  
  /* Unpack results. */
  une_int begin_index = begin.value._int;
  une_int end_index = end.type == UNE_RT_INT ? end.value._int : (une_int)scope;
  
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
