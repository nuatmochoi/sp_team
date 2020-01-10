#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define BUF_SIZE 100
void *send_msg(void * arg);
void error_handling(char *msg);

char msg[BUF_SIZE];

int main(int argc, char ** argv) {

	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread;
	void * thread_return;

	if(argc!=3){
		printf("Usage: %s <IP> <port>\n",argv[0]);
		exit(1);
	}

	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1){
		error_handling("socket() error");
	}
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}
	pthread_create(&snd_thread,NULL,send_msg,(void*)&sock);
	pthread_join(snd_thread,&thread_return);
	close(sock);
	return 0;

}

void *send_msg(void *arg){
	int sock = *((int*)arg);

	int rain_fd = open("/dev/rains_dev",O_RDONLY); //등록된 디바이스(빗물감지 센서)를 읽기 위해 open 함수를 사용
	if(rain_fd<0){
		perror("file open failed");
		exit(-1);
	}
	int rain_detect;

	while(1){
		
		read(rain_fd, &rain_detect, sizeof(int)); // 커널로부터 빗방울 감지 센서 값 0또는 1을 가져옴
		if(rain_detect == 0){ // 빗방울 감지 센서의 경우 물이 감지가 되었을 때가 0을 출력한다.
			printf("Rainning\n");
			write(sock,"3\n",2); // 0일 경우는 물이 감지 됨을 socket 서버에 보낸다.

		}
		else{
			printf("Not Rainning\n"); // 1일 경우는 물이 감지되지 않음을 socket 서버에 보낸다.
			write(sock,"4\n",2);

		}
		fflush(stdout);
		sleep(15);
	}
	close(rain_fd);
	return NULL;
}

void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
