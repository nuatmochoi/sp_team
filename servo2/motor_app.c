/* **************************************
	작성자: 최성우(201420913)
	소켓통신: 김창희(201420871)
	작성일자 : 2019.06.09
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

	// 시나리오에 맞는 센서들의 값을 저장하기 위해 int형 배열 선언 및 -1로 초기화, -1은 센서 값이 활성화 되지 않은 상태
	// sensor_arr[1] = 온도 30도 이상 상태, sensor_arr[2] = 온도 30도 이하 상태, sensor_arr[3] = 빗물감지센서 on 상태, sensor_arr[4] = 빗물감지센서 off 상태
	// sensor_arr[5] = 물이 꽉참(수위센서 full), sensor_arr[6] = 물이 절반이상(수위센서 half 이상), sensor_arr[7] = 물이 절반이하(수위센서 half 이하), sensor_arr[8] = 물이 거의없음
	
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

		//외벽에 물을 뿌리기 전에 창문을 닫기 위해 조건처리를 한 부분
		//비가 오지 않고, 온도가 높으며, 물탱크에 물이 없지 않을때 창문이 닫히도록 설계
		if(sensor_arr[1]==1 && sensor_arr[4]==1 && (sensor_arr[5]==1 || sensor_arr[6]==1||sensor_arr[7]==1)){
			sleep(2); //외벽 유수관 모터 개방 이전, 부저가 울린 이후로 순서를 지정하기 위함 
			to_dev='0';
			w=write(fd,&to_dev,1);
			printf("Window OPEN\n");

		}
		else if(sensor_arr[8]==1){ // 물탱크에 물이 없다면 닫히도록 설계
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
