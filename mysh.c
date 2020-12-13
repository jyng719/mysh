#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define READ 0
#define WRITE 1
#define MAX 1024

int fd, pid, child, status, amper, fg = 0;
int backgroundIndex, leftIndex, rightIndex, pipeIndex; // &, <, >, | index 변수들
int fdPipe[2];
char str[MAX]; // 사용자 입력 저장
char* command1[MAX];
char* command2[MAX];
char* bgCmdList[MAX]; // 백그라운드 프로세스 이름 리스트
int bgPidList[MAX] = {0}; // 백그라운드 프로세스 ID 리스트
int bgCount = 0; // 백그라운드 프로세스 개수
int argc; // 명령 인수 저장
char* argv[MAX]; // 명령 인자 저장

void init() // 초기화
{
	amper = 0;
	backgroundIndex = 0;
	leftIndex = 0;
	rightIndex = 0;
	pipeIndex = 0;
	memset(argv, '\0', MAX);
	memset(str, '\0', MAX);
	memset(command1, '\0', MAX);
	memset(command2, '\0', MAX);
}

void commandInput() // 사용자 입력 받기
{
	printf("[1692147 jjy]mysh : ");
	fgets(str, sizeof(str), stdin);
	str[strlen(str) - 1] = '\0';
}

int tokenize(char* str, char* tokens[]) // 사용자 입력을 명령인수와 명령인자로 나누기
{
	char* token; // 명령 인자
	int tokenCount = 0; // 명령 인수

	if(str == NULL) return tokenCount; // 사용자가 enter만 쳤으면 0을 리턴
	token = strtok(str, " \n\t"); // 공백, enter, tab을 기준으로 나눈다.
	while(token != NULL)
	{
		tokens[tokenCount++] = token;
		token = strtok(NULL, " \n\t");
	}
	tokens[tokenCount] = NULL;
	return tokenCount; // 명령 인수 리턴
}

void cheakIndex(int argc, char* argv[]) // &, <, >, | 체크해서 index 저장
{
	for(int i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "&") == 0)
		{
			amper = 1; // amper 변경
			backgroundIndex = i;
		}
		else if(strcmp(argv[i], "<") == 0)
		{
			leftIndex = i;
		}
		else if(strcmp(argv[i], ">") == 0)
		{
			rightIndex = i;
		}
		else if(strcmp(argv[i], "|") == 0)
		{
			pipeIndex = i;
		}
	}
}

void myjobs() // 현재 백그라운드 명령어들과 그들의 pid 출력
{
	printf("Background Count : [%d]\n", bgCount);
	for(int i = 0; i < bgCount; i++)
		printf("[현재 Background] : %s\t\t [Process ID] : %d\n", bgCmdList[i], bgPidList[i]);
}

void chldsignal() // 시그널 핸들러
{
	int pid, status;
	pid = waitpid(-1, &status, 0); // 임의의 자식 프로세스를 기다림
	for(int i = 0; i < bgCount; i++) // 백그라운드 프로세스 리스트에 존재하는지 체크
	{
		if (pid == bgPidList[i]) // 백그라운드 프로세스 종료 시
		{
			// 백그라운드 프로세스 리스트에서 삭제
			free(bgCmdList[i]); // 종료된 프로세스 이름 공간 할당 해제
			bgCmdList[i] = bgCmdList[bgCount - 1]; // 마지막에 있는 이름 가져오기
			bgCmdList[bgCount - 1] = NULL; // 마지막에 있는 이름 삭제
			bgPidList[i] = bgPidList[bgCount - 1]; // 마지막에 있는 pid값 가져오기
			bgPidList[bgCount - 1] = 0; // 마지막에 있는 pid 삭제
			bgCount--;
			return;
		}
	}
	fg = 0;	// 포그라운드 프로세스 종료 시 fg를 0으로
}

