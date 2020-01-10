/******************************************
201420871 소프트웨어학과 김창희 작성자
2019.05.24
******************************************/
#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define PROBEPATH "/sys/bus/w1/devices"
#define MAXPROBES 5
#define PROBENAMELEN 80
#define BUFSIZE 256

void *send_msg(void *arg);
void error_handling(char * msg);
char msg[BUF_SIZE];

char probepath[MAXPROBES][PROBENAMELEN];	//온도센서의 갯수에 따른 경로 저장
char probename[MAXPROBES][PROBENAMELEN];	//온도셋서의 갯수에 따를 이름 저장

FILE *probefd;
int numOfSensor;

int findprobes(void){
	struct dirent *pDirent;
	DIR *pDir;
	int count;

	count = 0;
	
	//wiringPi 사용시 온도를 읽어 나타내는 파일 경로 지정
	pDir = opendir(PROBEPATH);
	if(pDir==NULL){
		printf("Cannot open directory '%s'\n",PROBEPATH);
		return 0;
	}

	//DS18B20의 id는 28-로 시작하며 이를 찾기위한 구간
	while((pDirent = readdir(pDir)) != NULL){
		if(pDirent->d_name[0] == '2' && pDirent->d_name[1]=='8'
				&& pDirent->d_name[2] == '-'){

			snprintf(probepath[count],PROBENAMELEN-1,"%s/%s/w1_slave",PROBEPATH,pDirent->d_name);
			snprintf(probename[count],PROBENAMELEN-1,"%s",pDirent->d_name);

			count++;    //디지털 온도센서 사용 갯수
		}
	}

	closedir(pDir);
	return count;
}



int main(int argc, char * argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread;
	void * thread_return;

	//클라이언트 파일 실행 시 IP와 port 지정 
	if(argc!=3){
		printf("Usage: %s <IP> <port>\n",argv[0]);
		exit(1);
	}
	
	//소켓프로토톨 지정
	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1){
		error_handling("socket() error");
	}

	//클라이언트 소켓 형태 설정
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	//서버와의 소켓통신 접속요청
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}

	//온도센서, 수위센서, 강우량센서와 같은 입력센서의 경우 snd_thread로 쓰레드 생성 
	pthread_create(&snd_thread,NULL,send_msg,(void*)&sock);
	//쓰레드 종료대기 상태
	pthread_join(snd_thread,&thread_return);

	close(sock);
	return 0;
}

//send_msg에 역할 진행 스레드 함수
void * send_msg(void * arg){
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];

	int i;
	double temperature;
	char * temp;
	time_t now;
	struct tm *t;
	char buf[BUFSIZE];

	//디지털 온도센서 갯수와 위치 확인
	numOfSensor = findprobes();
	if(numOfSensor==0){
		printf("Error: No DS18B20 compatible probes located.\n");
		exit(-1);
	}

	while(1){
		for(i=0;i<numOfSensor;i++){
			probefd = fopen(probepath[i],"r");		//해당 온도 센서 경로 읽어오기
			if(probefd==NULL){						//온도 센서가 없을 시
				printf("Error\n");
				exit(-1);
			}

			fgets(buf,sizeof(buf)-1,probefd);		//wiringPi 첫 번째 줄에는 온도 정보를 주지 않는다
			memset(buf,0,sizeof(buf));

			fgets(buf,sizeof(buf)-1,probefd);		//두번째줄의 "t=" 토큰값을 이용해 온도를 가져온다.
			temp = strtok(buf,"t=");
			temp = strtok(NULL,"t=");
			temperature = atof(temp)/1000;

			//온도를 1000으로 나눈값이 30도 이상일 때 서버에게 "1\n"를 전달
			if(temperature>=30){
				write(sock,"1\n",2);
			}else		//온도를 1000으로 나누었을때 30도 미만인 경우 서버에게 "2\n"를 전달
			{
				write(sock,"2\n",2);
			}


			now = time(NULL);		//현재시간 날짜를 사용하기위한 준비구간
			t = localtime(&now);	//실제 현재 코드에서는 사용하지 않음

			printf("%2.3f\n",temperature);	//현재온도 출력

			fclose(probefd);
			fflush(stdout);
			sleep(15);				//15초 간격으로 입력센서 값 입력
		}
	}
}

//에러처리에 대한 함수
void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
