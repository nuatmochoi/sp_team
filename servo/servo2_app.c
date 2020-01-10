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
	

		printf("WaterSensor: %d\n",waterSensor);	
		if(sensor_arr[3] == 1 && flag == 0){ // 빗물감지 센서가 on 상태인 경우, 즉 비가오고 있는 경우 하수구 유수관을 열어서 기존에 물탱크에 있던 물을 빼준다
			printf("Door open\n");
			k='0';
			fp = write(fd,&k,1);
			if(sensor_arr[8] == 1){ // 물이 거의 없다고 판단 되면 다시 하수구 유수관을 닫는다.
				flag++;
				printf("Door close\n");
				k='1';
				fp = write(fd,&k,1);
			}
			waterSensor = 0;
		}
		else{
			if(sensor_arr[4] == 1){ // 빗물감지 센서가 off 상태인 경우, 즉 비가오고 있지 않는 경우 flag를 0으로 만들어서 다음번에 비가 올 때 하수구 유수관을 열 수 있도록 한다.
				flag = 0;
			}
			if(sensor_arr[1] == 1 && sensor_arr[4]==1)	waterSensor = 0; // 온도가 30도 이상이면서 비가 오지 않는 경우 waterSenor값을 0으로 설정한다. 이는 시나리오 상 외벽에 물을 뿌린 후 이기 때문에
																		// waterSeonsor 값으로 물이 고여 있지 않음을 알려준다.
			if(sensor_arr[2] == 1 && sensor_arr[4]==1){ // 온도가 30도 이하면서 비가 오지 않는 경우, 즉 날씨가 덥지 않다고 판단될 경우
				waterSensor++; //waterSensor 값으로 그 주기를 계속 확인한다.
				if(waterSensor > 3 && (sensor_arr[5]==1 || sensor_arr[6]==1 || sensor_arr[7]==1)){ // 시나리오 상은 5일이 지났을 때지만 빠른 확인을 위하여 4번 이상 감지되었을 때, 이 때 물이 거의 없는 상태가 아닌 경우
					printf("Door open\n"); // 하수구를 개방한다.
					k='0';
					fp = write(fd,&k,1);
					waterSensor=0; //waterSensor 값을 0으로 다시 만들어 물이 빠져나가서 고여 있지 않음을 알려준다.
				}
				else{
					printf("Door close\n"); // 물을 뺀 뒤면 waterSensor 값이 0이기 때문에 문을 닫고, 물탱크에 물이 없거나 Sensor 값이 3 이하인 경우는 하수구 문을 열지 않는다.
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

