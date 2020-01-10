/******************************************
201420871 소프트웨어학과 김창희
2019.5.31
******************************************/
#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char *msg,int len);
void error_handling(char * message);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char * argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	pthread_t t_id;

	//소켓통신 서버 입력 형태 ./chat_serv 0000(포트번호)
	if(argc!=2){
		printf("Usage:%s <port>\n",argv[0]);
		exit(1);
	}

	//pthread_mutex 시작을 위한 주소값 전달
	pthread_mutex_init(&mutx,NULL);

	//소켓 IPv4,TCP 호출 
	serv_sock = socket(PF_INET,SOCK_STREAM,0);
	if(serv_sock==-1){
		error_handling("socket() error");
	}

	//서버 주소값 초기화 및 주소형태 & 포트 설정
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	//bind()를 통해 서버 소켓 등록
	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1){
		error_handling("bind() error");
	}

	//서버 소케을 통해 클라이언트의 접속요청
	if(listen(serv_sock,10)==-1){
		error_handling("listen() error");
	}

	//여러 클라이언트와 저ㅂ속하기 위한 함수
	while(1){
		clnt_adr_sz = sizeof(clnt_adr);

		//클라이언트 소켓 접속
		clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		if(clnt_sock==-1){
			error_handling("accept() error");
		}

		//pthread mutex의 임계영역의 시작
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;  //클라이언트 소켓들을 담아둠
		pthread_mutex_unlock(&mutx);	//클라이언트 임계영역의 끝

		pthread_create(&t_id,NULL,handle_clnt,(void*)&clnt_sock); //쓰레드 함수호출
		pthread_detach(t_id);	//쓰레드 함수 호출 완료시 자동으로 쓰레드 소멸
		printf("Connected client: IP: %s\n",inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

//쓰레드 서버단 함수 처리
void * handle_clnt(void*arg){
	int clnt_sock=*((int*)arg);
	int str_len = 0;
	char msg[BUF_SIZE];

	//클라이언트 소켓으로부터 값을 읽어들임
	while((str_len = read(clnt_sock,msg,sizeof(msg)))!=0){
		msg[str_len]=0;
		printf("msg: %s",msg);
		//클라이언트 단에게 메세지를 보내는 곳
		send_msg(msg,str_len);
	}

	pthread_mutex_lock(&mutx);		//pthread mutx동기화

	//클라이언트 소켓 끈었을 경우
	for(int i=0;i<clnt_cnt;i++){
		if(clnt_sock==clnt_socks[i]){
			while(i++<clnt_cnt-1){
				clnt_socks[i]=clnt_socks[i+1];
			}
			break;
		}
	}

	clnt_cnt--;
	pthread_mutex_unlock(&mutx);	//pthread mutx임계영역 끝
	close(clnt_sock);
	return NULL;
}

void send_msg(char * msg,int len){

	pthread_mutex_lock(&mutx);		//pthread  mutx동기화 시작
	for(int i=0;i<clnt_cnt;i++){	//클라이언트 단에게 메세지 작성
		write(clnt_socks[i],msg,len);
	}
	pthread_mutex_unlock(&mutx);	//pthread mutx동기화 종료
}

//에러 메세지 처리
void error_handling(char * message){
	fputs(message,stderr);
	fputc('\n',stderr);
	exit(1);
}
