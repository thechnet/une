/*
logging.h - Une
Modified 2022-08-04
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
#define LOGGING_WIDE__ L""
#define LOGGING_OUTFN__ fwprintf
#define LOGGING_PRIS__ "%ls"
#define LOGGING_PRIHS__ "%hs"
#else
#define LOGGING_WIDE__
#define LOGGING_OUTFN__ fprintf
#define LOGGING_PRIS__ "%s"
#define LOGGING_PRIHS__ LOGGING_PRIS__
#endif

/* Format. */
#define LOGGING_STYLE_INFO__ FGBLUE
#define LOGGING_STYLE_SUCCESS__ FGGREEN
#define LOGGING_STYLE_WARN__ FGYELLOW BOLD
#define LOGGING_STYLE_FAIL__ FGRED BOLD
#define LOGGING_STYLE_OUT__ FGWHITE INVERT
#define LOGGING_WHERE LOGGING_PRIHS__ ":%d"
#define LOGGING_WHERE__ LOGGING_WHERE ": "

/* Message prefix. */
#ifndef LOGGING_ID
#define LOGGING_ID__ ""
#else
#define LOGGING_ID__ "(" LOGGING_ID ") "
#endif

/* Helper macros. */
#define LOGGING_msg__(style, msg) style LOGGING_ID__ LOGGING_WHERE__ msg "\33[0m\n"
#define LOGGING_out__(msg, ...) LOGGING_OUTFN__(stderr, LOGGING_WIDE__ msg, ##__VA_ARGS__)

#define LOGGING_logger__(file, line, style, msg, ...)\
  LOGGING_out__(LOGGING_msg__(style, msg), file, line, ##__VA_ARGS__)

/* Important information. */
#define info_at(file, line, msg, ...)\
  LOGGING_logger__(file, line, LOGGING_STYLE_INFO__, msg, ##__VA_ARGS__)
#define info(msg, ...)\
  info_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Positive information. */
#define success_at(file, line, msg, ...)\
  LOGGING_logger__(file, line, LOGGING_STYLE_SUCCESS__, msg, ##__VA_ARGS__)
#define success(msg, ...)\
  success_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Negative information. */
#define warn_at(file, line, msg, ...)\
  LOGGING_logger__(file, line, LOGGING_STYLE_WARN__, msg, ##__VA_ARGS__)
#define warn(msg, ...)\
  warn_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Fatal information. */
#define fail_at(file, line, msg, ...)\
  do {\
    LOGGING_logger__(file, line, LOGGING_STYLE_FAIL__, msg, ##__VA_ARGS__);\
    abort();\
  } while (0)
#define fail(msg, ...)\
  fail_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)

/* State information. */
#define out_at(file, line, msg, ...)\
  LOGGING_logger__(file, line, LOGGING_STYLE_OUT__, msg, ##__VA_ARGS__)
#define out(msg, ...)\
  out_at(__FILE__, __LINE__, msg, ##__VA_ARGS__)
#define outi(int_)\
  out(LOGGING_PRIS__ "=%lld", LOGGING_WIDE__ #int_, (long long)int_)
#define outf(float_)\
  out(LOGGING_PRIS__ "=%f", LOGGING_WIDE__ #float_, (double)float_)
#define outc(c)\
  out(LOGGING_PRIS__ "='%c'", LOGGING_WIDE__ #c, c)
#define outs(s)\
  out(LOGGING_PRIS__ "=\"" LOGGING_PRIS__ "\"", LOGGING_WIDE__ #s, s)
#ifdef LOGGING_WIDE
#define ouths(str)\
  out(LOGGING_PRIS__ "=\"%hs\"", LOGGING_WIDE__ #str, str)
#endif

#endif /* !LOGGING_H */
