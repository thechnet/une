/*
stream.h - Une
Modified 2023-02-22
*/

#ifndef UNE_STREAM_H
#define UNE_STREAM_H

/* Header-specific includes. */
#include "primitive.h"

/*
Holds information for retrieving data from an array or wide-character file.
*/
typedef struct une_istream_ {
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
typedef struct une_ostream_ {
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
#define UNE_ISTREAM_ARRAY_ITEM__(istream__, cast_type__, offset__)\
  ((cast_type__*)istream__->data.array.array)[(istream__->index)+offset__]

/*
Pull the next item from an array une_istream by value.
*/
#define UNE_ISTREAM_ARRAY_PULLER_VAL(id__, type__, cast_type__, error_value__, verify_position__)\
  une_static__ type__ id__(une_istream *istream)\
  {\
    if (verify_position__ && !une_istream_array_verify_position(istream, 1)) {\
      if (!istream->has_reached_end) {\
        istream->has_reached_end = true;\
        (istream->index)++;\
      }\
      return error_value__;\
    }\
    (istream->index)++;\
    return UNE_ISTREAM_ARRAY_ITEM__(istream, cast_type__, 0);\
  }

/*
Pull the next item from an array une_istream by reference.
*/
#define UNE_ISTREAM_ARRAY_PULLER_REF(id__, type__, cast_type__, error_value__, verify_position__)\
  une_static__ type__ *id__(une_istream *istream)\
  {\
    if (verify_position__ && !une_istream_array_verify_position(istream, 1)) {\
      if (!istream->has_reached_end) {\
        istream->has_reached_end = true;\
        (istream->index)++;\
      }\
      return error_value__;\
    }\
    (istream->index)++;\
    return &UNE_ISTREAM_ARRAY_ITEM__(istream, cast_type__, 0);\
  }

/*
Peek an item in an array une_istream by value.
*/
#define UNE_ISTREAM_ARRAY_PEEKER_VAL(id__, type__, cast_type__, error_value__, verify_position__)\
  une_static__ type__ id__(une_istream *istream, ptrdiff_t offset)\
  {\
    if (verify_position__ && !une_istream_array_verify_position(istream, offset))\
      return error_value__;\
    return UNE_ISTREAM_ARRAY_ITEM__(istream, cast_type__, offset);\
  }

/*
Peek an item in an array une_istream by reference.
*/
#define UNE_ISTREAM_ARRAY_PEEKER_REF(id__, type__, cast_type__, error_value__, verify_position__)\
  une_static__ type__ *id__(une_istream *istream, ptrdiff_t offset)\
  {\
    if (verify_position__ && !une_istream_array_verify_position(istream, offset))\
      return error_value__;\
    return &UNE_ISTREAM_ARRAY_ITEM__(istream, cast_type__, offset);\
  }

/*
Access the current item in an array une_istream by value.
*/
#define UNE_ISTREAM_ARRAY_ACCESS_VAL(id__, type__, cast_type__, error_value__, verify_position__)\
  une_static__ type__ id__(une_istream *istream)\
  {\
    if (verify_position__ && istream->has_reached_end)\
      return error_value__;\
    return UNE_ISTREAM_ARRAY_ITEM__(istream, cast_type__, 0);\
  }

/*
Access the current item in an array une_istream by reference.
*/
#define UNE_ISTREAM_ARRAY_ACCESS_REF(id__, type__, cast_type__, error_value__, verify_position__)\
  une_static__ type__ *id__(une_istream *istream)\
  {\
    if (verify_position__ && istream->has_reached_end)\
      return error_value__;\
    return &UNE_ISTREAM_ARRAY_ITEM__(istream, cast_type__, 0);\
  }

/****** Wfile. ******/

/*
The current item in a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_ITEM__(istream__)\
  istream__->data.wfile.wchar

/*
Pull the next item from a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_PULLER(id__)\
  une_static__ wint_t id__(une_istream *istream)\
  {\
    if (UNE_ISTREAM_WFILE_ITEM__(istream) != WEOF)\
      (istream->index)++;\
    UNE_ISTREAM_WFILE_ITEM__(istream) = fgetwc(istream->data.wfile.file);\
    return UNE_ISTREAM_WFILE_ITEM__(istream);\
  }

/*
Peek an item in a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_PEEKER(id__)\
  une_static__ wint_t id__(une_istream *istream, ptrdiff_t offset)\
  {\
    assert(offset >= 0);\
    if (!offset)\
      return UNE_ISTREAM_WFILE_ITEM__(istream);\
    wint_t wc_tmp = fgetwc(istream->data.wfile.file);\
    wint_t wc = wc_tmp;\
    if (offset-1 > 0)\
      wc = id__(istream, offset-1);\
    ungetwc(wc_tmp, istream->data.wfile.file);\
    return wc;\
  }

/*
Access the current item in a wfile une_istream.
*/
#define UNE_ISTREAM_WFILE_ACCESS(id__)\
  static inline wint_t id__(une_istream *istream)\
  {\
    return UNE_ISTREAM_WFILE_ITEM__(istream);\
  }

/****** Ostream. ******/

/*
The current item in a une_ostream, with an optional offset.
*/
#define UNE_OSTREAM_ITEM__(ostream__, type__, offset__)\
  ((type__*)ostream__->array)[(ostream__->index)+offset__]

/*
Push an item to a une_ostream.
*/
#define UNE_OSTREAM_PUSHER(id__, type__)\
  une_static__ void id__(une_ostream *ostream, type__ item)\
  {\
    if (!une_ostream_grow_if_needed(ostream, 1))\
      assert(false);\
    (ostream->index)++;\
    UNE_OSTREAM_ITEM__(ostream, type__, 0) = item;\
  }

/*
Peek an item in a une_ostream by value.
*/
#define UNE_OSTREAM_PEEKER_VAL(id__, type__, error_value__)\
  une_static__ type__ id__(une_ostream *ostream, ptrdiff_t offset)\
  {\
    if (!une_ostream_grow_if_needed(ostream, offset+1))\
      return error_value__;\
    return UNE_OSTREAM_ITEM__(ostream, type__, offset+1);\
  }

/*
Peek an item in a une_ostream by reference.
*/
#define UNE_OSTREAM_PEEKER_REF(id__, type__, error_value__)\
  une_static__ type__ *id__(une_ostream *ostream, ptrdiff_t offset)\
  {\
    if (!une_ostream_grow_if_needed(ostream, offset+1))\
      return error_value__;\
    return &UNE_OSTREAM_ITEM__(ostream, type__, offset+1);\
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
