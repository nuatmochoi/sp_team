#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <wiringPi.h> // wiringPi 사용
#include <wiringPiSPI.h> // SPI통신 사용
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 100

#define CS_MCP3008 6 // gpio_25
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

char msg[BUF_SIZE];
void *send_msg(void * arg); // ADC로 부터 값을 받아와서, 조건 처리 후, 출력하는 함수 
void error_handling(char * msg);


int read_mcp3008_adc(unsigned char adcChannel){ // ADC로 부터 값을 읽고 그 값을 adcValue라는 int값으로 반환  
	unsigned char buff[3]; // SPI를 사용해서 ADC로 부터 값을 가져오기 위한 buffer
	int adcValue=0;
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 7); // SPI통신 방식에 따른 buffer 설정 
	buff[1] = ((adcChannel & 0x07) << 6); //상동
	buff[2]=0x00; // 상동 
	digitalWrite(CS_MCP3008,0); // adc 활성화
	wiringPiSPIDataRW(SPI_CHANNEL,buff,3); // data를 읽고
	buff[1]=0x0f & buff[1];
	adcValue = (buff[1] <<8) | buff[2]; // 읽은 데이터를 adcValue에 대입
	digitalWrite(CS_MCP3008,1); // adc 비활성화
	return adcValue;
}

int main(int argc, char ** argv){
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread;
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

	if(connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}

	pthread_create(&snd_thread,NULL,send_msg,(void*)&sock);
	pthread_join(snd_thread,&thread_return);
	close(sock);
	return 0;
}


void *send_msg(void * arg){  // ADC로 부터 값을 받아와서, 조건 처리 후, 출력하는 함수
	int sock = *((int*)arg);
	char name_msg[BUF_SIZE];
	int adcChannel=0; 
	int adcValue=0;



	if(wiringPiSetup()==-1) return NULL; // using gpio
	if (wiringPiSPISetup(SPI_CHANNEL,SPI_SPEED) == -1) return NULL; // using spi
	pinMode(CS_MCP3008,OUTPUT);
	

	while(1) // 4가지 조건에 따라 출력값을 분류
	{
		adcValue=read_mcp3008_adc(adcChannel); // ADC에서 받아온 값 adcValue
		printf("Value = %d\n",adcValue);
		if(adcValue>=2400){
			printf("Water Full\n");
			write(sock,"5\n",2);
		}else if(adcValue>=2000){
			printf("Water Half\n");
			write(sock,"6\n",2);
		}else if(adcValue>=1000){
			printf("Half down\n");
			write(sock,"7\n",2);
		}else{
			printf("Inefficent\n");
			write(sock,"8\n",2);
		}
		fflush(stdout);
		sleep(15);
	}
	return NULL;
}

void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
