/*
traceback.h - Une
*/

#ifndef UNE_TRACEBACK_H
#define UNE_TRACEBACK_H

/* Header-specific includes. */
#include "common.h"
#include "struct/context.h"
#include "struct/module.h"

void une_traceback_print_trace(une_context *context);
void une_traceback_print(void);

#endif /* !UNE_TRACEBACK_H */
