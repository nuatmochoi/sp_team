/* **************************************
	작성자: 최성우(201420913)
	소켓통신: 김창희(201420871)
	작성일자 : 2019.05.31
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

	if(argc!=3){		//클라이언트단에서의 서버의 IP와 port번호 
		printf("Usage: %s <IP> <port>\n",argv[0]);
		exit(1);
	}

	sock = socket(PF_INET,SOCK_STREAM,0);	//IPv4와 TCP프로토콜 형태로 소켓 생성
	if(sock==-1){
		error_handling("socket() error");
	}
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	//서버와의 소켓 연결
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}

	//출입구, 유수관, 하수구, 창문, 부저의 디바이스의 경우 입력센서로부터 값을 입력 받아야 한다.(recv_msg 함수 실행)
	pthread_create(&rcv_thread,NULL,recv_msg,(void*)&sock);
	pthread_join(rcv_thread,&thread_return);	//쓰레드 종료대기
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
	sensor_arr 배열 값 전부 -1로 초기화 상태이다.
	sensor_arr[1] = 온도 30도 이상 상태, 
	sensor_arr[2] = 온도 30도 이하 상태, 
	sensor_arr[3] = 빗물감지센서 on 상태, 
	sensor_arr[4] = 빗물감지센서 off 상태,
	sensor_arr[5] = 물이 꽉참(수위센서 full),
	sensor_arr[6] = 물이 절반이상(수위센서 half 이상),
	sensor_arr[7] = 물이 절반이하(수위센서 half 이하),
	sensor_arr[8] = 물이 거의없음
	******************************************************/
	while(1){
		int sensor_arr[9];
		memset(sensor_arr,-1,sizeof(sensor_arr));
		int i=0;
		char cur_num;

		//입력센서로부터 3개의 값을 받아야한다.
		while(1){
			str_len = read(sock,name_msg,BUF_SIZE-1);
			if(str_len>0 && i<3){	//입력센서로부터 3개의 값을 받지 못했을 때
				name_msg[str_len]=0;

				if(sensor_arr[name_msg[0]-'0']==-1){		//입력센서로부터 중복된 값이 아닌 값을 받았을 때
					printf("num : %d", i++);			
					sensor_arr[name_msg[0]-'0']=1;
					fputs(name_msg,stdout);

					if(name_msg[2]-'0'>=0 && name_msg[2]-'0'<=9){	//입력센서의 delay로 인해 2개의 값을 연달아 받을경우
						printf("delay num : %d", i++);
						sensor_arr[name_msg[2]-'0']=1;
					}
				}
			}
			else{		//입력센서로부터 3개의 값을 모두 받은상태
				i=0;
				break;
			}
		}

		//입력받은 센서값 출력
		for(int t=0;t<9;t++){
			printf("%d ",sensor_arr[t]);
		}
		puts("");

		//sensor_arr의 idx 1번,4번,5번,6번 => 온도가 30도 이상, 강우 오지 않음, 물탱크 물이 꽉차있거나 절반이상시 부저 울림
		if(sensor_arr[1]==1 && sensor_arr[4]==1 && (sensor_arr[5]==1 || sensor_arr[6]==1)){
			if(flag==0){	//바로이전에 부저가 울리지 않은 상태라면 
			int len = write(fd, &input,1);	//디바이스단에 값 1 전달
			flag=1;
			}
			else{
				cnt++;		//바로 이전에 부저가 울린 상태일 경우
				if(cnt==1)	//연속하지않는 정도, 즉 cnt값이 1이 되었을시 flag값 다시 초기화
					flag=0;
			}
		}

	}

	return NULL;
}

//에러 함수 처리
void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
