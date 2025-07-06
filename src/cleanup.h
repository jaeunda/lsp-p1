#ifndef CLEANUP_H
#define CLEANUP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define INPUT_SIZE 8192
#define PROMPT "20232372> "
#define ARGS_MAX 64

void parse_input(char *input, int *argc, char *argv[]);

void print_usage(int num);

int is_command_valid(int argc, char *argv[]);
int to_realpath(const char *original_path, char *resolved_path);
int set_option(int argc, char *argv[], char *output_args[]);


#endif
