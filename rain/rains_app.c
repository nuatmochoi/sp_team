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

	int rain_fd = open("/dev/rains_dev",O_RDONLY); //��ϵ� ����̽�(�������� ����)�� �б� ���� open �Լ��� ���
	if(rain_fd<0){
		perror("file open failed");
		exit(-1);
	}
	int rain_detect;

	while(1){
		
		read(rain_fd, &rain_detect, sizeof(int)); // Ŀ�ηκ��� ����� ���� ���� �� 0�Ǵ� 1�� ������
		if(rain_detect == 0){ // ����� ���� ������ ��� ���� ������ �Ǿ��� ���� 0�� ����Ѵ�.
			printf("Rainning\n");
			write(sock,"3\n",2); // 0�� ���� ���� ���� ���� socket ������ ������.

		}
		else{
			printf("Not Rainning\n"); // 1�� ���� ���� �������� ������ socket ������ ������.
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
