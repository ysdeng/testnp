#include "HW3_102062335.h"

int port;

typedef struct {
	int connfd;
	char name[30];
	char ip[100];
	int port;
} Member;
Member self;

void help();
void updatef(int sockfd);
void *tcpListener();
char user[40];

int main(int argc, char **argv) {
	int sockfd;
	int n, i, j;
	pthread_t ttcp;
	struct sockaddr_in servaddr;

	pthread_create(&ttcp, NULL, &tcpListener, NULL);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port = htons(atoi(argv[2]));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	connect(sockfd, (SA*)&servaddr, sizeof(servaddr));

	char msg[MAXLINE];

	while(1) {
		scanf("%s", msg);
		if(!strcmp(msg, "help")) {
			help();
		}
		if(!strcmp(msg, "login")) {
			//send cmd to server
			write(sockfd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
			printf("username: ");
			scanf("%s", msg);

			/*- only for test -*/
			char sss[100];
			sprintf(user, "%s", msg);
			sprintf(sss, "mkdir %s", msg);
			system(sss);

			//send username
			write(sockfd, msg, strlen(msg));
			read(sockfd, msg, sizeof(msg));
			if(!strcmp(msg, "0")) {
				puts("server overloaded");
				exit(0);
			}
			else {
				printf("login success\n");

				/*- update current files to server -*/
				updatef(sockfd);
				puts("Data update is complete.");

				bzero(msg, sizeof(msg));
			}
		}
		if(!strcmp(msg, "logout")) {
			//send cmd to server
			write(sockfd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
			read(sockfd, msg, MAXLINE);
			puts(msg);
			exit(0);
		}
		if(!strcmp(msg, "listu")) {
			//send cmd to server
			write(sockfd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
			read(sockfd, msg, MAXLINE);
			printf("%s", msg);
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "updatef")) {
			updatef(sockfd);
		}
		if(!strcmp(msg, "listf")) {
			//send cmd to server
			write(sockfd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
			read(sockfd, msg, MAXLINE);
			printf("%s", msg);
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "connect")) {
			char ip[40];
			scanf("%s", ip);
			struct sockaddr_in dstaddr;
			int dstfd;
			dstfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&dstaddr, sizeof(dstaddr));
			servaddr.sin_port = htons(1500);
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = inet_addr(ip);
			connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));
			read(dstfd, msg, MAXLINE);
			puts(msg);
			bzero(msg, sizeof(msg));
		}
	}
	return 0;
}

void help () {
	printf("we provide following service\n");
	printf("login\n");
	printf("logout\n");
	printf("listu: list online user's infos\n");
	printf("listf: list online user's files\n");
	printf("updatef: update local files\n");
}

void updatef(int sockfd) {
	char msg[MAXLINE];
	printf("Updating data...\n");
	write(sockfd, "updatef", strlen("updatef"));

	char aaa[200];
	sprintf(aaa, "ls -1 %s/ | wc -l >list",user);
	system(aaa);

	//system("ls -1 DataCli/ | wc -l >list");
	FILE *fp;
	fp = fopen("list", "r");
	int count;
	if(fp != NULL) {
		fscanf(fp, "%d", &count);
		fclose(fp);
	}

	//send file count to server
	sprintf(msg, "%d", count);
	write(sockfd, msg, strlen(msg));
	
	sprintf(aaa, "ls %s/ >list",user);
	system(aaa);

	//system("ls DataCli/ > list");
	fp = fopen("list", "r");
	if(fp != NULL) {
		int i;
		char fname[40];
		for(i = 0; i < count; i++) {
			fscanf(fp, "%s", fname);
			puts(fname);
			strcpy(msg, fname);
			write(sockfd, msg, strlen(msg));
			while(1) {
				bzero(msg, sizeof(msg));
				read(sockfd, msg, MAXLINE);
				if(!strcmp(msg, "ACK")) break;
			}
		}
		fclose(fp);
	}
	system("rm list");
}

void* tcpListener() {
	puts("thread start");
	int port = 1500;
	printf("tcpport: %d\n", port);
	struct sockaddr_in tcpSer, client;
	int tcpfd;

	tcpfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&tcpSer, sizeof(tcpSer));

	tcpSer.sin_family = AF_INET;
	tcpSer.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpSer.sin_port = htons(port);
	
	bind(tcpfd, (SA*)&tcpSer, sizeof(tcpSer));
	listen(tcpfd, LISTENQ);

	int connfd;
	socklen_t chilen;
	bzero(&client, sizeof(client));

	while(1) {
		chilen = sizeof(client);
        connfd = accept(tcpfd, (SA *)&client, &chilen);
        printf("Connect...\nIP address is: %s, ", inet_ntoa(client.sin_addr));
        printf("port is: %d\n", (int) ntohs(client.sin_port));
        write(connfd, "hi", strlen("hi"));
        close(connfd);
	}
}