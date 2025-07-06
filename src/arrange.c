#include "arrange.h"

// head of linked list(global)
	ExtNode *head = NULL;
// struct option(global) 초기
	struct option opt={0};

int main(int argc, char *argv[]){
	// set option
	if (set_option(argc, argv)<0){
		fprintf(stderr, "main: set option error\n");
		exit(1);
	}
	
	char input_path[PATH_MAX+1]; // MAX: 4096
	
	if (argv[2]!=NULL && strlen(argv[2])<(PATH_MAX+1)){
		strcpy(input_path, argv[2]);
	} else {
		fprintf(stderr, "main: input path error\n");
		exit(1);
	}	
	
	// to realpath
	char resolved_path[PATH_MAX+1];
	if (to_realpath(input_path, resolved_path)<0){
		fprintf(stderr, "Error: path error\n");
		exit(1);
	}
	// scan directory
	// root: input_path
	if (scan_dir(resolved_path, 0)<0){
		fprintf(stderr, "main: scan directory error\n");
		exit(1);
	}

	// make arranged directory
	if (make_dir(resolved_path)<0){
		fprintf(stderr, "main: make directory error\n");
		exit(1);
	}

	// arrange success
	fprintf(stdout, "%s arranged\n", input_path);
	
	// strtok() free
	for (int i=0; i<opt.expath_count; i++){
		free(opt.exclude_path[i]);
	}
	for (int i=0; i<opt.ext_count; i++){
		free(opt.extension[i]);
	}

	// 프로세스 종료
	exit(0);
}
int set_option(int argc, char **argv){
	if (argc!=7)
		return -1;
	
	char *option_bit=NULL;
	if (argv[1]!=NULL){
		option_bit=argv[1];
	} else {
		fprintf(stderr, "set_option: option_bit error\n");
		return -1;
	}
	// option -d: output_path
	if (option_bit[0]=='1'){
		char output_path[PATH_MAX+1];
		opt.option_d=1;
		if (argv[3]==NULL) return -1;
		
		// struct option에 <output_path> 저장
		if (strcmp(argv[3],"-")!=0 && strlen(argv[3])<=PATH_MAX){
			strncpy(output_path, argv[3], (size_t)PATH_MAX+1);
		} else {
			fprintf(stderr, "set_option: set option -d error\n");
			return -1;
		}
		if (output_path[0]=='~'){
			char temp_path[PATH_MAX+1];
			strcpy(temp_path, output_path);
			const char *home = getenv("HOME");
			if (home == NULL)
				home=getpwuid(getuid())->pw_dir;
			if (snprintf(output_path, (size_t)PATH_MAX+1, "%s%s", home, temp_path+1)<0){
				fprintf(stderr, "Error: Path too long\n");
				return -1;
			}
		}
		strncpy(opt.output_path, output_path, (size_t)PATH_MAX+1);

	}
	// option -t: seconds
	if (option_bit[1]=='1'){
		opt.option_t=1;
		// struct option에 사용자가 입력한 <seconds> 저장
		if (argv[4]!=NULL && strcmp(argv[4], "-")!=0){
			opt.seconds=(time_t)atoi(argv[4]);
		} else {
			fprintf(stderr, "set_option: set option -t error\n");
			return -1;
		}
	}
	// option -x: exclude_path
	if (option_bit[2]=='1'){
		opt.option_x=1;
		if (argv[5]!=NULL && strcmp(argv[5], "-")!=0){
			// argument parsing
			char *copy=strdup(argv[5]);
			char *token=strtok(copy, " ");
			int count=0;

			// struct option에 사용자가 입력한 <exclude_path> 저장
			while (token != NULL && count<EXPTH_CNT) {
				char *temp=strdup(token);
				char exclude_realpath[PATH_MAX+1];
				if (to_realpath(temp, exclude_realpath)<0){
					fprintf(stderr, "Error: path error\n");
					exit(1);
				}
				opt.exclude_path[count]=strdup(exclude_realpath);
				if (opt.exclude_path[count] == NULL){
					fprintf(stderr, "exclude path is NULL\n");
					return -1;
				}
				count++;
				token=strtok(NULL, " ");
				free(temp);
			}
			opt.expath_count=count;
			free(copy);
		} else {
			fprintf(stderr, "set_option: set option -x error\n");
			return -1;
		}
	}
	// option -e: extensions
	if (option_bit[3]=='1'){
		opt.option_e=1;	
		if (argv[6]!=NULL && strcmp(argv[6], "-")!=0){
			// argument parsing
			char *copy=strdup(argv[6]);
			char *token=strtok(copy, " ");
			int count=0;

			// struct option에 사용자가 입력한 <extention> 저장
			while (token != NULL && count<EXT_CNT) {
				opt.extension[count++]=strdup(token);
				token=strtok(NULL, " ");
			}
			opt.ext_count=count;
			free(copy);
		} else {
			fprintf(stderr, "set_option: set option -e error\n");
			return -1;
		}
	}
	return 0;
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
            return 0;
        } else if ((error_code=errno) == EACCES){
            fprintf(stderr, "Error: Read or search permission denied\n");
            return -1;
        } else if (error_code == ENAMETOOLONG){
            fprintf(stderr, "Error: File name too long\n");
            return -1;
        } else if (error_code == ENOTDIR){
            fprintf(stderr, "Error: Invalid path\n");
            return -1;
        } else if (error_code == ENOENT){
			fprintf(stderr, "Error: %s does not exist\n", original_path);
			return -1;
		} else if (error_code == EINVAL){
            fprintf(stderr, "Error: Path is null\n");
            return -1;
        } else return -1;

        return 0;

}
int scan_dir(const char *cur_path, int depth){
    struct dirent **namelist;
    int n=-1;
    // system call: scan directory
	// scandir()는 목록을 namelist에 동적으로 할당하여 저장 
    if ((n=scandir(cur_path, &namelist, NULL, alphasort))<0){
        fprintf(stderr, "scan_dir: scandir error\n");
		return -1;
    } else if (n==0){
		fprintf(stdout, "scan_dir: There is no file or diretory in %s\n", cur_path);
		exit(0);
	}
	// n: 파일 또는 디렉토리의 개수
    for (int i=0; i<n; i++){
        // 현재 디렉토리와 부모 디렉토리 제외
		if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..")==0)
            continue;
        // 숨김파일 제외
        if (namelist[i]->d_name[0] == '.')
            continue;
        // namelist[i]->d_name (member of struct dirent)
        struct stat sb; // file의 정보를 담은 struct stat
        
		// 현재 디렉토리의 하위 파일 경로
        char child_path[PATH_MAX+1]; // MAX 4096
        if (cur_path==NULL){
			fprintf(stderr, "scan_dir: cur_path is NULL\n");
			return -1;
		}
		if (strlen(namelist[i]->d_name) > (size_t)PATH_MAX){
			fprintf(stderr, "Error: file name too long\n");
		}
		if (snprintf(child_path,(size_t)(PATH_MAX+1),"%s/%s", cur_path, namelist[i]->d_name)>PATH_MAX){
			fprintf(stderr, "Error: path too long\n");
			return -1;
		}

        // 하위 파일의 stat information 불러오기
		if (stat(child_path, &sb) <0){
            fprintf(stderr, "scan_dir: stat error\n");
			return -1;
        }
        // struct stat를 통해 파일의 유형 확인
		if (S_IFDIR & sb.st_mode){
			// option_x: exclude_path에 해당할 경우 탐색하지 않음
			if (opt.option_x==1 && is_excluded(child_path)==1){
				continue;
			}
			// 디렉토리일 경우 재귀 탐색
			if (scan_dir(child_path, depth+1)<0){
				fprintf(stderr, "%s: scan directory recursion error\n", child_path);
				exit(1);
        	}
			continue; // linked list에 추가하지 않고 다음 루프로
		}
		// 디렉토리가 아닐 경우
			// option_t: time 조건에 부합하지 않을 경우 추가하지 않음
			if (opt.option_t==1 && is_recent(sb.st_mtime)==0){
				continue;
			}
		// parsing -> name, extension
			char filename[NAME_MAX+1];
			char extension[EXT_MAX];
			// 파일의 경로:  char child_path[]
			
			// filename에서 extension 분리
			strcpy(filename, namelist[i]->d_name);
			if (split_filename(filename, extension)<0){
				fprintf(stderr, "scan_dir: split filename error\n");
				return -1;
			}
			// option_e: extension 조건에 부합하지 않을 경우 추가하지 않음
			if (opt.option_e == 1 && is_valid_extension(extension)==0){
				continue;
			}
		// 디렉토리 구조:  Linked List로 관리
			// find_extension(): List에 해당 확장자 node가 존재하는지 확인
			int is_exist=0;
			if ((is_exist=find_extension(extension))<0){
				fprintf(stderr, "scan_dir: find extension error\n");
				return -1;
			}
			// add_files(): extension node가 이미 존재하면 file node 추가
			if (is_exist){
				if (add_files(extension, filename, child_path)<0){
					fprintf(stderr, "%s: add files in list error\n", filename);
					return -1;
				}
			}
			// add_extension(): 존재하지 않으면 extension node 추가 후 파일 추가
			else {
				// extension node 추가
				if (add_extension(extension)<0){
					fprintf(stderr, "%s: add extension in list error\n", filename);
					return -1;
				}
				// file node 추가
				if (add_files(extension, filename, child_path)<0){
					fprintf(stderr, "%s: add files in list error\n", filename);
					return -1;
				}
			}
	}
	// 탐색 종료 후 namelist 할당 해제
    for (int i=0; i<n; i++)
		free(namelist[i]);
	free(namelist);

	return 0;
}
int split_filename(const char *filename, char *extension){
	// 파일 이름에서 .을 기준으로 확장자명 분리
	// split 성공 시 0, 실패 시 -1 return

	char temp[NAME_MAX+1];
	// strtok()는 원본 문자열을 수정하므로 temp 변수 생성하여 복사
	if (filename != NULL){
		strcpy(temp, filename);
	} else {
		fprintf(stderr, "split_filename: filename is NULL\n");
		return -1;
	}
	// filename: name.extension
	char *token=strtok(temp, "."); // name
	if (token==NULL){
		// 이름 없음
		return -1;
	}

	token=strtok(NULL, "."); // extension
	if (token == NULL){ // 확장자가 존재하지 않는 경우
		strcpy(extension, "no_extension");
		return 0;
	} else {
		strcpy(extension, token);
		return 0;
	}
}
int is_excluded(const char *path){
	for (int i=0; i<opt.expath_count; i++){
		if (opt.exclude_path[i] == NULL){
			fprintf(stderr, "is_excluded: exclude_path is NULL\n");
			exit(1);
		}
		if (strcmp(path, opt.exclude_path[i])==0)
			return 1;
	}
	return 0;
}
int is_valid_extension(const char *extension){
	for (int i=0; i<opt.ext_count; i++){
		if (strcmp(extension, opt.extension[i])==0){
			return 1;
		}
	}
	return 0;
}
int is_recent(time_t modified_time){
	time_t now=time(NULL);
	double seconds_ago = difftime(now, modified_time);
	// 마지막으로 수정한 시간이 <seconds>초 이내이면 return 1;
	if (seconds_ago <= opt.seconds)
		return 1;
	// <seconds>초가 지났으면 return 0;
	else return 0;
}
int add_extension(const char *extension){
// list에 해당 extension node 추가
// 성공 시 0, 실패 시 -1 return
	// 새로운 extension node
	ExtNode *new_ext=(ExtNode *)malloc(sizeof(ExtNode));
	// node 내 value 초기화
	if (extension == NULL){
		fprintf(stderr, "add_extension: extension is NULL\n");
		return -1;
	}
	strcpy(new_ext->extension, extension);
	new_ext->files=NULL;
	new_ext->next=NULL;
	// head가 NULL인 경우, 새로운 node를 head에 링크
	if (head == NULL){
		head=new_ext;
		return 0;
	}

	ExtNode *temp=head;
	// linked list 끝(next==NULL)에 도달할 때까지
	while (temp->next != NULL){
		temp=temp->next;
	}
	temp->next=new_ext;
	
	return 0;
}
int add_files(const char* extension, const char *filename, const char *path){
// file 정보를 담은 file node 생성
// 해당 extension node에 링크
// 성공 시 0, 실패 시 -1 return
	// 새로운 file node 생성
	FileNode *new_file=(FileNode *)malloc(sizeof(FileNode));
	
	if (filename == NULL || extension == NULL || path == NULL)
		return -1;
	if (strlen(filename) > (NAME_MAX+1)){
		fprintf(stderr, "Error: File name too long\n");
		exit(1);
	}
	if (strlen(path) > (PATH_MAX+1)){
		fprintf(stderr, "Error: Path too long\n");
		exit(1);
	}
	// file node 초기화
	strcpy(new_file->filename, filename);
	strcpy(new_file->path, path);
	new_file->next=NULL;

	// extension node 탐색
	if (head==NULL){
		return -1;
	}
	ExtNode *temp_ext=head;
	int is_exist=0;
	do {
		if (strcmp(temp_ext->extension, extension)==0){
			is_exist=1;
			break;
		}
		temp_ext=temp_ext->next;
	} while (temp_ext!=NULL);
	if (is_exist == 0){
		fprintf(stderr, "%s: extension does not exists\n", filename);
		return -1;
	}
	// file node 추가
	// file node가 존재하지 않을 경우
	if (temp_ext->files == NULL){
		temp_ext->files = new_file;
		return 0;
	}
	// file node가 이미 존재할 경우
	// file list 끝에 추가
	// 탐색 도중 중복 파일이 존재할 경우 handle_conflict() 실행
	int handle=0;
	// handle > 0: 중복파일 발생
	// handle == 0: 중복 없음
	FileNode *temp_file=temp_ext->files;
	while (temp_file->next != NULL){
		if (strcmp(temp_file->filename, "-")!=0 && strcmp(temp_file->filename, new_file->filename)==0){
			handle=handle_dup(temp_file, new_file);
			break;
		}
		temp_file=temp_file->next;
	}
	if (strcmp(temp_file->filename, "-")!=0 && strcmp(temp_file->filename, new_file->filename)==0){
		handle=handle_dup(temp_file, new_file);
	}
	if (handle == 0){ // 중복 파일이 없으면 리스트 끝에 노드 추가
		temp_file->next=new_file;
		return 0;
	}
	free(new_file);
	return 0;
}
int handle_dup(FileNode *old, FileNode *new){
	printf("\n1. %s\n2. %s\n", old->path, new->path);

	char input[512];
	int arg1=0, arg2=0;

	while (1){
		printf("\n\nchoose an option:\n");
		printf("0. select [num]\n1. diff [num1] [num2]\n2. vi [num]\n3. do not select\n\n");
		printf("20232372> ");
		fflush(stdout);

		fgets(input, sizeof(input), stdin);
		input[strcspn(input, "\n")]='\0';


		if (sscanf(input, "select %d", &arg1) == 1){
			if (arg1 == 1){ // old 선택
				return 1;
			}
			else if (arg1 == 2){ // new 선택
				strcpy(old->path, new->path);
				return 1;
			}
			else continue;
		}
		else if (strcmp(input, "do not select") == 0){
			strcpy(old->filename, "-");
			strcpy(old->path, "-");
			return 1;
		}
		else if (sscanf(input, "diff %d %d", &arg1, &arg2) == 2 ){
			char path_1[PATH_MAX+1], path_2[PATH_MAX+1];
			if (arg1 == 1 && arg2 == 2){
				strcpy(path_1, old->path);
				strcpy(path_2, new->path);
			} else if (arg1 == 2 && arg2 == 1){
				strcpy(path_1, new->path);
				strcpy(path_2, old->path);
			} else continue;

			pid_t pid = fork();
			if (pid==0){
				char *args[]={"diff", path_1, path_2, NULL};
				if (execvp("diff", args)<0){
					fprintf(stderr, "diff execvp error\n");
					continue;
				} 
				else {
					waitpid(pid, NULL, 0);
				}
			}
			continue;
		}
		else if (sscanf(input, "vi %d", &arg1) == 1){
			char selected_path[PATH_MAX+1];
			if (arg1 == 1){
				strcpy(selected_path, old->path);
			} else if (arg1 == 2){
				strcpy(selected_path, new->path);
			} else continue;
			// 자식 프로세스 생성
			pid_t pid=fork();
			if (pid == 0){
				// vi 실행
				char *args[]={"vi", selected_path, NULL};
				if (execvp("vi", args)<0){
					fprintf(stderr, "vi execvp error\n");
					continue;
				}
				else {
					waitpid(pid, NULL, 0);
				}
			}
			continue;
		}
		else continue;
	}
}
int find_extension(const char *extension){
	// list에 해당 extension node가 존재하는지 확인
	// extension node가 존재하면 1, 존재하지 않으면 0 return
	// 에러 시 -1 return
	if (extension == NULL){
		return -1;
	}
	if (head==NULL){
		return 0;
	}
	ExtNode *temp = head;
	do {
		if (strcmp(temp->extension, extension)==0){
			return 1;
		}
		temp=temp->next;
	} while (temp != NULL);
	
	// 리스트의 끝에 도달하면 return 0
	// 해당 extension node가 존재하지 않으므로
	return 0;

}
int make_dir(const char *cur_path){
	// linked list 구조에 따라 디렉토리 생성 
	// 성공 시 0, 실패 시 -1 return
	char output_path[PATH_MAX+1]; // MAX 4096
	int error_code;
	// option_d: <output_path>
	if (opt.option_d == 1){
		if (strcmp(opt.output_path, "")!=0 && strlen(opt.output_path)<=PATH_MAX){
			strcpy(output_path, opt.output_path);
		}
	} else{ // default: <DIR_PATH>_arranged
		if(snprintf(output_path, (size_t)(PATH_MAX+1), "%s_arranged", cur_path)>(PATH_MAX+1)){
			fprintf(stderr, "Error: path too long\n");
			exit(1);
		}
	}

	// extension node: make directory
	if (head==NULL){
		fprintf(stderr, "make_dir: head is NULL\n");
		return -1;
	}
	// output_path: 디렉토리 생성
	if (mkdir(output_path, 0755)<0){
		if ((error_code=errno) == EEXIST){
			// 디렉토리가 이미 존재하면 에러 처리 후 프롬프트 재출력
			fprintf(stderr, "%s already exists\n", output_path);
			exit(1);
		} else if (error_code == ENAMETOOLONG){
			fprintf(stderr, "Error: File name too long\n");
			exit(1);
		} else if (error_code == ENOTDIR){
			fprintf(stderr, "Error: Invalid path\n");
			exit(1);
		} else {
			fprintf(stderr, "mkdir error\n");
			exit(1);
		}
	}
	

	ExtNode *temp_ext=head;
	// extension node 탐색
	while (temp_ext != NULL){
		// extension 디렉토리 경로 설정
		char dir_path[PATH_MAX+1];
		if (snprintf(dir_path, (size_t)(PATH_MAX+1), "%s/%s", output_path, temp_ext->extension)>PATH_MAX){
			fprintf(stderr, "Error: path too long\n");
			exit(1);
		}
		// 디렉토리 생성
		if (mkdir(dir_path, 0755)<0){
			if ((error_code=errno) == EEXIST){
				// extension 디렉토리가 이미 존재할 경우
				// 디렉토리 생성하지 않고 파일 추가
			} else {
				fprintf(stderr, "mkdir error\n");
				exit(1);
			}
		}

		// extension node에 파일이 링크되어 있지 않을 경우
		if (temp_ext->files == NULL){
			// 파일을 추가하지 않고 continue
			temp_ext=temp_ext->next;
			continue;
		}

		// file node가 링크되어 있을 경우
		// file node 탐색하며 file 복사
		FileNode *temp_file = temp_ext->files;


		while (temp_file != NULL){
			if (strcmp(temp_file->filename,"-")==0){
				temp_file=temp_file->next;
				continue;
			}
			if (copy_file(temp_file->path, temp_file->filename, dir_path)<0){
				fprintf(stderr, "copy file error\n");
				return -1;
			}

			// 다음 file node로 이동
			temp_file=temp_file->next;
		}
		// 다음 extension node로 이동
		temp_ext=temp_ext->next;
	}
	return 0;
}
int copy_file(const char *file_path, const char *filename, const char *dir_path){
	// 파일 복사: open -> write
	// 복사 성공 시 0, 실패 시 -1 리턴
	
	int original_fd, copy_fd;
	
	char new_path[PATH_MAX];
	if (snprintf(new_path, (size_t)(PATH_MAX+1), "%s/%s", dir_path, filename)>PATH_MAX){
		fprintf(stderr, "Error: path too long\n");
		exit(1);
	}

	// fd
	// 원본 파일 open
	if ((original_fd=open(file_path, O_RDONLY))<0){
		fprintf(stderr, "open error for %s\n", filename);
		return -1;
	}
	// 복사할 파일 create
	if ((copy_fd=open(new_path, O_WRONLY | O_CREAT | O_TRUNC, 00666))<0){
		fprintf(stderr, "create error for %s\n", filename);
		return -1;
		
    // O_CREAT: 복사본 파일이 존재하지 않을 경우, 새로운 파일 생성
    // O_TRUNC: 복사본 파일이 이미 존재할 경우, 이전 데이터 전부 삭제
	}

	// EOF에 도달할 때까지 BUFFER_SIZE 만큼 read & write
    while (1){
        char buffer[BUFFER_SIZE];
        int read_bytes, write_bytes;
        // 1. read
        if ((read_bytes=read(original_fd, buffer, BUFFER_SIZE))<0){
            fprintf(stderr, "read error for %s\n", filename);
            exit(1);
        }
        // 2. write
        // 읽은 바이트만큼만 저장
        if ((write_bytes=write(copy_fd, buffer, read_bytes))<0){
            fprintf(stderr, "write error for %s\n", filename);
            exit(1);
        }
        // 복사한 데이터가 BUFFER_SIZE 미만인 경우(EOF 도달) 종료
        if (read_bytes < BUFFER_SIZE) break;
    }

    close(original_fd);
    close(copy_fd);

}
