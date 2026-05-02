/*
main.h - Une
*/

#ifndef UNE_MAIN_H
#define UNE_MAIN_H

/* Header-specific includes. */
#include "signal.h"
#include "struct/engine.h"

extern volatile sig_atomic_t sigint_fired;

int main(int argc, char *argv[]);
void interactive_sigint_handler(int signal);
void interactive(void);
void print_usage(char *executable_path);

#endif /* !UNE_MAIN_H */
