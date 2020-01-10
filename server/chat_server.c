/******************************************
201420871 ����Ʈ�����а� ��â��
2019.5.31
******************************************/
#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char *msg,int len);
void error_handling(char * message);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char * argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	pthread_t t_id;

	//������� ���� �Է� ���� ./chat_serv 0000(��Ʈ��ȣ)
	if(argc!=2){
		printf("Usage:%s <port>\n",argv[0]);
		exit(1);
	}

	//pthread_mutex ������ ���� �ּҰ� ����
	pthread_mutex_init(&mutx,NULL);

	//���� IPv4,TCP ȣ�� 
	serv_sock = socket(PF_INET,SOCK_STREAM,0);
	if(serv_sock==-1){
		error_handling("socket() error");
	}

	//���� �ּҰ� �ʱ�ȭ �� �ּ����� & ��Ʈ ����
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	//bind()�� ���� ���� ���� ���
	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1){
		error_handling("bind() error");
	}

	//���� ������ ���� Ŭ���̾�Ʈ�� ���ӿ�û
	if(listen(serv_sock,10)==-1){
		error_handling("listen() error");
	}

	//���� Ŭ���̾�Ʈ�� �������ϱ� ���� �Լ�
	while(1){
		clnt_adr_sz = sizeof(clnt_adr);

		//Ŭ���̾�Ʈ ���� ����
		clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		if(clnt_sock==-1){
			error_handling("accept() error");
		}

		//pthread mutex�� �Ӱ迵���� ����
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;  //Ŭ���̾�Ʈ ���ϵ��� ��Ƶ�
		pthread_mutex_unlock(&mutx);	//Ŭ���̾�Ʈ �Ӱ迵���� ��

		pthread_create(&t_id,NULL,handle_clnt,(void*)&clnt_sock); //������ �Լ�ȣ��
		pthread_detach(t_id);	//������ �Լ� ȣ�� �Ϸ�� �ڵ����� ������ �Ҹ�
		printf("Connected client: IP: %s\n",inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

//������ ������ �Լ� ó��
void * handle_clnt(void*arg){
	int clnt_sock=*((int*)arg);
	int str_len = 0;
	char msg[BUF_SIZE];

	//Ŭ���̾�Ʈ �������κ��� ���� �о����
	while((str_len = read(clnt_sock,msg,sizeof(msg)))!=0){
		msg[str_len]=0;
		printf("msg: %s",msg);
		//Ŭ���̾�Ʈ �ܿ��� �޼����� ������ ��
		send_msg(msg,str_len);
	}

	pthread_mutex_lock(&mutx);		//pthread mutx����ȭ

	//Ŭ���̾�Ʈ ���� ������ ���
	for(int i=0;i<clnt_cnt;i++){
		if(clnt_sock==clnt_socks[i]){
			while(i++<clnt_cnt-1){
				clnt_socks[i]=clnt_socks[i+1];
			}
			break;
		}
	}

	clnt_cnt--;
	pthread_mutex_unlock(&mutx);	//pthread mutx�Ӱ迵�� ��
	close(clnt_sock);
	return NULL;
}

void send_msg(char * msg,int len){

	pthread_mutex_lock(&mutx);		//pthread  mutx����ȭ ����
	for(int i=0;i<clnt_cnt;i++){	//Ŭ���̾�Ʈ �ܿ��� �޼��� �ۼ�
		write(clnt_socks[i],msg,len);
	}
	pthread_mutex_unlock(&mutx);	//pthread mutx����ȭ ����
}

//���� �޼��� ó��
void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
