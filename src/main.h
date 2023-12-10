/*
main.h - Une
Modified 2023-12-10
*/

#ifndef UNE_MAIN_H
#define UNE_MAIN_H

/* Header-specific includes. */
#include "struct/engine.h"
#include "signal.h"

enum une_main_action {
	SHOW_USAGE,
	RUN_SCRIPT_FILE,
	RUN_SCRIPT_STRING,
	ENTER_INTERACTIVE_MODE
};

extern volatile sig_atomic_t sigint_fired;

int main(int argc, char *argv[]);
void interactive_sigint_handler(int signal);
void interactive(void);
void print_usage(char *executable_path);

#endif /* !UNE_MAIN_H */
