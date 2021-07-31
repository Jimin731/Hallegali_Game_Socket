#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define NORMAL_SIZE 20

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* message);

//char name[NORMAL_SIZE] = "[DEFALT]";     // name
char msg_form[NORMAL_SIZE];            // msg form
char msg[BUF_SIZE];                    // msg

int main(int argc, char* argv[])
{
	system("clear");
	printf("+---------------------------+\n");
	printf("+ Halli Galli Game          +\n");
	printf("+ Berkely Socket            +\n");
	printf("+ Client                    +\n");
	printf("+---------------------------+\n");

	int sock;
	pthread_t snd_thread, rcv_thread;
	void* thread_return;
	struct sockaddr_in serv_adr;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}

void* send_msg(void* arg)
{
	int sock = *((int*)arg);
	char name_msg[NORMAL_SIZE + BUF_SIZE];

	// 시작 메시지 보냄
	printf(" >> join the game !! \n");

	while (1)
	{
		fgets(msg, BUF_SIZE, stdin);

		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) // q Q 입력시 서버와 연결 종료
		{
			close(sock);
			exit(0);
		}

		// 서버에 메시지 전송
		sprintf(name_msg, "%s", msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}

void* recv_msg(void* arg)
{
	int sock = *((int*)arg);
	char name_msg[NORMAL_SIZE + BUF_SIZE];
	int str_len;

	while (1)
	{
		str_len = read(sock, name_msg, NORMAL_SIZE + BUF_SIZE - 1);

		if (str_len == -1) // 서버에게 받은 메시지와 다를 경우 -1 리턴
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout); // 받은 메시지 출력
	}
	return NULL;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
