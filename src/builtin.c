/*
builtin.c - Une
Modified 2021-08-05
*/

/* Header-specific includes. */
#include "builtin.h"

/* Implementation-specific includes. */
#include <time.h>
#include "tools.h"
#include "une.h"
#include "stream.h"

/*
Get the une_builtin_type from a string containing a built-in function name.
*/
const une_builtin_type une_builtin_wcs_to_type(wchar_t *name)
{
  if (wcscmp(name, L"put") == 0)
    return UNE_BIF_PUT;
  if (wcscmp(name, L"print") == 0)
    return UNE_BIF_PRINT;
  if (wcscmp(name, L"int") == 0)
    return UNE_BIF_TO_INT;
  if (wcscmp(name, L"flt") == 0)
    return UNE_BIF_TO_FLT;
  if (wcscmp(name, L"str") == 0)
    return UNE_BIF_TO_STR;
  if (wcscmp(name, L"len") == 0)
    return UNE_BIF_GET_LEN;
  if (wcscmp(name, L"sleep") == 0)
    return UNE_BIF_SLEEP;
  if (wcscmp(name, L"chr") == 0)
    return UNE_BIF_CHR;
  if (wcscmp(name, L"ord") == 0)
    return UNE_BIF_ORD;
  if (wcscmp(name, L"read") == 0)
    return UNE_BIF_READ;
  if (wcscmp(name, L"write") == 0)
    return UNE_BIF_WRITE;
  if (wcscmp(name, L"input") == 0)
    return UNE_BIF_INPUT;
  if (wcscmp(name, L"script") == 0)
    return UNE_BIF_SCRIPT;
  if (wcscmp(name, L"exist") == 0)
    return UNE_BIF_EXIST;
  if (wcscmp(name, L"split") == 0)
    return UNE_BIF_SPLIT;
  return __UNE_BIF_none__;
}

/*
Get the number of parameters required by a built-in function.
*/
const une_int une_builtin_get_num_of_params(une_builtin_type type)
{
  assert(UNE_BUILTIN_TYPE_IS_VALID(type));
  switch (type) {
    case UNE_BIF_WRITE:
    case UNE_BIF_SPLIT:
      return 2;
    default:
      return 1;
  }
}

