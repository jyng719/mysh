#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 128

int pid, status, child;
int amper;
char cmd[MAX];
int argc;
char* argv[MAX];
int tokenCount;
char* tokens[MAX];
int additionalFlag = 0;

void Tokenize(char* cmd, char* argv[])
{
	char* token;
	int tokenCount = 0;
	token = strtok(cmd, " ");

}

void init()
{
	argc = 0;
	amper = 0;
	memset(cmd, 0, MAX);
}

int main()
{
	/*int pid, status, child;
	int amper;
	char cmd[MAX]; // 사용자 입력 버퍼
	int argc; // 인자 개수
	char* argv[MAX]; // 인자들
	int tokenCount;
	char * tokens[Max];
	int additionalFlag = 0;*/
	
	while(1)
	{
		// 초기화
		/*argc = 0;
		amper = 0;
		memset(buf, 0, MAX);*/
		init();
		
		// 사용자 입력 받기
		write(1, "[1692147]mysh~>> : ", 20); // 입력 메세지 출력
		char c;
		/*while(read(0, &c, 1) > 0) // 한글자씩 체크
		{
			if(c == '\n') break; // 엔터키가 있으면 브레이크
			strcat(buf, &c); // 버퍼에 저장
		}*/
		fgets(cmd, sizeof(cmd) - 1, stdin);
		//write(1, buf, MAX); // debug
		
		// 명령어와 인자 나누기
		argv[argc] = malloc(sizeof(char) * 3); // 인자가 들어갈 공간 할당
		char* cp = cmd; // 버퍼에서 한글자를 가리킬 포인터
		for(int i = 0; i < MAX; i++) // 버퍼에서 한글자씩 체크
		{
			if(*cp == '\0') break; // 버퍼의 끝에 도달하면 브레이크
			//if(buf[i] == '&') amper = 1; // &가 있으면 amper를 1로 만든다.
			/*if(*cp == ' ') // 스페이스가 있으면
			{
				//argc++; // 인자 개수를 하나 증가하고
				//argv[argc] = malloc(sizeof(char) * MAX); // 인자가 들어갈 공간 할당
			}*/
			strncat(argv[argc], cp, 1); // 인자에 한글자씩 넣기
			cp++; // 다음 글자 가리키기
		}
		strcat(argv[argc], "\0");
		//write(1, argv[0], MAX); // debug
		
		// 명령어와 인자 나누기
		// 마지막 인자 NULL 만들기
		argc++;
		argv[argc] = NULL;
		
		// debug
		for(int i = 0; i < argc; i++)
		{
			write(1, "debug", 6);
		}
		write(1, cmd, MAX);
		write(1, argv[0], MAX);
		
		// 백그라운드 명령어(&) 체크
		/*for(int i = 0; i < MAX; i++) // 버퍼에서 한글자씩 체크
		{
			if(buf[i] == '\0') break; // 버퍼의 끝에 도달하면 브레이크
			if(buf[i] == '&') amper = 1; // &가 있으면 amper를 1로 만든다.
		}*/
		
		// 체크해야할것 : &, <, >, |
		argv[0] = cmd; // 명령어 저장
		//argv[1] = NULL; // 명령어 인수 저장
		if(strcmp(argv[0], "exit") == 0 || // 명령어가 exit이거나
			strcmp(argv[0], "logout") == 0) // logout이면
		{
			exit(0); // 종료한다.
		}
		
		// 자식 생성하기
		pid = fork();
		if(pid == 0) // 자식일 경우
		{
			// 리다이렉트 체크
			
			// 파이프 체크
			execvp(argv[0], &argv[0]);
			fprintf(stderr, "%s:실행 불가\n", argv[1]);
		}
		else // 부모일 경우
		{
			// amper 체크
			if(amper == 0) child = wait(&status); // 포그라운드 명령이면 자식을 기다린다.
			// 백그라운드 명령이면 처음으로 돌아간다.
			//printf("[%d] 자식 프로세스 %d 종료 \n", getpid(), pid);
			//printf("\t종료 코드 %d \n", status>>8);
			// 할당받은 메모리 반환
			for(int i = 0; i < argc; i++) free(argv[argc]);
		}
	}
}
