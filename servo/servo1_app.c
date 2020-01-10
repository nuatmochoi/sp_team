/******************************************
201420927 소프트웨어학과 서지용 작성자
201420871 소프트웨어학과 김창희 소켓통신
2019.6.3
******************************************/
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
  	
	fd = open("/dev/servo1_dev", O_RDWR);
	if(fd < 0){
		perror("File open failed : ");
		exit(-1);
	}
	printf("while before\n");
	int flag = 0;	
	 while(1){
		 
		 int sensor_arr[9];// 시나리오에 맞는 센서들의 값을 저장하기 위해 int형 배열 선언 및 -1로 초기화, -1은 센서 값이 활성화 되지 않은 상태
		 // sensor_arr[1] = 온도 30도 이상 상태, sensor_arr[2] = 온도 30도 이하 상태, sensor_arr[3] = 빗물감지센서 on 상태, sensor_arr[4] = 빗물감지센서 off 상태
		 // sensor_arr[5] = 물이 꽉참(수위센서 full), sensor_arr[6] = 물이 절반이상(수위센서 half 이상), sensor_arr[7] = 물이 절반이하(수위센서 half 이하), sensor_arr[8] = 물이 거의없음
		 memset(sensor_arr,-1,sizeof(sensor_arr));
		 i=0;	
		 char cur_num;
		 while(1){
			 str_len = read(sock,name_msg,BUF_SIZE-1); // 서버로부터 센서 값을 읽어들임
			 if(str_len>0 && i<3){
				name_msg[str_len]=0;
			
				if(sensor_arr[name_msg[0]-'0']==-1){
					printf("num: %d",i++);
					sensor_arr[name_msg[0]-'0'] = 1;
					fputs(name_msg,stdout);
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
		
		if(sensor_arr[3] == 1 && flag == 0){ // 빗물감지 센서가 on 상태인 경우, 즉 비가오고 있는 경우 물을 받기 위해 물탱크 입구를 개방함
			printf("Door open\n");
			sleep(4); // sleep을 주는 이유는 빗물을 받기 전에 하수구 개방을 하여
			k='0';  // 문자 '0'값을 디바이스 단으로 write 하여 모터 각도 180도를 만들어줌
			fp = write(fd,&k,1);

			if(sensor_arr[5] == 1){ // 비가 오고 있는 상태에서 수위센서를 통해 물탱크에 물이 꽉찼음을 확인 할 경우 물탱크 입구를 닫아줌
				flag=1; // flag를 사용하여 비가 온 뒤 물탱크가 꽉찬 다음에도 계속 비가 올 경우에는 물탱크 입구를 열지 않도록 조건을 줌
				printf("Door close\n");
				k='1'; // 문자 '1'값을 디바이스 단으로 write 하여 모터 각도 0도를 만들어줌
				fp = write(fd,&k,1);
			}
			waterSensor = 0;
		}
		else{
			if(sensor_arr[4] == 1){ // 빗물감지 센서가 off 상태인 경우, 즉 비가 멈춘 경우나 비가 오지 않는 경우 물탱크 입구를 닫음
				flag = 0;
				printf("Door close\n");
				k='1'; // 문자 '1'값을 디바이스 단으로 write 하여 모터 각도 0도를 만들어줌
				fp = write(fd,&k,1);
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

