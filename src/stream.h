/*
stream.h - Une
Modified 2021-08-12
*/

#ifndef UNE_STREAM_H
#define UNE_STREAM_H

/* Header-specific includes. */
#include "primitive.h"

/*
Holds information for retrieving data from an array or wide-character file.
*/
typedef struct _une_istream {
  bool has_reached_end;
  ptrdiff_t index;
  union {
    struct {
      void *array;
      size_t size;
    } array;
    struct {
      wint_t wchar;
      FILE *file;
    } wfile;
  } data;
} une_istream;

/*
Holds information for storing data in an array.
*/
typedef struct _une_ostream {
  ptrdiff_t index;
  void *array;
  size_t array_size;
  size_t item_size;
  bool allow_resize;
} une_ostream;

/*
*** Interface.
*/

/****** Array. ******/

/*
The current item in an array une_istream, with an optional offset.
*/
#define __UNE_ISTREAM_ARRAY_ITEM(__istream, __cast_type, __offset)\
  ((__cast_type*)__istream->data.array.array)[(__istream->index)+__offset]

/*
Pull the next item from an array une_istream by value.
*/
#define UNE_ISTREAM_ARRAY_PULLER_VAL(__id, __type, __cast_type, __error_value, __verify_position)\
  __une_static __type __id(une_istream *istream)\
  {\
    if (__verify_position && !une_istream_array_verify_position(istream, 1)) {\
      if (!istream->has_reached_end) {\
        istream->has_reached_end = true;\
        (istream->index)++;\
      }\
      return __error_value;\
    }\
    (istream->index)++;\
    return __UNE_ISTREAM_ARRAY_ITEM(istream, __cast_type, 0);\
  }

/*
Pull the next item from an array une_istream by reference.
*/
#define UNE_ISTREAM_ARRAY_PULLER_REF(__id, __type, __cast_type, __error_value, __verify_position)\
  __une_static __type *__id(une_istream *istream)\
  {\
    if (__verify_position && !une_istream_array_verify_position(istream, 1)) {\
      if (!istream->has_reached_end) {\
        istream->has_reached_end = true;\
        (istream->index)++;\
      }\
      return __error_value;\
    }\
    (istream->index)++;\
    return &__UNE_ISTREAM_ARRAY_ITEM(istream, __cast_type, 0);\
  }

/*
Peek an item in an array une_istream by value.
*/
#define UNE_ISTREAM_ARRAY_PEEKER_VAL(__id, __type, __cast_type, __error_value, __verify_position)\
  __une_static __type __id(une_istream *istream, ptrdiff_t offset)\
  {\
    if (__verify_position && !une_istream_array_verify_position(istream, offset))\
      return __error_value;\
    return __UNE_ISTREAM_ARRAY_ITEM(istream, __cast_type, offset);\
  }

/*
Peek an item in an array une_istream by reference.
*/
#define UNE_ISTREAM_ARRAY_PEEKER_REF(__id, __type, __cast_type, __error_value, __verify_position)\
  __une_static __type *__id(une_istream *istream, ptrdiff_t offset)\
  {\
    if (__verify_position && !une_istream_array_verify_position(istream, offset))\
      return __error_value;\
    return &__UNE_ISTREAM_ARRAY_ITEM(istream, __cast_type, offset);\
  }

/*
Access the current item in an array une_istream by value.
*/
#define UNE_ISTREAM_ARRAY_ACCESS_VAL(__id, __type, __cast_type, __error_value, __verify_position)\
  __une_static __type __id(une_istream *istream)\
  {\
    if (__verify_position && istream->has_reached_end)\
      return __error_value;\
    return __UNE_ISTREAM_ARRAY_ITEM(istream, __cast_type, 0);\
  }

/*
Access the current item in an array une_istream by reference.
*/
#define UNE_ISTREAM_ARRAY_ACCESS_REF(__id, __type, __cast_type, __error_value, __verify_position)\
  __une_static __type *__id(une_istream *istream)\
  {\
    if (__verify_position && istream->has_reached_end)\
      return __error_value;\
    return &__UNE_ISTREAM_ARRAY_ITEM(istream, __cast_type, 0);\
  }

/****** Wfile. ******/

/*
The current item in a wfile une_istream.
*/
#define __UNE_ISTREAM_WFILE_ITEM(__istream)\
  __istream->data.wfile.wchar

/*
Pull the next item from a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_PULLER(__id)\
  __une_static wint_t __id(une_istream *istream)\
  {\
    if (__UNE_ISTREAM_WFILE_ITEM(istream) != WEOF)\
      (istream->index)++;\
    __UNE_ISTREAM_WFILE_ITEM(istream) = fgetwc(istream->data.wfile.file);\
    return __UNE_ISTREAM_WFILE_ITEM(istream);\
  }

/*
Peek an item in a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_PEEKER(__id)\
  __une_static wint_t __id(une_istream *istream, ptrdiff_t offset)\
  {\
    wint_t wc_tmp = fgetwc(istream->data.wfile.file);\
    wint_t wc = wc_tmp;\
    if (offset-1 > 0)\
      wc = __id(istream, offset-1);\
    ungetwc(wc_tmp, istream->data.wfile.file);\
    return wc;\
  }

/*
Access the current item in a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_ACCESS(__id)\
  static inline wint_t __id(une_istream *istream)\
  {\
    return __UNE_ISTREAM_WFILE_ITEM(istream);\
  }

/****** Ostream. ******/

/*
The current item in a une_ostream, with an optional offset.
*/
#define __UNE_OSTREAM_ITEM(__ostream, __type, __offset)\
  ((__type*)__ostream->array)[(__ostream->index)+__offset]

/*
Push an item to a une_ostream.
*/
#define UNE_OSTREAM_PUSHER(__id, __type)\
  __une_static void __id(une_ostream *ostream, __type item)\
  {\
    if (!une_ostream_grow_if_needed(ostream, 1))\
      assert(false);\
    (ostream->index)++;\
    __UNE_OSTREAM_ITEM(ostream, __type, 0) = item;\
  }

/*
Peek an item in a une_ostream by value.
*/
#define UNE_OSTREAM_PEEKER_VAL(__id, __type, __error_value)\
  __une_static __type __id(une_ostream *ostream, ptrdiff_t offset)\
  {\
    if (!une_ostream_grow_if_needed(ostream, offset+1))\
      return __error_value;\
    return __UNE_OSTREAM_ITEM(ostream, __type, offset+1);\
  }

/*
Peek an item in a une_ostream by reference.
*/
#define UNE_OSTREAM_PEEKER_REF(__id, __type, __error_value)\
  __une_static __type *__id(une_ostream *ostream, ptrdiff_t offset)\
  {\
    if (!une_ostream_grow_if_needed(ostream, offset+1))\
      return __error_value;\
    return &__UNE_OSTREAM_ITEM(ostream, __type, offset+1);\
  }

une_istream une_istream_array_create(void *array, size_t size);
une_istream une_istream_wfile_create(char *path);
une_ostream une_ostream_create(void *array, size_t array_size, size_t item_size, bool allow_resize);

void une_istream_array_reset(une_istream *istream);
void une_istream_wfile_reset(une_istream *istream);

void une_istream_wfile_free(une_istream stream);

bool une_istream_array_verify_position(une_istream *istream, ptrdiff_t offset);
bool une_ostream_grow_if_needed(une_ostream *ostream, ptrdiff_t offset);

#endif /* !UNE_STREAM_H */
