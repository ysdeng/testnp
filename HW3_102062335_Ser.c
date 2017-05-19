#include "HW3_102062335.h"

typedef struct{
	int clifd;
	struct sockaddr_in cliaddr;
} clientSock;

typedef struct {
	int connfd;
	struct sockaddr_in socketaddr;
	char name[30];
	int port;
	int valid;
	char files[200][40];
	int filecount;
} Member;
Member member[1024];
pthread_mutex_t memberLock = PTHREAD_MUTEX_INITIALIZER;
void init();

void* service( void *argv);

int main( int argc, char **argv) {
	init();

	int sockfd;
	pthread_t tid;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	
	bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
	listen(sockfd, LISTENQ);

	while(1){
		clientSock* cliInfo; 
		cliInfo = malloc(sizeof(clientSock));
		socklen_t addrlen = sizeof(cliInfo->cliaddr);
		cliInfo->clifd = accept(sockfd, (struct sockaddr*)&(cliInfo->cliaddr), &addrlen);
		printf("%s\n", inet_ntoa(cliInfo->cliaddr.sin_addr));
		pthread_create(&tid, NULL, &service, (void *)cliInfo);
	}

	return 0;
}

void init() {
	/*- read some user data -*/
	system("mkdir serData");
	system("cd serData");
}

void* service( void *arg) {
	int connfd = *((int *)arg);
	clientSock clisock = *((clientSock*)arg);
	free(arg);
	char buf[MAXLINE];
	char msg[MAXLINE];
	//printf("%s\n", inet_ntoa(clisock.cliaddr.sin_addr));
	int n;
	while(1) {
		bzero(buf, sizeof(buf));
		bzero(msg, sizeof(msg));
		read(clisock.clifd, msg, MAXLINE);
		if(!strcmp(msg, "login")) {
			puts("login");
			bzero(msg, sizeof(msg));
			//get username
			read(clisock.clifd, msg, MAXLINE);

			pthread_mutex_lock(&memberLock);
			int i, full = 1, idx;
			for(i = 0; i < 1024; i++) {
				if(member[i].valid == 0) {
					idx = i;
					full = 0;
					member[i].valid = 1;
					strcpy(member[i].name, msg);
					member[i].connfd = clisock.clifd;
					member[i].socketaddr = clisock.cliaddr;
					member[i].port = (int)ntohs(clisock.cliaddr.sin_port);
					break;
				}
			}
			pthread_mutex_unlock(&memberLock);
			if(!full) {
				printf("welcome %s\n", member[idx].name);
				printf("\t%s", inet_ntoa(member[i].socketaddr.sin_addr));
				printf(" %d\n", member[i].port);
				
			}
			//send final state
			sprintf(msg, "%d", (full)?0:member[i].port);
			write(clisock.clifd, msg, strlen(msg));
		}
		if(!strcmp(msg, "logout")) {
			puts("logout");
			bzero(msg, sizeof(msg));

			pthread_mutex_lock(&memberLock);
			int i, success = 0;
			for(i = 0; i < 1024; i++) {
				if(member[i].valid == 1 && clisock.clifd == member[i].connfd) {
					member[i].valid = 0;
					success = 1;
					printf("bye-bye %s\n", member[i].name);
					break;
				}
			}
			pthread_mutex_unlock(&memberLock);
			if(success) {
				write(clisock.clifd, "bye-bye", strlen("bye-bye"));
				close(clisock.clifd);
				return (NULL);
			}
		}
		if(!strcmp(msg, "listu")) {
			puts("list online user");
			bzero(msg, sizeof(msg));
			strcpy(msg, "");

			pthread_mutex_lock(&memberLock);
			int i;
			for(i = 0; i < 1024; i++) {
				if(member[i].valid == 1) {
					char aaa[MAXLINE];
					int kerker = 0;
					if(clisock.clifd == member[i].connfd) kerker = 1;
					sprintf(aaa, "%s %s %d %s\n", member[i].name, 
						inet_ntoa(member[i].socketaddr.sin_addr), member[i].port,
						(kerker)?"**": "");

					strcat(msg, aaa);
				}
			}
			pthread_mutex_unlock(&memberLock);
			write(clisock.clifd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "updatef")) {
			puts("user file list update");
			bzero(msg, sizeof(msg));
			//get file count
			read(clisock.clifd, msg, MAXLINE);
			int count = atoi(msg), i;
			bzero(msg, sizeof(msg));
			//find user
			int idx;
			pthread_mutex_lock(&memberLock);
			for(i = 0; i < 1024; i++) {
				if(member[i].valid == 1 && clisock.clifd == member[i].connfd) {
					idx = i;
					member[i].filecount = count;
					break;
				}
			}
			printf("%s has %d files\n", member[idx].name, member[idx].filecount);
			for(i = 0; i < count; i++) {
				read(clisock.clifd, msg, MAXLINE);
				sprintf(member[idx].files[i],"%s", msg);
				bzero(msg, sizeof(msg));
				puts(member[idx].files[i]);
				write(clisock.clifd, "ACK", strlen("ACK"));
			}
			pthread_mutex_unlock(&memberLock);
		}
		if(!strcmp(msg, "listf")) {
			puts("list online user's files");
			bzero(msg, sizeof(msg));
			strcpy(msg, "");

			pthread_mutex_lock(&memberLock);
			int i;
			for(i = 0; i < 1024; i++) {
				int kerker = 0;
				if(member[i].valid == 1) {
					char aaa[MAXLINE];
					if(clisock.clifd == member[i].connfd) kerker = 1;
					sprintf(aaa, "%s %s\n", member[i].name, 
						(kerker)?"**": "");

					strcat(msg, aaa);
					int j;
					if(member[i].filecount) {
						for(j = 0; j < member[i].filecount; j++) {
							sprintf(aaa, "%s\n", member[i].files[j]);
							strcat(msg, aaa);
						}
					}
					else {
						sprintf(aaa, "no file\n");
						strcat(msg, aaa);
					}
				}
			}
			pthread_mutex_unlock(&memberLock);
			write(clisock.clifd, msg, strlen(msg));
			bzero(msg, sizeof(msg));

		}

	}
}