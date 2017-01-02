/*
 * File: logging.h
 * Author: Adam Van Prooyen [avanproo]
 * Description: A collection of logging functions for more information about
 *   proxy operation. Logging can be turned on an off via the verbose flag.
 */

#pragma once

// use this instead of printf for logging that should be disabled [white]
void print(char *msg, ...);
// use this for fatal errors that should end program execution [red]
void error(char *msg, ...);
// use this for normal program updates [green]
void info(char *msg, ...);
// use this for minor errors (errors causing a single request to fail) [yellow]
void warn(char *msg, ...);
