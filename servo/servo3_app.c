/******************************************
201420927 소프트웨어학과 서지용 작성자
201420871 소프트웨어학과 김창희 소켓통신
2019.06.07
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
	
	fd = open("/dev/servo3_dev", O_RDWR);
	if(fd < 0){
		perror("File open failed : ");
		exit(-1);
	}
	printf("while before\n");
	
	 while(1){
		 
		 int sensor_arr[9];// 시나리오에 맞는 센서들의 값을 저장하기 위해 int형 배열 선언 및 -1로 초기화, -1은 센서 값이 활성화 되지 않은 상태
		 // sensor_arr[1] = 온도 30도 이상 상태, sensor_arr[2] = 온도 30도 이하 상태, sensor_arr[3] = 빗물감지센서 on 상태, sensor_arr[4] = 빗물감지센서 off 상태
		 // sensor_arr[5] = 물이 꽉참(수위센서 full), sensor_arr[6] = 물이 절반이상(수위센서 half 이상), sensor_arr[7] = 물이 절반이하(수위센서 half 이하), sensor_arr[8] = 물이 거의없음
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
		
		if(sensor_arr[4] == 1 && (sensor_arr[5]==1 || sensor_arr[6]==1) && sensor_arr[1]==1){ // 비가 오지 않으면서 물이 꽉 차 있거나 절반 이상인 경우, 동시에 온도가 30도 이상이면
			sleep(5); //시나리오 상 외벽에 물을 뿌려야 하는 경우이다. 그 전에 창문을 닫아야 함으로 sleep을 통해 지연시킨다.
			printf("Door open\n"); // 유수관 입구를 개방한다.
			k='0'; 
			fp = write(fd,&k,1);
			fflush(stdin);
		}
		else if(sensor_arr[3] == 1 || sensor_arr[8] == 1) //외벽에 물을 뿌리고 난 경우 즉 물이 거의 없으면 문을 닫아준다. 비가 오고 있을 때도 열리지 않아야 한다.
		{
			printf("Door close\n");
			k='1';
			fp = write(fd,&k,1);
			fflush(stdin);
		}

		
	}	

  return NULL;
}

void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}

