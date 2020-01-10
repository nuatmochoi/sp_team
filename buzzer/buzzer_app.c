/* **************************************
	�ۼ���: �ּ���(201420913)
	�������: ��â��(201420871)
	�ۼ����� : 2019.05.31
 *************************************** */
#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void *recv_msg(void * arg);
void error_handling(char * msg);

char msg[BUF_SIZE];

int main(int argc, char *argv[]){

	int sock;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;
	void  *thread_return;

	if(argc!=3){		//Ŭ���̾�Ʈ�ܿ����� ������ IP�� port��ȣ 
		printf("Usage: %s <IP> <port>\n",argv[0]);
		exit(1);
	}

	sock = socket(PF_INET,SOCK_STREAM,0);	//IPv4�� TCP�������� ���·� ���� ����
	if(sock==-1){
		error_handling("socket() error");
	}
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	//�������� ���� ����
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}

	//���Ա�, ������, �ϼ���, â��, ������ ����̽��� ��� �Է¼����κ��� ���� �Է� �޾ƾ� �Ѵ�.(recv_msg �Լ� ����)
	pthread_create(&rcv_thread,NULL,recv_msg,(void*)&sock);
	pthread_join(rcv_thread,&thread_return);	//������ ������
	close(sock);
	return 0;
}

void * recv_msg(void * arg){
	int flag=0;
	int cnt=0;
	int sock = *((int*)arg);
	int str_len;
	char name_msg[BUF_SIZE];
	int fd;
	char input = '1';
	fd = open("/dev/buzzer_dev",O_RDWR);
	if(fd<0){
		perror("Failed\n");
		return NULL;
	
	}

	/*****************************************************
	sensor_arr �迭 �� ���� -1�� �ʱ�ȭ �����̴�.
	sensor_arr[1] = �µ� 30�� �̻� ����, 
	sensor_arr[2] = �µ� 30�� ���� ����, 
	sensor_arr[3] = ������������ on ����, 
	sensor_arr[4] = ������������ off ����,
	sensor_arr[5] = ���� ����(�������� full),
	sensor_arr[6] = ���� �����̻�(�������� half �̻�),
	sensor_arr[7] = ���� ��������(�������� half ����),
	sensor_arr[8] = ���� ���Ǿ���
	******************************************************/
	while(1){
		int sensor_arr[9];
		memset(sensor_arr,-1,sizeof(sensor_arr));
		int i=0;
		char cur_num;

		//�Է¼����κ��� 3���� ���� �޾ƾ��Ѵ�.
		while(1){
			str_len = read(sock,name_msg,BUF_SIZE-1);
			if(str_len>0 && i<3){	//�Է¼����κ��� 3���� ���� ���� ������ ��
				name_msg[str_len]=0;

				if(sensor_arr[name_msg[0]-'0']==-1){		//�Է¼����κ��� �ߺ��� ���� �ƴ� ���� �޾��� ��
					printf("num : %d", i++);			
					sensor_arr[name_msg[0]-'0']=1;
					fputs(name_msg,stdout);

					if(name_msg[2]-'0'>=0 && name_msg[2]-'0'<=9){	//�Է¼����� delay�� ���� 2���� ���� ���޾� �������
						printf("delay num : %d", i++);
						sensor_arr[name_msg[2]-'0']=1;
					}
				}
			}
			else{		//�Է¼����κ��� 3���� ���� ��� ��������
				i=0;
				break;
			}
		}

		//�Է¹��� ������ ���
		for(int t=0;t<9;t++){
			printf("%d ",sensor_arr[t]);
		}
		puts("");

		//sensor_arr�� idx 1��,4��,5��,6�� => �µ��� 30�� �̻�, ���� ���� ����, ����ũ ���� �����ְų� �����̻�� ���� �︲
		if(sensor_arr[1]==1 && sensor_arr[4]==1 && (sensor_arr[5]==1 || sensor_arr[6]==1)){
			if(flag==0){	//�ٷ������� ������ �︮�� ���� ���¶�� 
			int len = write(fd, &input,1);	//����̽��ܿ� �� 1 ����
			flag=1;
			}
			else{
				cnt++;		//�ٷ� ������ ������ �︰ ������ ���
				if(cnt==1)	//���������ʴ� ����, �� cnt���� 1�� �Ǿ����� flag�� �ٽ� �ʱ�ȭ
					flag=0;
			}
		}

	}

	return NULL;
}

//���� �Լ� ó��
void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
