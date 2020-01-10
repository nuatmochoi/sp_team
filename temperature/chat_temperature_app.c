/******************************************
201420871 ����Ʈ�����а� ��â�� �ۼ���
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

char probepath[MAXPROBES][PROBENAMELEN];	//�µ������� ������ ���� ��� ����
char probename[MAXPROBES][PROBENAMELEN];	//�µ��¼��� ������ ���� �̸� ����

FILE *probefd;
int numOfSensor;

int findprobes(void){
	struct dirent *pDirent;
	DIR *pDir;
	int count;

	count = 0;
	
	//wiringPi ���� �µ��� �о� ��Ÿ���� ���� ��� ����
	pDir = opendir(PROBEPATH);
	if(pDir==NULL){
		printf("Cannot open directory '%s'\n",PROBEPATH);
		return 0;
	}

	//DS18B20�� id�� 28-�� �����ϸ� �̸� ã������ ����
	while((pDirent = readdir(pDir)) != NULL){
		if(pDirent->d_name[0] == '2' && pDirent->d_name[1]=='8'
				&& pDirent->d_name[2] == '-'){

			snprintf(probepath[count],PROBENAMELEN-1,"%s/%s/w1_slave",PROBEPATH,pDirent->d_name);
			snprintf(probename[count],PROBENAMELEN-1,"%s",pDirent->d_name);

			count++;    //������ �µ����� ��� ����
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

	//Ŭ���̾�Ʈ ���� ���� �� IP�� port ���� 
	if(argc!=3){
		printf("Usage: %s <IP> <port>\n",argv[0]);
		exit(1);
	}
	
	//������������ ����
	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1){
		error_handling("socket() error");
	}

	//Ŭ���̾�Ʈ ���� ���� ����
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	//�������� ������� ���ӿ�û
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1){
		error_handling("connect() error");
	}

	//�µ�����, ��������, ���췮������ ���� �Է¼����� ��� snd_thread�� ������ ���� 
	pthread_create(&snd_thread,NULL,send_msg,(void*)&sock);
	//������ ������ ����
	pthread_join(snd_thread,&thread_return);

	close(sock);
	return 0;
}

//send_msg�� ���� ���� ������ �Լ�
void * send_msg(void * arg){
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];

	int i;
	double temperature;
	char * temp;
	time_t now;
	struct tm *t;
	char buf[BUFSIZE];

	//������ �µ����� ������ ��ġ Ȯ��
	numOfSensor = findprobes();
	if(numOfSensor==0){
		printf("Error: No DS18B20 compatible probes located.\n");
		exit(-1);
	}

	while(1){
		for(i=0;i<numOfSensor;i++){
			probefd = fopen(probepath[i],"r");		//�ش� �µ� ���� ��� �о����
			if(probefd==NULL){						//�µ� ������ ���� ��
				printf("Error\n");
				exit(-1);
			}

			fgets(buf,sizeof(buf)-1,probefd);		//wiringPi ù ��° �ٿ��� �µ� ������ ���� �ʴ´�
			memset(buf,0,sizeof(buf));

			fgets(buf,sizeof(buf)-1,probefd);		//�ι�°���� "t=" ��ū���� �̿��� �µ��� �����´�.
			temp = strtok(buf,"t=");
			temp = strtok(NULL,"t=");
			temperature = atof(temp)/1000;

			//�µ��� 1000���� �������� 30�� �̻��� �� �������� "1\n"�� ����
			if(temperature>=30){
				write(sock,"1\n",2);
			}else		//�µ��� 1000���� ���������� 30�� �̸��� ��� �������� "2\n"�� ����
			{
				write(sock,"2\n",2);
			}


			now = time(NULL);		//����ð� ��¥�� ����ϱ����� �غ񱸰�
			t = localtime(&now);	//���� ���� �ڵ忡���� ������� ����

			printf("%2.3f\n",temperature);	//����µ� ���

			fclose(probefd);
			fflush(stdout);
			sleep(15);				//15�� �������� �Է¼��� �� �Է�
		}
	}
}

//����ó���� ���� �Լ�
void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
