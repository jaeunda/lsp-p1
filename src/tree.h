#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

struct option{
    int option_s;
    int option_p;
    int is_parent_last;
    int is_root;
    int is_root_last;
};

void print_usage();
int set_option(struct option *ops, const char *selected_option);
int scan_dir(const char *cur_path, const struct option opt, int depth); 
void print_name(const char *name, const struct stat sb, const struct option opt);
void mode_to_string(mode_t mode, char *mode_string);
int to_realpath(const char *original_path, char *resolved_path);


#endif

