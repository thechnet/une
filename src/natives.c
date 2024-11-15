/*
natives.c - Une
Modified 2024-01-31
*/

/* Header-specific includes. */
#include "natives.h"

/* Implementation-specific includes. */
#include <time.h>
#include "tools.h"
#include "struct/engine.h"
#include "deprecated/stream.h"
#include "types/types.h"

/*
Pointers to all native functions.
*/
une_native_fnptr une_natives[] = {
	#define NATIVE_FUNCTION__(name__) &une_native_fn_##name__,
	UNE_ENUMERATE_NATIVE_FUNCTIONS(NATIVE_FUNCTION__)
	#undef NATIVE_FUNCTION__
};

/*
String representations of native function names.
*/
const wchar_t *une_natives_as_strings[] = {
	#define NATIVE_AS_STRING__(name__) L"" #name__,
	UNE_ENUMERATE_NATIVE_FUNCTIONS(NATIVE_AS_STRING__)
	#undef NATIVE_AS_STRING__
};

/* The amount of parameters of native functions. */
const size_t une_natives_params_count[] = {
	1, /* put */
	1, /* print */
	1, /* int */
	1, /* flt */
	1, /* str */
	1, /* len */
	1, /* sleep */
	1, /* chr */
	1, /* ord */
	1, /* read */
	2, /* write */
	2, /* append */
	1, /* input */
	1, /* script */
	1, /* exist */
	2, /* split */
	1, /* eval */
	3, /* replace */
	2, /* join */
	2, /* sort */
	0, /* getcwd */
	1, /* setcwd */
	1, /* playwav */
};

/*
*** Interface.
*/

/*
Get the number of parameters of a native function.
*/
size_t une_native_params_count(une_native function)
{
	assert(UNE_NATIVE_IS_VALID(function));
	return une_natives_params_count[function-1];
}

/*
Get the function pointer for a native-in function.
*/
une_native_fnptr une_native_to_fnptr(une_native function)
{
	assert(UNE_NATIVE_IS_VALID(function));
	return une_natives[function-1];
}

/*
Get a pointer to the native function matching the given string or NULL;
*/
une_native une_native_wcs_to_function(wchar_t *wcs)
{
	une_native fn = UNE_NATIVE_none__;
	for (une_native i=1; i<UNE_NATIVE_max__; i++)
		if (wcscmp(une_natives_as_strings[i-1], wcs) == 0) {
			fn = i;
			break;
		}
	return fn;
}

/*
Convert a une_native to its string representation.
*/
#ifdef UNE_DEBUG
const wchar_t *une_native_to_wcs(une_native function)
{
	assert(UNE_NATIVE_IS_VALID(function));
	return une_natives_as_strings[function-1];
}
#endif /* UNE_DEBUG */

/*
***** Native Functions.
*/

/*
Print a text representation of a une_result.
*/
une_native_fn__(put)
{
	une_native_param string = 0;
	une_type string_type = UNE_TYPE_FOR_RESULT(args[string]);
	
	assert(string_type.represent != NULL);
	string_type.represent(stdout, args[string]);
	
	return une_result_create(UNE_RK_VOID);
}

/*
Print a text representation of a une_result, always adding a newline at the end.
*/
une_native_fn__(print)
{
	une_native_fn_put(call_node, args);
	
	putwc(L'\n', stdout);
	
	return une_result_create(UNE_RK_VOID);
}