/*
Print a text representation of a une_result.
*/
__une_builtin_fn(une_builtin_put, une_position pos, une_result result)
{
  /* Print text representation. */
  une_result_represent(stdout, result);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Print a text representation of a une_result, always adding a newline at the end.
*/
__une_builtin_fn(une_builtin_print, une_position pos, une_result result)
{
  une_builtin_put(error, is, pos, result);
  
  putwc(L'\n', stdout);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Convert une_result to UNE_RT_INT une_result and return it.
*/
__une_builtin_fn(une_builtin_to_int, une_position pos, une_result result)
{
  /* Convert une_result. */
  switch (result.type) {
    case UNE_RT_INT:
      return une_result_copy(result);
    case UNE_RT_FLT:
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = (une_int)result.value._flt
      };
    case UNE_RT_STR: {
      une_int int_;
      if (!une_wcs_to_une_int(result.value._wcs, &int_)) {
        *error = UNE_ERROR_SET(UNE_ET_ENCODING, pos);
        return une_result_create(UNE_RT_ERROR);
      }
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = int_
      };
    }
  }
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Convert une_result to UNE_RT_FLT une_result and return it.
*/
__une_builtin_fn(une_builtin_to_flt, une_position pos, une_result result)
{
  /* Convert une_result. */
  switch (result.type) {
    case UNE_RT_INT:
      return (une_result){
        .type = UNE_RT_FLT,
        .value._flt = (une_flt)result.value._int
      };
    case UNE_RT_FLT:
      return une_result_copy(result);
    case UNE_RT_STR: {
      une_flt flt;
      if (!une_wcs_to_une_flt(result.value._wcs, &flt)) {
        *error = UNE_ERROR_SET(UNE_ET_ENCODING, pos);
        return une_result_create(UNE_RT_ERROR);
      }
      return (une_result){
        .type = UNE_RT_FLT,
        .value._flt = flt
      };
    }
  }
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Convert une_result to UNE_RT_STR une_result and return it.
*/
__une_builtin_fn(une_builtin_to_str, une_position pos, une_result result)
{
  /* Convert une_result. */
  switch (result.type) {
    case UNE_RT_INT: {
      wchar_t *out = malloc(UNE_SIZE_NUM_TO_STR_LEN*sizeof(*out));
      swprintf(out, UNE_SIZE_NUM_TO_STR_LEN, UNE_PRINTF_UNE_INT, result.value._int);
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = out
      };
    }
    case UNE_RT_FLT: {
      wchar_t *out = malloc(UNE_SIZE_NUM_TO_STR_LEN*sizeof(*out));
      swprintf(out, UNE_SIZE_NUM_TO_STR_LEN, UNE_PRINTF_UNE_FLT, result.value._flt);
      return (une_result){
        .type = UNE_RT_STR,
        .value._wcs = out
      };
    }
    case UNE_RT_STR:
      return une_result_copy(result);
  }
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Get length of une_result and return it.
*/
__une_builtin_fn(une_builtin_get_len, une_position pos, une_result result)
{
  /* Get length of une_result. */
  switch (result.type) {
    case UNE_RT_STR:
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = wcslen(result.value._wcs)
      };
    case UNE_RT_LIST: {
      UNE_UNPACK_RESULT_LIST(result, result_p, result_size);
      return (une_result){
        .type = UNE_RT_INT,
        .value._int = result_size
      };
    }
  }
  
  /* Illegal input une_result_type. */
  *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
  return une_result_create(UNE_RT_ERROR);
}

/*
Halt execution for a given amount of miliseconds.
*/
__une_builtin_fn(une_builtin_sleep, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_INT. */
  if (result.type != UNE_RT_INT) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Halt execution. */
  struct timespec ts = {
    .tv_sec = result.value._int / 1000,
    .tv_nsec = result.value._int % 1000 * 1000000
  };
  nanosleep(&ts, NULL);
  
  return une_result_create(UNE_RT_VOID);
}

/*
Convert a number to its corresponding one-character string.
*/
__une_builtin_fn(une_builtin_chr, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_INT. */
  if (result.type != UNE_RT_INT) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  une_result chr = une_result_create(UNE_RT_STR);
  chr.value._wcs = malloc(2*sizeof(*chr.value._wcs));
  chr.value._wcs[0] = (wchar_t)result.value._int;
  chr.value._wcs[1] = L'\0';
  return chr;
}

/*
Convert a one-character string to its corresponding number.
*/
__une_builtin_fn(une_builtin_ord, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_STR and is only one character long. */
  if (result.type != UNE_RT_STR || wcslen(result.value._wcs) != 1) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  une_result ord = une_result_create(UNE_RT_INT);
  ord.value._int = (une_int)result.value._wcs[0];
  return ord;
}

/*
Return the entire contents of a file as text.
*/
__une_builtin_fn(une_builtin_read, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_STR. */
  if (result.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Check if file exists. */
  char *path = une_wcs_to_str(result.value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  if (!une_file_exists(path)) {
    free(path);
    *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  
  /* Read file. */
  une_result str = une_result_create(UNE_RT_STR);
  str.value._wcs = une_file_read(path);
  assert(str.value._wcs != NULL);
  free(path);
  return str;
}

/*
Write text to a file.
*/
__une_builtin_fn(une_builtin_write, une_position pos_file, une_result file, une_position pos_text, une_result text)
{
  /* Ensure input une_result_types are UNE_RT_STR. */
  if (file.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos_file);
    return une_result_create(UNE_RT_ERROR);
  }
  if (text.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos_text);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Create file. */
  char *path = une_wcs_to_str(file.value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, pos_file);
    return une_result_create(UNE_RT_ERROR);
  }
  FILE *fp = fopen(path, UNE_FOPEN_WFLAGS);
  free(path);
  if (fp == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, pos_file);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Print text. */
  fputws(text.value._wcs, fp);
  fclose(fp);
  return une_result_create(UNE_RT_VOID);
}

/*
Get user input as string from the console.
*/
__une_builtin_fn(une_builtin_input, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_STR. */
  if (result.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Get string. */
  une_result_represent(stdout, result);
  wchar_t *instr = malloc(UNE_SIZE_FGETWS_BUFFER*sizeof(*instr));
  fgetws(instr, UNE_SIZE_FGETWS_BUFFER, stdin);
  size_t len = wcslen(instr);
  instr[--len] = L'\0'; /* Remove trailing newline. */

  /* Return result. */
  une_result str = une_result_create(UNE_RT_STR);
  str.value._wcs = malloc((len+1)*sizeof(*str.value._wcs));
  wcscpy(str.value._wcs, instr);
  free(instr);
  return str;
}

/*
Run an external Une script.
*/
__une_builtin_fn(une_builtin_script, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_STR. */
  if (result.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Check if file exists. */
  char *path = une_wcs_to_str(result.value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  if (!une_file_exists(path)) {
    free(path);
    *error = UNE_ERROR_SET(UNE_ET_FILE_NOT_FOUND, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Run script. */
  une_result out = une_run(true, path, NULL);
  free(path);
  return out;
}

/*
Check if a file or directory exists.
*/
__une_builtin_fn(une_builtin_exist, une_position pos, une_result result)
{
  /* Ensure input une_result_type is UNE_RT_STR. */
  if (result.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos);
    return une_result_create(UNE_RT_ERROR);
  }

  /* Check if file or folder exists. */
  char *path = une_wcs_to_str(result.value._wcs);
  if (path == NULL) {
    *error = UNE_ERROR_SET(UNE_ET_ENCODING, pos);
    return une_result_create(UNE_RT_ERROR);
  }
  bool exists = une_file_or_folder_exists(path);
  free(path);
  return (une_result){
    .type = UNE_RT_INT,
    .value._int = (une_int)exists
  };
}

/* Split a string into a list of substrings. */
UNE_OSTREAM_PUSHER(__une_builtin_split_push, une_result)
__une_builtin_fn(une_builtin_split, une_position pos_string, une_result string, une_position pos_delims, une_result delims)
{
  /* Ensure input une_result_types are UNE_RT_STR and UNE_RT_LIST, where each member is of type UNE_RT_STR. */
  if (string.type != UNE_RT_STR) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos_string);
    return une_result_create(UNE_RT_ERROR);
  }
  if (delims.type != UNE_RT_LIST || ((une_result*)delims.value._vp)[0].value._int == 0) {
    *error = UNE_ERROR_SET(UNE_ET_TYPE, pos_delims);
    return une_result_create(UNE_RT_ERROR);
  }
  UNE_UNPACK_RESULT_LIST(delims, delims_p, delims_len);
  UNE_FOR_RESULT_LIST_ITEM(i, delims_len) {
    if (delims_p[i].type != UNE_RT_STR || wcslen(delims_p[i].value._wcs) <= 0) {
      *error = UNE_ERROR_SET(UNE_ET_TYPE, pos_delims);
      return une_result_create(UNE_RT_ERROR);
    }
  }

  /* Setup. */
  size_t tokens_amt = 0;
  une_result *tokens = malloc((UNE_SIZE_BIF_SPLIT_TKS+1)*sizeof(*tokens));
  une_ostream out = une_ostream_create((void*)tokens, UNE_SIZE_BIF_SPLIT_TKS+1, sizeof(*tokens), true);
  tokens = NULL; /* This pointer can turn stale after pushing. */
  void (*push)(une_ostream*, une_result) = &__une_builtin_split_push;
  push(&out, une_result_create(UNE_RT_SIZE));
  /* Cache delimiter lengths for performance. */
  size_t *delim_lens = malloc(delims_len*sizeof(*delim_lens));
  for (size_t i=0; i<delims_len; i++)
    delim_lens[i] = wcslen(delims_p[i+1].value._wcs);
  wchar_t *wcs = string.value._wcs;
  size_t wcs_len = wcslen(wcs);
  size_t last_token_end = 0;
  int left_for_match = 0;

  /* Create tokens. */
  for (size_t i=0; i<wcs_len; i++) { /* For each character. */
    for (size_t delim=0; delim<delims_len; delim++) { /* For each delimiter. */
      left_for_match = delim_lens[delim];
      for (size_t delim_i=0; delim_i<delim_lens[delim]; delim_i++) { /* For each character in the delimiter. */
        if (i+delim_i >= wcs_len || wcs[i+delim_i] != delims_p[delim+1].value._wcs[delim_i])
          break;
        left_for_match--;
      }
      if (left_for_match != 0 && i+1 == wcs_len) { /* Special case for end of string. */
        i++;
        left_for_match = 0;
      }
      if (left_for_match == 0) { /* All characters have been matched. */
        size_t last_token_end_cpy = last_token_end;
        size_t substr_len = i-last_token_end_cpy;
        i += delim_lens[delim]-1 /* Compensate for next i++. */;
        last_token_end = i+1; /* last_token_end is not incremented next loop. */
        /* Break if there was no token before this delimiter. */
        if (substr_len == 0)
          break;
        /* Create substring. */
        wchar_t *substr = malloc((substr_len+1)*sizeof(*substr));
        wmemcpy(substr, wcs+last_token_end_cpy, substr_len);
        substr[substr_len] = L'\0';
        /* Push substring. */
        push(&out, (une_result){
          .type = UNE_RT_STR,
          .value._wcs = substr
        });
        tokens_amt++;
        break;
      }
    }
  }

  /* Wrap up. */
  free(delim_lens);
  tokens = (une_result*)out.array; /* Reobtain up-to-date pointer. */
  tokens[0].value._int = tokens_amt;
  return (une_result){
    .type = UNE_RT_LIST,
    .value._vp = (void*)tokens
  };
}