int main()
{
	signal(SIGCHLD, chldsignal); // 시그널 핸들러 등록
	while(1)
	{
		// 초기화
		init();

		// 사용자 입력 받기
		commandInput();

		// 명령어와 인자 나누기
		argc = tokenize(str, argv);
		if(argc == 0) continue; // 명령 개수가 0이면 처음으로

		// 종료 체크(exit, logout)
		if(strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "logout") == 0)
			exit(0);

		// myjobs
		if(strcmp(argv[0], "myjobs") == 0)
		{
			myjobs();
			continue;
		}

		// &, <, >, | 체크
		cheakIndex(argc, argv);

		// command에서 & 지우기
		if(amper == 1)
		{
			int count = 0; // 지워질 argc 개수
			for(int i = backgroundIndex; i < argc; i++) // & 앞까지만 저장
			{
				argv[i] = NULL;
				count++;
			}
			argc = argc - count;
		}

		// 자식 생성
		pid = fork();
		if(pid == 0) // 자식일 경우
		{
			// 리다이렉트 체크
			if(leftIndex > 0) // <
			{
				fd = open(argv[leftIndex + 1], O_WRONLY);
				dup2(fd, STDIN_FILENO); // fd가 기본 입력
				close(fd);
				for(int i = 0; i < argc; i++) // command에서 < 지우기
				{
					if(i == leftIndex)
					{
						for(int j = leftIndex; j < argc - 1; j++)
							command1[j] = argv[j + 1];
						break;
					}
					command1[i] = argv[i];
				}
			}
			else if(rightIndex > 0) // >
			{
				fd = open(argv[rightIndex + 1], O_CREAT|O_TRUNC|O_WRONLY, 0600);
				dup2(fd, STDOUT_FILENO); // fd가 기본 출력
				close(fd);
				for(int i = 0; i < rightIndex; i++) // command에서 > 지우기
					command1[i] = argv[i];
			}
			// 파이프 체크
			if(pipeIndex > 0)
			{
				// command 나누기
				for(int i = 0; i < pipeIndex; i++)
					command1[i] = argv[i];
				for(int i = pipeIndex; i < argc - 1; i++)
					command2[i - pipeIndex] = argv[i + 1];
				pipe(fdPipe); // 파이프 연결
				if (fork() == 0) // 손자일 경우
				{
					close(fdPipe[READ]);
					dup2(fdPipe[WRITE], STDOUT_FILENO); // 쓰기용 파이프를 표준출력에 복제
					close(fdPipe[WRITE]);
					execvp(command1[0], command1); // 첫번째 명령 실행
					perror("pipe");
				}
				else // 자식일 경우
				{
					close(fdPipe[WRITE]);
					dup2(fdPipe[READ], STDIN_FILENO); // 읽기용 파이프를 표준입력에 복제
					close(fdPipe[READ]);
					execvp(command2[0], command2); // 두번째 명령 실행
					perror("pipe");
				}
			}
			// 모든 체크가 끝난 후 명령 실행
			if(leftIndex > 0 || rightIndex > 0) // 리다이렉션이면 command1 실행
				execvp(command1[0], command1);
			else // 나머지 경우는 argv[0] 실행
				execvp(argv[0], &argv[0]);
			fprintf(stderr, "%s:실행 불가\n", argv[0]);
			exit(1);
		}
		else // 부모일 경우
		{
			// amper 체크
			if(amper == 0) // 포그라운드 명령일 경우
			{
				fg = 1; // 포그라운드 마킹
				while(fg == 1) // 포그라운드 명령이 끝나기 전까지
					pause(); // 멈춤
			}
			else // 백그라운드 명령일 경우
			{
				bgCmdList[bgCount] = malloc(sizeof(char) * MAX); // free 잊지말것
				strcpy(bgCmdList[bgCount], argv[0]); // 프로그램 이름 저장
				bgPidList[bgCount] = pid; // 프로세스 ID 저장
				bgCount++;
			}

		}
	}
}

