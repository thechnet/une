/*
logging.h - Une
Modified 2021-07-12
*/

#ifndef LOGGING_H
#define LOGGING_H

/* Header-specific includes. */
#include <stdio.h>
#include <stdlib.h>
#ifdef LOGGING_WIDE
#define ESCSEQ_WIDE
#include <wchar.h>
#endif
#include "escseq.h"

/* Character-width differences. */
#ifdef LOGGING_WIDE
#define __LOGGING_WIDE L""
#define __LOGGING_OUTFN fwprintf
#define __LOGGING_PRIS "%ls"
#define __LOGGING_PRIHS "%hs"
#else
#define __LOGGING_WIDE
#define __LOGGING_OUTFN fprintf
#define __LOGGING_PRIS "%s"
#define __LOGGING_PRIHS __LOGGING_PRIS
#endif

/* Format. */
#define __LOGGING_STYLE_INFO FGBLUE
#define __LOGGING_STYLE_SUCCESS FGGREEN
#define __LOGGING_STYLE_WARN FGYELLOW BOLD
#define __LOGGING_STYLE_FAIL FGRED BOLD
#define __LOGGING_STYLE_OUT FGWHITE INVERT
#define LOGGING_WHERE __LOGGING_PRIHS ":%d"
#define __LOGGING_WHERE LOGGING_WHERE ": "

/* Message prefix. */
#ifndef LOGGING_ID
#define __LOGGING_ID ""
#else
#define __LOGGING_ID "(" LOGGING_ID ") "
#endif

/* Helper macros. */
#define __LOGGING_msg(style, msg) style __LOGGING_ID __LOGGING_WHERE msg "\33[0m\n"
#define __LOGGING_out(msg, ...) __LOGGING_OUTFN(stderr, __LOGGING_WIDE msg, ##__VA_ARGS__)

#define __LOGGING_logger(file, line, style, msg, ...)\
  __LOGGING_out(__LOGGING_msg(style, msg), file, line, ##__VA_ARGS__)

/* Important information. */
#define info_at(file, line, msg, ...)\
  __LOGGING_logger(file, line, __LOGGING_STYLE_INFO, msg, ##__VA_ARGS__)
#define info(msg, ...)\
  info_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Positive information. */
#define success_at(file, line, msg, ...)\
  __LOGGING_logger(file, line, __LOGGING_STYLE_SUCCESS, msg, ##__VA_ARGS__)
#define success(msg, ...)\
  success_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Negative information. */
#define warn_at(file, line, msg, ...)\
  __LOGGING_logger(file, line, __LOGGING_STYLE_WARN, msg, ##__VA_ARGS__)
#define warn(msg, ...)\
  warn_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Fatal information. */
#define fail_at(file, line, msg, ...)\
  {\
    __LOGGING_logger(file, line, __LOGGING_STYLE_FAIL, msg, ##__VA_ARGS__);\
    exit(1);\
  }
#define fail(msg, ...)\
  fail_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* State information. */
#define out_at(file, line, msg, ...)\
  __LOGGING_logger(file, line, __LOGGING_STYLE_OUT, msg, ##__VA_ARGS__)
#define out(msg, ...)\
  out_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)
#define outi(int_)\
  out(__LOGGING_PRIS "=%lld", __LOGGING_WIDE #int_, (long long)int_)
#define outf(float_)\
  out(__LOGGING_PRIS "=%f", __LOGGING_WIDE #float_, (double)float_)
#define outc(c)\
  out(__LOGGING_PRIS "='%c'", __LOGGING_WIDE #c, c)
#define outs(s)\
  out(__LOGGING_PRIS "=\"" __LOGGING_PRIS "\"", __LOGGING_WIDE #s, s)
#ifdef LOGGING_WIDE
#define ouths(str)\
  out(__LOGGING_PRIS "=\"%hs\"", __LOGGING_WIDE #str, str)
#endif

#endif /* !LOGGING_H */
