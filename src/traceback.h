/*
traceback.h - Une
Modified 2023-12-10
*/

#ifndef UNE_TRACEBACK_H
#define UNE_TRACEBACK_H

/* Header-specific includes. */
#include "common.h"
#include "struct/module.h"
#include "struct/context.h"

void une_traceback_print_trace(une_context *context);
void une_traceback_print(void);

#endif /* !UNE_TRACEBACK_H */