/*
Convert une_result to UNE_RK_INT une_result and return it.
*/
une_native_fn__(int)
{
	une_native_param result = 0;
	une_type result_type = UNE_TYPE_FOR_RESULT(args[result]);
	
	if (result_type.as_int == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	une_result result_as_int = result_type.as_int(args[result]);
	if (result_as_int.kind == UNE_RK_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	return result_as_int;
}

/*
Convert une_result to UNE_RK_FLT une_result and return it.
*/
une_native_fn__(flt)
{
	une_native_param result = 0;
	une_type result_type = UNE_TYPE_FOR_RESULT(args[result]);
	
	if (result_type.as_flt == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	une_result result_as_flt = result_type.as_flt(args[result]);
	if (result_as_flt.kind == UNE_RK_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	return result_as_flt;
}

/*
Convert une_result to UNE_RK_STR une_result and return it.
*/
une_native_fn__(str)
{
	une_native_param result = 0;
	une_type result_type = UNE_TYPE_FOR_RESULT(args[result]);
	
	if (result_type.as_str == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	une_result result_as_str = result_type.as_str(args[result]);
	assert(result_as_str.kind != UNE_RK_ERROR);
	return result_as_str;
}

/*
Get length of une_result and return it.
*/
une_native_fn__(len)
{
	une_native_param result = 0;
	une_type result_type = UNE_TYPE_FOR_RESULT(args[result]);
	
	if (result_type.get_len == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	size_t result_len = result_type.get_len(args[result]);
	
	return (une_result){
		.kind = UNE_RK_INT,
		.value._int = (une_int)result_len
	};
}

/*
Halt execution for a given amount of miliseconds.
*/
une_native_fn__(sleep)
{
	une_native_param result = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(result, UNE_RK_INT);
	
	/* Halt execution. */
	une_sleep_ms((int)args[result].value._int);
	
	return une_result_create(UNE_RK_VOID);
}

/*
Convert a number to its corresponding one-character string.
*/
une_native_fn__(chr)
{
	une_native_param result = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(result, UNE_RK_INT);
	
	if (args[result].value._int > WCHAR_MAX) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}
	
	une_result chr = une_result_create(UNE_RK_STR);
	chr.value._wcs = malloc(2*sizeof(*chr.value._wcs));
	verify(chr.value._wcs);
	chr.value._wcs[0] = (wchar_t)args[result].value._int;
	chr.value._wcs[1] = L'\0';
	return chr;
}

/*
Convert a one-character string to its corresponding number.
*/
une_native_fn__(ord)
{
	une_native_param result = 0;
	
	/* Ensure input une_result_kind is UNE_RK_STR and is only one character long. */
	if (args[result].kind != UNE_RK_STR || wcslen(args[result].value._wcs) != 1) {
		felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(result));
		return une_result_create(UNE_RK_ERROR);
	}

	une_result ord = une_result_create(UNE_RK_INT);
	ord.value._int = (une_int)args[result].value._wcs[0];
	return ord;
}

/*
Return the entire contents of a file as text.
*/
une_native_fn__(read)
{
	une_native_param file = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(file, UNE_RK_STR);
	
	/* Check if file exists. */
	char *path = une_wcs_to_str(args[file].value._wcs);
	if (path == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(file));
		return une_result_create(UNE_RK_ERROR);
	}
	if (!une_file_exists(path)) {
		free(path);
		felix->error = UNE_ERROR_SET(UNE_EK_FILE, UNE_NATIVE_POS_OF_ARG(file));
		return une_result_create(UNE_RK_ERROR);
	}
	
	/* Read file. */
	une_result str = une_result_create(UNE_RK_STR);
	str.value._wcs = une_file_read(path, true, 0);
	assert(str.value._wcs != NULL);
	free(path);
	return str;
}

/*
Write/append text to a file (helper).
*/
une_result une_native_fn_write_or_append(une_node *call_node, une_result *args, bool write)
{
	une_native_param file = 0;
	une_native_param text = 1;
	
	UNE_NATIVE_VERIFY_ARG_KIND(file, UNE_RK_STR);
	UNE_NATIVE_VERIFY_ARG_KIND(text, UNE_RK_STR);

	/* Create file. */
	char *path = une_wcs_to_str(args[file].value._wcs);
	if (path == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(file));
		return une_result_create(UNE_RK_ERROR);
	}
	FILE *fp = fopen(path, write ? UNE_FOPEN_WFLAGS : UNE_FOPEN_AFLAGS);
	free(path);
	if (fp == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_FILE, UNE_NATIVE_POS_OF_ARG(file));
		return une_result_create(UNE_RK_ERROR);
	}

	/* Print text. */
	fputws(args[text].value._wcs, fp);
	fclose(fp);
	return une_result_create(UNE_RK_VOID);
}

/*
Write text to a file.
*/
une_native_fn__(write)
{
	return une_native_fn_write_or_append(call_node, args, true);
}

/*
Append text to a file.
*/
une_native_fn__(append)
{
	return une_native_fn_write_or_append(call_node, args, false);
}

/*
Get user input as string from the console.
*/
une_native_fn__(input)
{
	une_native_param prompt = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(prompt, UNE_RK_STR);

	/* Get string. */
	une_type_str_represent(stdout, args[prompt]);
	wchar_t *instr = malloc(UNE_SIZE_FGETWS_BUFFER*sizeof(*instr));
	verify(instr);
	if (!fgetws(instr, UNE_SIZE_FGETWS_BUFFER, stdin)) assert(false);
	size_t len = wcslen(instr);
	instr[--len] = L'\0'; /* Remove trailing newline. */

	/* Return result. */
	une_result str = une_result_create(UNE_RK_STR);
	str.value._wcs = malloc((len+1)*sizeof(*str.value._wcs));
	verify(str.value._wcs);
	wcscpy(str.value._wcs, instr);
	free(instr);
	return str;
}

/*
Run an external Une script.
*/
une_native_fn__(script)
{
	une_native_param script = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(script, UNE_RK_STR);

	/* Check if file exists. */
	char *path = une_wcs_to_str(args[script].value._wcs);
	if (path == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(script));
		return une_result_create(UNE_RK_ERROR);
	}
	if (!une_file_exists(path)) {
		free(path);
		felix->error = UNE_ERROR_SET(UNE_EK_FILE, UNE_NATIVE_POS_OF_ARG(script));
		return une_result_create(UNE_RK_ERROR);
	}

	/* Run script. */
	une_result out = une_engine_interpret_file_or_wcs_with_position(path, NULL, call_node->pos);
	free(path);
	
	return out;
}

