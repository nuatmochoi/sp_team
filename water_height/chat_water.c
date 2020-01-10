#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <wiringPi.h> // wiringPi ���
#include <wiringPiSPI.h> // SPI��� ���
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 100

#define CS_MCP3008 6 // gpio_25
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

char msg[BUF_SIZE];
void *send_msg(void * arg); // ADC�� ���� ���� �޾ƿͼ�, ���� ó�� ��, ����ϴ� �Լ� 
void error_handling(char * msg);


int read_mcp3008_adc(unsigned char adcChannel){ // ADC�� ���� ���� �а� �� ���� adcValue��� int������ ��ȯ  
	unsigned char buff[3]; // SPI�� ����ؼ� ADC�� ���� ���� �������� ���� buffer
	int adcValue=0;
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 7); // SPI��� ��Ŀ� ���� buffer ���� 
	buff[1] = ((adcChannel & 0x07) << 6); //��
	buff[2]=0x00; // �� 
	digitalWrite(CS_MCP3008,0); // adc Ȱ��ȭ
	wiringPiSPIDataRW(SPI_CHANNEL,buff,3); // data�� �а�
	buff[1]=0x0f & buff[1];
	adcValue = (buff[1] <<8) | buff[2]; // ���� �����͸� adcValue�� ����
	digitalWrite(CS_MCP3008,1); // adc ��Ȱ��ȭ
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


void *send_msg(void * arg){  // ADC�� ���� ���� �޾ƿͼ�, ���� ó�� ��, ����ϴ� �Լ�
	int sock = *((int*)arg);
	char name_msg[BUF_SIZE];
	int adcChannel=0; 
	int adcValue=0;



	if(wiringPiSetup()==-1) return NULL; // using gpio
	if (wiringPiSPISetup(SPI_CHANNEL,SPI_SPEED) == -1) return NULL; // using spi
	pinMode(CS_MCP3008,OUTPUT);
	

	while(1) // 4���� ���ǿ� ���� ��°��� �з�
	{
		adcValue=read_mcp3008_adc(adcChannel); // ADC���� �޾ƿ� �� adcValue
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
