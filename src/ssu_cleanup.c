#include "cleanup.h"

int main(void){
	char input[INPUT_SIZE];
	int input_argc;
	char *input_argv[ARGS_MAX];

	while(1){
		// print prompt
		printf("20232372> ");
		fflush(stdout);
		
		// 사용자 입력
		if (fgets(input, sizeof(input), stdin) == NULL){
			fprintf(stderr, "ssu_cleanup: input error\n");
			exit(1);
		} else if (input[0] == '\n') 
			continue; // 프롬프트 상에서 엔터만 입력 시 프롬프트 재출력

		// input 문자열 parsing
		parse_input(input, &input_argc, input_argv);

		// exit: 프로그램 종료
			if (input_argv[0]!=NULL && strcmp(input_argv[0], "exit")==0){
				exit(0);
			}
		// help: usage 출력
			else if (input_argv[0]!=NULL && strcmp(input_argv[0], "help")==0){
		// help [COMMAND]
				if (input_argv[1]!=NULL && strcmp(input_argv[1], "tree")==0)
					print_usage(1);
				else if (input_argv[1]!=NULL && strcmp(input_argv[1], "arrange")==0)
					print_usage(2);
				else
					print_usage(0);
				continue;
			}
		
		// 명령어 & 필수 인자 유효성 검사
		int command_valid;
		if ((command_valid=is_command_valid(input_argc, input_argv))<0){
			continue;
		} else if (command_valid==0){
			print_usage(0);
			continue;
		}

		// 옵션 유효성 검사
		int option_valid;
		char *command_args[ARGS_MAX];
		if ((option_valid=set_option(input_argc, input_argv, command_args))<0){
			print_usage(0);
			continue;
		}

		// tree
		if ( option_valid == 1){
			// [0]: tree, [1]: <DIR_PATH> [2]: option-sp 00~11
			pid_t pid;
			// fork(): 자식 프로세스 생성
			if ((pid = fork()) < 0){
				fprintf(stderr, "ssu_cleanup: fork error\n");
				exit(1);
			}
			if (pid == 0){ // execvp(): 자식 프로세스에서 실행 (이미지 대체)
				if (execvp("./tree", command_args) < 0){
					fprintf(stderr, "ssu_cleanup: tree execvp error\n");
					exit(1);
				}
			} else {
				wait(NULL);
			}
		}
		// arrange
		else if ( option_valid == 2){
			// [0]: arrange [1]: option-dtxe 0000~1111 [2]: <DIRPATH>
			// [3]: -d output_path [4]: -t seconds 
			// [5] -x "exclude_path1 exclude_path2 ..." [6]: -e "extension1 extension2 ..."
			pid_t pid;
			// fork(): 자식 프로세스 생성
			if ((pid=fork())<0){
				fprintf(stderr, "ssu_cleanup: fork error\n");
				exit(1);
			}
			if (pid == 0){
				if (execvp("./arrange", command_args)<0){
					fprintf(stderr, "ssu_cleanup: arrange execvp error\n");
					exit(1);
				}
			} else {
				wait(NULL);
			}
		}
		// 기타 명령어 입력 -> usage 출력 후 프롬프트 재출력
		else {
			print_usage(0);
			continue;
		}

	}
	return 0;

}
void parse_input(char *input, int *argc_out, char *argv_out[]){
	int argc = 0;
	char *p = input;

	while (*p) {
		while (isspace(*p)) p++; // 공백 무시
		if (*p == '\0') break; // NUL에 도달하면 종료
		
		// parsing
		char *start;
		if (*p == '"'){
			p++; // 큰따옴표 처리
			start = p;
			while (*p && *p !='"') p++; // 다음 큰 따옴표가 나오기 전까지(공백 포함)
		} else {
			// 따옴표로 묶인 인자가 아닐 경우 공백을 기준으로 분리
			start = p;
			while (*p && !isspace(*p)) p++;
		}

		int len = p-start;
		char *arg=(char *)malloc(len+1);
		strncpy(arg, start, len);
		arg[len] = '\0';

		argv_out[argc++] = arg;
		if (*p == '"') p++;
	}
	argv_out[argc]=NULL;
	*argc_out=argc;

	return;
}
void print_usage(int num){
	// 0: help
	// 1: help tree
	// 2: help arrange

	const char *usage[]={
		"Usage:\n",
		"\t> tree <DIR_PATH> [OPTION]...\n",
		"\t\t<none> : Display the directory structure recursively if <DIR_PATH> is a directory\n",
		"\t\t-s : Display the directory structure recursively if <DIR_PATH> is a directory, including the size of each file\n",
		"\t\t-p : Display the directory structure recursively if <DIR_PATH> is a directory, including the permissions of each directory and file\n",
		"\t> arrange <DIR_PATH> [OPTION]...\n",
		"\t\t<none> : Arrange the directory if <DIR_PATH> is a directory\n",
		"\t\t-d <output_path> : Specify the output directory <output_path> where <DIR_PATH> will be arranged if <DIR_PATH> is a directory\n",
		"\t\t-t <seconds> : Only arrange files that were modified more than <seconds> seconds ago\n",
		"\t\t-x <exclude_path1, exclude_path2, ...> : Arrange the directory if <DIR_PATH> is a directory except for the files inside <exclude_path> directory\n",
		"\t\t-e <extension1, extension2, ...> : Arrange the directory with the specified extension <extension1, extension2, ...>\n",
		"\t> help [COMMAND]\n",
		"\t> exit\n"
	};
	switch (num){
		case 1: // tree
			for (int i=0; i<5; i++) printf("%s", usage[i]);
			break;
		case 0: // help
			for (int i=0; i<13; i++) printf("%s", usage[i]);
			break;
		case 2: 
			printf("%s", usage[0]);
			for (int i=5; i<11; i++) printf("%s", usage[i]);
			break;
	}
	return;
}
int is_command_valid(int argc, char *argv[]){
		// RETURN VALUE
		//	1: valid 
		// 	0: usage 출력 
		// -1: error 메시지 출력 후 프롬프트 재출력
		
		if (argc < 2) return 0;
		
		char original_path[PATH_MAX+1];
		char resolved_path[PATH_MAX+1];
		

		// 1. 유효한 명령인지 검사
		if (argv[0] == NULL)
			return 0;
		else if (strcmp(argv[0],"tree")!=0 && strcmp(argv[0],"arrange")!=0)
			return 0;
		// 2. 경로 검사
		// 경로가 NULL인 경우 usage 출력
		if (argv[1]==NULL)
			return 0;
		else if (argv[1][0] == '-')
			return 0;
		else if (strlen(argv[1])>PATH_MAX){
			fprintf(stderr, "Error: Path too long\n");
			return -1;
		}
		strcpy(original_path, argv[1]);
		return to_realpath(original_path, resolved_path);
}
int set_option(int argc, char *argv[], char *output_args[]){
	// RETURN
	// -1: 실패
	//  1: set tree option
	//  2: set arrange option

	if (argv[0]==NULL)
		return -1;
	if (strcmp(argv[0], "tree")==0){
		// [0]: tree, [1]: <DIR_PATH> [2]: option-sp 00~11
		int opt_s=0;
		int opt_p=0;
		int opt;
		optind=2;
		while ((opt=getopt(argc, argv, "sp")) != -1){
			switch(opt){
				case 's':
					opt_s=1;
					break;
				case 'p':
					opt_p=1;
					break;
				case '?':
					return -1;
			}
		}
		// set tree option
		output_args[0]=strdup(argv[0]);
		// DIR_PATH
		output_args[1]=strdup(argv[1]);
		// option code
		output_args[2]=malloc((size_t)3*sizeof(char));
		output_args[2][0]=(opt_s)? '1':'0';
		output_args[2][1]=(opt_p)? '1':'0';
		output_args[2][2]='\0';
		// NULL
		output_args[3]=NULL;
		
		return 1;
	} 
	else if (strcmp(argv[0], "arrange")==0){
		// [0]: arrange [1]: option-dtxe 0000~1111 [2]: <DIRPATH>
		// [3]: -d output_path [4]: -t seconds 
		// [5] -x "exclude_path1 exclude_path2 ..." [6]: -e "extension1 extension2 ..."
		char *d_arg=NULL;
		char *t_arg=NULL;
		char *x_arg=NULL;
		char *e_arg=NULL;

		int opt_d=0;
		int opt_t=0;
		int opt_x=0;
		int opt_e=0;

		// set arrange option
		int opt;
		optind=2;
		while ((opt=getopt(argc, argv, "e:x:t:d:")) != -1){
			switch(opt){
				case 'd':
					// 옵션이 두 번 선택된 경우 에러
					if (opt_d==1) 
						return -1;
					// option flag
					opt_d=1;
					// 옵션에 두 개 이상의 인자가 존재하면 에러
					if (optind < argc && argv[optind][0]!='-')
						return -1;
					else
						d_arg=strdup(optarg);
					break;
				case 't':
					// 옵션이 두 번 선택된 경우 에러
					if (opt_t==1) 
						return -1;
					// option flag
					opt_t=1;
					// 옵션에 두 개 이상의 인자가 존재하면 에러
					if (optind < argc && argv[optind][0]!='-')
						return -1;
					else
						t_arg=strdup(optarg);
					break;
				case 'x':
					// 옵션이 두 번 선택된 경우 에러
					if (opt_x==1) 
						return -1;
					// option flag
					opt_x=1;
					// 옵션에 두 개 이상의 인자가 존재하는 경우 별도 처리
					if (optind < argc && argv[optind][0]!='-'){
						size_t len = strlen(optarg)+strlen(argv[optind])+2;
						x_arg=malloc(len);
						snprintf(x_arg, len, "%s %s", optarg, argv[optind]);
						optind++;
					} else // 인자가 하나일 경우 별도 처리 없이 복제
						x_arg=strdup(optarg);
					break;
				case 'e':
					// 옵션이 두 번 선택된 경우 에러
					if (opt_e==1) 
						return -1;
					// option flag
					opt_e=1;
					// 옵션에 두 개 이상의 인자가 존재하는 경우 별도 처리
					if (optind < argc && argv[optind][0]!='-'){
						size_t len = strlen(optarg)+strlen(argv[optind])+2;
						e_arg=malloc(len);
						snprintf(e_arg, len, "%s %s", optarg, argv[optind]);
						optind++;
					} else // 인자가 하나일 경우 별도 처리 없이 복제
						e_arg=strdup(optarg);
					break;
				case '?':
					return -1;
			}
		}
		// set output_args
		output_args[0]=strdup(argv[0]); 
		// option code
		output_args[1]=malloc((size_t)5*sizeof(char));
		output_args[1][0]= (opt_d) ? '1' : '0'; 
		output_args[1][1]= (opt_t) ? '1' : '0'; 
		output_args[1][2]= (opt_x) ? '1' : '0'; 
		output_args[1][3]= (opt_e) ? '1' : '0'; 
		output_args[1][4]='\0';
		// DIR_PATH
		output_args[2]=strdup(argv[1]); 
		// arguments
		output_args[3]=(opt_d) ? strdup(d_arg) : strdup("-");
		output_args[4]=(opt_t) ? strdup(t_arg) : strdup("-");
		output_args[5]=(opt_x) ? strdup(x_arg) : strdup("-");
		output_args[6]=(opt_e) ? strdup(e_arg) : strdup("-");
		// NULL
		output_args[7]=NULL;
		return 2;
	} 
	else return -1;
	
}
int to_realpath(const char *original_path, char *resolved_path){
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