/*
Check if a file or directory exists.
*/
une_native_fn__(exist)
{
	une_native_param path = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(path, UNE_RK_STR);

	/* Check if file or folder exists. */
	char *path_str = une_wcs_to_str(args[path].value._wcs);
	if (path_str == NULL) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(path));
		return une_result_create(UNE_RK_ERROR);
	}
	bool exists = une_file_or_folder_exists(path_str);
	free(path_str);
	return (une_result){
		.kind = UNE_RK_INT,
		.value._int = (une_int)exists
	};
}

/*
Split a string into a list of substrings.
*/
UNE_OSTREAM_PUSHER(une_native_split_push__, une_result)
une_native_fn__(split)
{
	une_native_param string = 0;
	une_native_param delims = 1;
	
	UNE_NATIVE_VERIFY_ARG_KIND(string, UNE_RK_STR);
	UNE_NATIVE_VERIFY_ARG_KIND(delims, UNE_RK_LIST);
	
	/* Ensure every member of delims is of kind UNE_RK_STR. */
	UNE_UNPACK_RESULT_LIST(args[delims], delims_p, delims_len);
	UNE_FOR_RESULT_LIST_ITEM(i, delims_len) {
		if (delims_p[i].kind != UNE_RK_STR || wcslen(delims_p[i].value._wcs) <= 0) {
			felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(delims));
			return une_result_create(UNE_RK_ERROR);
		}
	}

	/* Setup. */
	size_t tokens_amt = 0;
	une_result *tokens = malloc((UNE_SIZE_BIF_SPLIT_TKS+1)*sizeof(*tokens));
	verify(tokens);
	une_ostream out = une_ostream_create((void*)tokens, UNE_SIZE_BIF_SPLIT_TKS+1, sizeof(*tokens), true);
	tokens = NULL; /* This pointer can turn stale after pushing. */
	void (*push)(une_ostream*, une_result) = &une_native_split_push__;
	push(&out, une_result_create(UNE_RK_SIZE));
	/* Cache delimiter lengths for performance. */
	size_t *delim_lens = malloc(delims_len*sizeof(*delim_lens));
	verify(delim_lens);
	for (size_t i=0; i<delims_len; i++)
		delim_lens[i] = wcslen(delims_p[i+1].value._wcs);
	wchar_t *wcs = args[string].value._wcs;
	size_t wcs_len = wcslen(wcs);
	size_t last_token_end = 0;
	size_t left_for_match = 0;

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
				verify(substr);
				wmemcpy(substr, wcs+last_token_end_cpy, substr_len);
				substr[substr_len] = L'\0';
				/* Push substring. */
				push(&out, (une_result){
					.kind = UNE_RK_STR,
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
	tokens[0].value._int = (une_int)tokens_amt;
	return (une_result){
		.kind = UNE_RK_LIST,
		.value._vp = (void*)tokens
	};
}

/*
Evaluate a Une script in string form.
*/
une_native_fn__(eval)
{
	une_native_param script = 0;
	
	UNE_NATIVE_VERIFY_ARG_KIND(script, UNE_RK_STR);

	/* Run script. */
	une_result out = une_engine_interpret_file_or_wcs_with_position(NULL, args[script].value._wcs, call_node->pos);
	
	return out;
}

/*
Replace string 'search' with string 'replace' in string 'subject'.
*/
une_native_fn__(replace)
{
	une_native_param search_arg = 0;
	une_native_param replace_arg = 1;
	une_native_param subject_arg = 2;
	
	UNE_NATIVE_VERIFY_ARG_KIND(search_arg, UNE_RK_STR);
	UNE_NATIVE_VERIFY_ARG_KIND(replace_arg, UNE_RK_STR);
	UNE_NATIVE_VERIFY_ARG_KIND(subject_arg, UNE_RK_STR);
	
	wchar_t *search = args[search_arg].value._wcs;
	wchar_t *replace = args[replace_arg].value._wcs;
	wchar_t *subject = args[subject_arg].value._wcs;
	
	size_t search_len = wcslen(search);
	if (!search_len) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(search_arg));
		return une_result_create(UNE_RK_ERROR);
	}
	size_t replace_len = wcslen(replace);
	size_t subject_len = wcslen(subject);
	size_t new_size = subject_len * (replace_len || 1) + 1;
	
	/* Allocate memory for new string. */
	wchar_t *new = malloc(new_size*sizeof(*new));
	
	/* Create new string. */
	size_t new_idx = 0, subject_idx = 0;
	while (subject_idx < subject_len)
		if (wcsncmp(subject+subject_idx, search, search_len) == 0) {
			subject_idx += search_len;
			wcsncpy(new+new_idx, replace, replace_len);
			new_idx += replace_len;
		} else {
			new[new_idx++] = subject[subject_idx++];
		}
	new[new_idx] = L'\0';
	
	une_result result = une_result_create(UNE_RK_STR);
	result.value._wcs = new;
	return result;
}

