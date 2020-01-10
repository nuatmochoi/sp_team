/* **************************************
	�ۼ���: �ּ���(201420913)
	�������: ��â��(201420871)
	�ۼ����� : 2019.06.09
 *************************************** */

#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 100

void * recv_msg(void *arg);
void error_handling(char * msg);

int main(int argc, char * argv[]){

	int sock;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;
	void *thread_return;

	if(argc!=3){
		printf("Usage: %s <IP> <port>\n",argv[0]);
		exit(1);
	}

	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock ==-1){
		error_handling("socket() error");
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}

	pthread_create(&rcv_thread,NULL,recv_msg,(void*)&sock);
	pthread_join(rcv_thread,&thread_return);
	close(sock);
	return 0;
}

void *recv_msg(void *arg){
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];
	int str_len;
	char to_dev;
	int w;
	int fd;
	
	fd = open("/dev/motor_dev",O_RDWR);
	if(fd<0){
		perror("opening device FAIL!\n");
		return NULL;
	}

	// �ó������� �´� �������� ���� �����ϱ� ���� int�� �迭 ���� �� -1�� �ʱ�ȭ, -1�� ���� ���� Ȱ��ȭ ���� ���� ����
	// sensor_arr[1] = �µ� 30�� �̻� ����, sensor_arr[2] = �µ� 30�� ���� ����, sensor_arr[3] = ������������ on ����, sensor_arr[4] = ������������ off ����
	// sensor_arr[5] = ���� ����(�������� full), sensor_arr[6] = ���� �����̻�(�������� half �̻�), sensor_arr[7] = ���� ��������(�������� half ����), sensor_arr[8] = ���� ���Ǿ���
	
	while(1){
		int sensor_arr[9];
		memset(sensor_arr,-1,sizeof(sensor_arr));
		int i=0;
		char cur_num;
		while(1){
			str_len = read(sock,name_msg,BUF_SIZE-1);
			if(str_len>0 && i<3){
				name_msg[str_len]=0;
			
				if(sensor_arr[name_msg[0]-'0']==-1){
				
					printf("num : %d", i++);
					sensor_arr[name_msg[0]-'0']=1;
					fputs(name_msg,stdout);
					if(name_msg[2]-'0'>=0 && name_msg[2]-'0'<=9){
						printf("delay num : %d", i++);
						sensor_arr[name_msg[2]-'0']=1; //
					}
				}
			}
			else{
				i=0;
				break;
			}
		}
		for(int t=0;t<9;t++){
			printf("%d ",sensor_arr[t]);
		}
		puts("");

		//�ܺ��� ���� �Ѹ��� ���� â���� �ݱ� ���� ����ó���� �� �κ�
		//�� ���� �ʰ�, �µ��� ������, ����ũ�� ���� ���� ������ â���� �������� ����
		if(sensor_arr[1]==1 && sensor_arr[4]==1 && (sensor_arr[5]==1 || sensor_arr[6]==1||sensor_arr[7]==1)){
			sleep(2); //�ܺ� ������ ���� ���� ����, ������ �︰ ���ķ� ������ �����ϱ� ���� 
			to_dev='0';
			w=write(fd,&to_dev,1);
			printf("Window OPEN\n");

		}
		else if(sensor_arr[8]==1){ // ����ũ�� ���� ���ٸ� �������� ����
			printf("8:%d 7:%d\n",sensor_arr[8],sensor_arr[7]);
			to_dev='1';
			w=write(fd,&to_dev,1);
			printf("Window CLOSE\n");
		}
	
	}
	return NULL;
}
		
		

void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
