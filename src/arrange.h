#ifndef ARRANGE_H
#define ARRANGE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define EXPTH_CNT 128
#define EXT_CNT 128
#define EXT_MAX 32
#define BUFFER_SIZE 4096


// linked-list
typedef struct FileNode{
    char filename[NAME_MAX+1]; // 파일명
    char path[PATH_MAX+1]; // 파일 복사를 위한 절대 경로
    struct FileNode *next; // 다음 파일 노드 (같은 확장자)
} FileNode;
typedef struct ExtNode{
    char extension[EXT_MAX]; // 확장자명
    FileNode *files; // 해당 확장자 파일 리스트
    struct ExtNode *next; // 다음 확장자 노드
} ExtNode;



int set_option(int argc, char **argv);
int to_realpath(const char *original_path, char *resolved_path);

int scan_dir(const char *cur_path, int depth);
int split_filename(const char *filename, char *extension);
int is_excluded(const char *path);
int is_valid_extension(const char *extension);
int is_recent(time_t modified_time);

int add_extension(const char *extension);
int add_files(const char* extension, const char *filename, const char *path);
int handle_dup(FileNode *old, FileNode *new);
int find_extension(const char *extension);

int make_dir(const char *cur_path);
int copy_file(const char *file_path, const char *filename, const char *dir_path); 

struct option {
    int option_d;
    int option_t;
    int option_x;
    int option_e;

    char output_path[PATH_MAX+1];
    time_t seconds;
    char *exclude_path[EXPTH_CNT];
    char *extension[EXT_CNT];

    int expath_count;
    int ext_count;
};



#endif