/*
Join the strings in list 'list' with seperator 'seperator'.
*/
une_native_fn__(join)
{
	une_native_param list = 0;
	une_native_param seperator = 1;
	
	UNE_NATIVE_VERIFY_ARG_KIND(list, UNE_RK_LIST);
	UNE_UNPACK_RESULT_LIST(args[list], elements, count);
	UNE_FOR_RESULT_LIST_ITEM(i, count)
		if (elements[i].kind != UNE_RK_STR) {
			felix->error = UNE_ERROR_SET(UNE_EK_TYPE, UNE_NATIVE_POS_OF_ARG(list));
			return une_result_create(UNE_RK_ERROR);
		}
	UNE_NATIVE_VERIFY_ARG_KIND(seperator, UNE_RK_STR);
	
	/* Prepare lengths and sizes. */
	size_t seperator_length = wcslen(args[seperator].value._wcs);
	size_t joined_length = (count > 1 ? count-1 : 0) * seperator_length;
	UNE_FOR_RESULT_LIST_ITEM(i, count)
		joined_length += wcslen(elements[i].value._wcs);
	wchar_t *joined_string = malloc((joined_length+1)*sizeof(*joined_string));
	verify(joined_string);
	
	/* Assemble string. */
	joined_string[0] = L'\0';
	UNE_FOR_RESULT_LIST_ITEM(i, count) {
		wcscat(joined_string, elements[i].value._wcs);
		if (i < count)
			wcscat(joined_string, args[seperator].value._wcs);
	}
	joined_string[joined_length] = L'\0';
	
	une_result result = une_result_create(UNE_RK_STR);
	result.value._wcs = joined_string;
	return result;
}

static une_error *sort_error = NULL;
static une_node *sort_call_node = NULL;
static une_result sort_comparator = { .kind = UNE_RK_none__ };

