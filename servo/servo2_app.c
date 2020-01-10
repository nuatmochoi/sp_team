#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 100

void * recv_msg(void * arg);
void error_handling(char *msg);
char msg[BUF_SIZE];

int main(int argc, char * argv[]){


	int sock;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;
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

	pthread_create(&rcv_thread,NULL,recv_msg,(void*)&sock);
	pthread_join(rcv_thread,&thread_return);
	close(sock);
	return 0;
}

void * recv_msg(void *arg){

	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];
	int str_len;

	int waterSensor=0;
    char k;
  	int fd, fp, i;
	
	fd = open("/dev/servo2_dev", O_RDWR);
	if(fd < 0){
		perror("File open failed : ");
		exit(-1);
	}
	printf("while before\n");
	int flag = 0;	
	 while(1){
		 
		 int sensor_arr[9];// �ó������� �´� �������� ���� �����ϱ� ���� int�� �迭 ���� �� -1�� �ʱ�ȭ, -1�� ���� ���� Ȱ��ȭ ���� ���� ����
		 // sensor_arr[1] = �µ� 30�� �̻� ����, sensor_arr[2] = �µ� 30�� ���� ����, sensor_arr[3] = ������������ on ����, sensor_arr[4] = ������������ off ����
		 // sensor_arr[5] = ���� ����(�������� full), sensor_arr[6] = ���� �����̻�(�������� half �̻�), sensor_arr[7] = ���� ��������(�������� half ����), sensor_arr[8] = ���� ���Ǿ���
		 memset(sensor_arr,-1,sizeof(sensor_arr));
		 i=0;
		
		 char cur_num;
		 while(1){
			 str_len = read(sock,name_msg,BUF_SIZE-1);
			 if(str_len>0 && i<3){
				name_msg[str_len]=0;
			
				if(sensor_arr[name_msg[0]-'0']==-1){
					printf("num: %d",i++);
					fputs(name_msg,stdout);
					sensor_arr[name_msg[0]-'0'] = 1;
					if(name_msg[2]-'0'>=0 && name_msg[2]-'0'<=9){
						printf("delay num: %d",i++);
						sensor_arr[name_msg[2]-'0'] = 1;
					}
				}
			 }else{
				 i=0;
				 break;
			 }
		 }
		
		for(i=0;i<9;i++){
			printf("%d ",sensor_arr[i]);
		}printf("\n");
	

		printf("WaterSensor: %d\n",waterSensor);	
		if(sensor_arr[3] == 1 && flag == 0){ // �������� ������ on ������ ���, �� �񰡿��� �ִ� ��� �ϼ��� �������� ��� ������ ����ũ�� �ִ� ���� ���ش�
			printf("Door open\n");
			k='0';
			fp = write(fd,&k,1);
			if(sensor_arr[8] == 1){ // ���� ���� ���ٰ� �Ǵ� �Ǹ� �ٽ� �ϼ��� �������� �ݴ´�.
				flag++;
				printf("Door close\n");
				k='1';
				fp = write(fd,&k,1);
			}
			waterSensor = 0;
		}
		else{
			if(sensor_arr[4] == 1){ // �������� ������ off ������ ���, �� �񰡿��� ���� �ʴ� ��� flag�� 0���� ���� �������� �� �� �� �ϼ��� �������� �� �� �ֵ��� �Ѵ�.
				flag = 0;
			}
			if(sensor_arr[1] == 1 && sensor_arr[4]==1)	waterSensor = 0; // �µ��� 30�� �̻��̸鼭 �� ���� �ʴ� ��� waterSenor���� 0���� �����Ѵ�. �̴� �ó����� �� �ܺ��� ���� �Ѹ� �� �̱� ������
																		// waterSeonsor ������ ���� �� ���� ������ �˷��ش�.
			if(sensor_arr[2] == 1 && sensor_arr[4]==1){ // �µ��� 30�� ���ϸ鼭 �� ���� �ʴ� ���, �� ������ ���� �ʴٰ� �Ǵܵ� ���
				waterSensor++; //waterSensor ������ �� �ֱ⸦ ��� Ȯ���Ѵ�.
				if(waterSensor > 3 && (sensor_arr[5]==1 || sensor_arr[6]==1 || sensor_arr[7]==1)){ // �ó����� ���� 5���� ������ ������ ���� Ȯ���� ���Ͽ� 4�� �̻� �����Ǿ��� ��, �� �� ���� ���� ���� ���°� �ƴ� ���
					printf("Door open\n"); // �ϼ����� �����Ѵ�.
					k='0';
					fp = write(fd,&k,1);
					waterSensor=0; //waterSensor ���� 0���� �ٽ� ����� ���� ���������� �� ���� ������ �˷��ش�.
				}
				else{
					printf("Door close\n"); // ���� �� �ڸ� waterSensor ���� 0�̱� ������ ���� �ݰ�, ����ũ�� ���� ���ų� Sensor ���� 3 ������ ���� �ϼ��� ���� ���� �ʴ´�.
					k='1';
					fp = write(fd,&k,1);
				}
			}
			
		}
		
	}	

  return NULL;
}

void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

