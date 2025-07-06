#include "tree.h"

int file_cnt=0;
int dir_cnt=0;

int main(int argc, char *argv[]){
	if (argc!=3){ 
		print_usage();
		exit(1);
	}
	char *input_path=NULL;
	if (argv[1]==NULL){
		fprintf(stderr, "main: path is null\n");
		exit(1);
	} 
	input_path=argv[1];

	struct option ops= {0, 0, 0, 0, 0} ;
	if (argv[2]==NULL){
		fprintf(stderr, "main: option bit is null\n");
		exit(1);
	}
	// set option
	if (set_option(&ops, argv[2])<0){
		fprintf(stderr, "main: set option error\n");
		exit(1);
	}

	printf("%s\n", input_path);
	// input path를 절대 경로로 변환
	char resolved_path[PATH_MAX+1];
	if (to_realpath(input_path, resolved_path)<0){
		fprintf(stderr,"Error: path error\n");
		exit(1);
	}

	// direcory 탐색&출력
	dir_cnt=1;
	if (scan_dir(resolved_path, ops, 0)<0){
		fprintf(stderr, "main: scan_dir error\n");
		exit(1);
	}

	// 파일, 디렉토리 개수 출력
	printf("%d directories, %d files\n", dir_cnt, file_cnt);
	// 프로세스 종료
	exit(0);
}
int set_option(struct option *ops, const char *selected_option){
	// sp 00 01 10 11
	if (selected_option == NULL){
		fprintf(stderr, "set_option: option bit is null\n");
		return -1;
	}
	if (ops  == NULL){
		fprintf(stderr, "set_option: struct option is null\n");
		return -1;
	}

	int option_int = atoi(selected_option);
	// option -s 선택 여부
	if ((option_int/10) == 1){
		ops->option_s=1;
	} else if ((option_int/10) == 0){
		ops->option_s=0;
	} else {
		fprintf(stderr, "set_option: option -s bit error\n");
		return -1;
	}
	// option -p 선택 여부
	if ((option_int%10) == 1){
		ops->option_p=1;
	} else if ((option_int%10) == 0){
		ops->option_p=0;
	} else {
		fprintf(stderr, "set_option: option -p bit error\n");
		return -1;
	}
	return 0;
}
int scan_dir(const char *cur_path, const struct option opt, int depth){
	struct dirent **namelist;
	int n;
	// system call: scan directory
	if ((n=scandir(cur_path, &namelist, NULL, alphasort))<0){
		// n: 파일 또는 디렉토리의 개수
		fprintf(stderr, "scandir error\n");
		return -1;
	}

	// namelist: print file or directory name
	for (int i=0; i<n; i++){
		// 현재 디렉토리와 부모 디렉토리 제외
		if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..")==0)
			continue;
		// 숨김파일 제외
		if (namelist[i]->d_name[0] == '.') 
			continue;
		// namelist[i]->d_name ...
		// 트리 구조 출력
		for (int k=0; k<depth; k++){
			if (k==0 && opt.is_root==1 && opt.is_root_last==0)
				printf("│   ");
			else if (k == (depth-1) && opt.is_parent_last == 0)
				printf("│   ");
			else 
				printf("    ");
		}
		if (i==n-1) 
			printf("└─");
		else 
			printf("├─");
	
		// stat information
		struct stat sb;
		
		// child path
		char child_path[PATH_MAX+1];
		if (cur_path == NULL){
			fprintf(stderr, "scan_dir: cur_path is null\n");
			return -1;
		}
		if (snprintf(child_path, sizeof(child_path), "%s/%s", cur_path, namelist[i]->d_name)<0){
			fprintf(stderr, "scan_dir: child_path too long\n");
			return -1;
		}

		// 파일의 stat information 불러오기
		if (stat(child_path,&sb) <0){
			fprintf(stderr, "scan_dir: stat error\n");
			return -1;
		}

		// 디렉토리일 경우 재귀 탐색
		if (S_IFDIR & sb.st_mode){
			// 디렉토리 이름 출력
			print_name(namelist[i]->d_name, sb, opt);
			// 디렉토리 개수 update
			dir_cnt++;
			// child option 초기화
			struct option child_opt = {opt.option_s, opt.option_p, 0, opt.is_root, opt.is_root_last};
			if (i==n-1 && depth==0)
				child_opt.is_root_last=1;
			if (i==n-1)
				child_opt.is_parent_last=1;
			if (depth==0)
				child_opt.is_root=1;
			// 재귀 탐색
			scan_dir(child_path, child_opt, depth+1);
		} else {// 파일일 경우
			print_name(namelist[i]->d_name, sb, opt);
			file_cnt++;
		}
	}

	// namelist 동적 할당 해제
	for (int i=0; i<n; i++){
		free(namelist[i]);
	}
	free(namelist);
}
void print_name(const char *name, const struct stat sb, const struct option opt){
	// 파일 혹은 디렉토리의 권한을 string 형태로 변환
	char mode[11]; // -p
	mode_to_string(sb.st_mode, mode);
	// 옵션에 따라 파일 혹은 디렉토리 이름 출력
	if ( opt.option_s == 1 && opt.option_p == 1)
		printf("[%s %ld] ", mode, sb.st_size);
	else if (opt.option_s)
		printf("[%ld] ", sb.st_size);
	else if (opt.option_p)
		printf("[%s] ", mode);
	if (S_IFDIR & sb.st_mode)
		printf("%s/\n", name);
	else
		printf("%s\n", name);
}
void mode_to_string(mode_t mode, char *mode_string){
	// 파일 혹은 디렉토리의 권한을 string 형태로 변환
	// type
	if ((S_IFMT & mode) == S_IFDIR) mode_string[0]='d';
	else if ((S_IFMT & mode) == S_IFREG) mode_string[0]='-';
	else if ((S_IFMT & mode) == S_IFLNK) mode_string[0]='l';
	else if ((S_IFMT & mode) == S_IFBLK) mode_string[0]='b';
	else if ((S_IFMT & mode) == S_IFCHR) mode_string[0]='c';
	else if ((S_IFMT & mode) == S_IFIFO) mode_string[0]='p';
	else if ((S_IFMT & mode) == S_IFSOCK) mode_string[0]='s';

	// user
	mode_string[1]=(S_IRUSR & mode) ? 'r':'-';
	mode_string[2]=(S_IWUSR & mode) ? 'w':'-';
	mode_string[3]=(S_IXUSR & mode) ? 'x':'-';

	// group
	mode_string[4]=(S_IRGRP & mode) ? 'r':'-';
	mode_string[5]=(S_IWGRP & mode) ? 'w':'-';
	mode_string[6]=(S_IXGRP & mode) ? 'x':'-';
	// other
	mode_string[7]=(S_IROTH & mode) ? 'r':'-';
	mode_string[8]=(S_IWOTH & mode) ? 'w':'-';
	mode_string[9]=(S_IXOTH & mode) ? 'x':'-';

	// NUL
	mode_string[10]='\0';
}
void print_usage(){
	const char *usage[]={
		"Usage:\n",
        "\t> tree <DIR_PATH> [OPTION]...\n",
        "\t\t<none> : Display the directory structure recursively if <DIR_PATH> is a     directory\n",
        "\t\t-s : Display the directory structure recursively if <DIR_PATH> is a dir    ectory, including the size of each file\n",
        "\t\t-p : Display the directory structure recursively if <DIR_PATH> is a dir    ectory, including the permissions of each directory and file\n"
	};
    for (int i=0;i<5;i++) printf("%s",usage[i]);
    return;
}
int to_realpath(const char *original_path, char *resolved_path){
	// ~로 시작하는 경로는 환경변수를 통해 홈 디렉토리 경로로 변환
	// 상대경로는 절대경로로 변환
	// 변환 성공 시 0, 실패 시 -1 return

	int error_code;
        char temp_path[PATH_MAX+1];
        strcpy(temp_path, original_path);
        // 사용자 홈 디렉토리
        const char *home = getenv("HOME"); 
        if (home == NULL){
            home = getpwuid(getuid())->pw_dir;
        }

        // ~로 시작하는 경로
        if (original_path[0] == '~'){
            if (snprintf(temp_path, PATH_MAX, "%s%s", home, original_path+1)<0){
                fprintf(stderr, "Error: Path too long\n");
                return -1; 
            }
        }
        if (realpath(temp_path, resolved_path) !=NULL){
            // 홈 디렉토리 검사
            if (strncmp(resolved_path, home, strlen(home))!=0){
                fprintf(stderr, "Error: %s is outside the home directory\n", original_path);
                return -1; 
            }
            // 디렉토리인지 검사
            struct stat sb; 
            if (stat(resolved_path, &sb)<0){
                fprintf(stderr, "Error: stat error\n");
                return -1; 
            }
            if ((sb.st_mode & S_IFMT)!=S_IFDIR){
                fprintf(stderr, "Error: %s is not a directory\n", original_path);
                return -1; 
            }
            return 1;
        } else if ((error_code=errno) == EACCES){
            fprintf(stderr, "Error: Read or search permission denied\n");
            return -1; 
        } else if (error_code == ENAMETOOLONG){
            fprintf(stderr, "Error: File name too long\n");
            return -1; 
        } else if (error_code == ENOENT){
            fprintf(stderr, "Error: %s does not exist.\n", original_path);
            return -1; 
        } else if (error_code == ENOTDIR){
            fprintf(stderr, "Error: Invalid path\n");
            return -1; 
        } else if (error_code == EINVAL){
            fprintf(stderr, "Error: Path is null\n");
            return -1; 
        } else return 0;
    
        return 1;

}
