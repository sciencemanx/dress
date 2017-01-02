/*
 * File: logging.c
 * Author: Adam Van Prooyen [avanproo]
 * Description: A collection of logging functions for more information about
 *   proxy operation. Logging can be turned on an off via the verbose flag.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#include "logging.h"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RESET "\x1b[0m"

// this flag determines whether these functions actually print anything
bool verbose = false;

// pass through to printf that can be disabled by the verbose flag [white]
void print(char *msg, ...) {
	va_list args;

	if (!verbose) return;

	va_start(args, msg);

	vprintf(msg, args);
}

// logs an error to the console and quits - only for extreme errors [red]
void error(char *msg, ...) {
	va_list args;

	va_start(args, msg);

	printf("%s[!] Error: ", RED);
	vprintf(msg, args);
	printf("%s\n", RESET);
	exit(1);
}

// logs an info [green]
void info(char *msg, ...) {
	va_list args;

	if (!verbose) return;

	va_start(args, msg);

	printf("%s[+] ", GREEN);
	vprintf(msg, args);
	printf("%s\n", RESET);
}

// logs a warning [yellow]
void warn(char *msg, ...) {
	va_list args;

	if (!verbose) return;

	va_start(args, msg);

	printf("%s[-] ", YELLOW);
	vprintf(msg, args);
	printf("%s\n", RESET);
}