static int sort_compare(const void *a, const void *b)
{
	assert(sort_error);
	/* Avoid repeated traceback. */
	if (sort_error->kind != UNE_EK_none__)
		return 0;
	
	assert(sort_call_node);
	une_type comparator_type = UNE_TYPE_FOR_RESULT(sort_comparator);
	assert(comparator_type.call);
	
	une_result *subjects = une_result_list_create(2);
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wcast-qual"
	subjects[1] = une_result_copy(*(une_result*)a);
	subjects[2] = une_result_copy(*(une_result*)b);
	#pragma clang diagnostic pop
	une_result comparator_args = une_result_create(UNE_RK_LIST);
	comparator_args.value._vp = (void*)subjects;
	
	une_result result = comparator_type.call(sort_call_node, sort_comparator, comparator_args, NULL);
	int rating = 0;
	if (result.kind == UNE_RK_INT)
		rating = result.value._int > 0 ? 1 : result.value._int < 0 ? -1 : 0;
	else if (result.kind != UNE_RK_ERROR)
		*sort_error = UNE_ERROR_SET(UNE_EK_TYPE, sort_call_node->pos);
	
	une_result_free(result);
	une_result_free(comparator_args);
	
	return rating;
}

/*
Return the list 'subject', sorted using the function 'compare'.
*/
une_native_fn__(sort)
{
	une_native_param subject = 0;
	une_native_param compare = 1;
	
	UNE_NATIVE_VERIFY_ARG_KIND(subject, UNE_RK_LIST);
	UNE_NATIVE_VERIFY_ARG_KIND(compare, UNE_RK_FUNCTION);
	
	assert(!sort_error);
	assert(!sort_call_node);
	assert(sort_comparator.kind == UNE_RK_none__);
	sort_error = &felix->error;
	sort_call_node = call_node;
	sort_comparator = args[compare];
	
	une_result result = une_result_copy(args[subject]);
	UNE_UNPACK_RESULT_LIST(result, elements, count);
	qsort(elements + 1 /* Size. */, count, sizeof(*elements), &sort_compare);
	if (felix->error.kind != UNE_EK_none__) {
		une_result_free(result);
		return une_result_create(UNE_RK_ERROR);
	}
	
	sort_error = NULL;
	sort_call_node = NULL;
	sort_comparator = (une_result){ .kind = UNE_RK_none__ };
	
	return result;
}

/*
Get the working directory.
*/
une_native_fn__(getwd)
{
	wchar_t *path = une_get_working_directory();
	if (!path) {
		felix->error = UNE_ERROR_SET(UNE_EK_SYSTEM, call_node->pos);
		return une_result_create(UNE_RK_ERROR);
	}
	
	return (une_result){
		.kind = UNE_RK_STR,
		.value._wcs = path
	};
}

/*
Set the working directory.
*/
une_native_fn__(setwd)
{
	une_native_param path = 0;
	UNE_NATIVE_VERIFY_ARG_KIND(path, UNE_RK_STR);
	
	bool success = une_set_working_directory(args[path].value._wcs);
	if (!success) {
		felix->error = UNE_ERROR_SET(UNE_EK_FILE, UNE_NATIVE_POS_OF_ARG(path));
		return une_result_create(UNE_RK_ERROR);
	}
	
	return une_result_create(UNE_RK_VOID);
}

/*
Play a WAV file.
*/
une_native_fn__(playwav)
{
	une_native_param path = 0;
	UNE_NATIVE_VERIFY_ARG_KIND(path, UNE_RK_STR);
	
	char *path_narrow = une_wcs_to_str(args[path].value._wcs);
	if (!path_narrow) {
		felix->error = UNE_ERROR_SET(UNE_EK_ENCODING, UNE_NATIVE_POS_OF_ARG(path));
		return une_result_create(UNE_RK_ERROR);
	}
	bool file_not_fit = !une_file_exists(path_narrow) || !une_file_extension_matches(path_narrow, ".wav");
	free(path_narrow);
	if (file_not_fit) {
		felix->error = UNE_ERROR_SET(UNE_EK_FILE, UNE_NATIVE_POS_OF_ARG(path));
		return une_result_create(UNE_RK_ERROR);
	}
	
	return (une_result){
		.kind = UNE_RK_INT,
		.value._int = une_play_wav(args[path].value._wcs)
	};
}
