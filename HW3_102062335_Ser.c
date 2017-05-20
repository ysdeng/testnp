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
char serfile[200][40];
int serfileCount;

void* service( void *argv);
void listServerFile();
FILE *file;
pthread_mutex_t fileLock = PTHREAD_MUTEX_INITIALIZER;

int main( int argc, char **argv) {
	
	system("mkdir serData");

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
		pthread_create(&tid, NULL, &service, (void *)cliInfo);
	}

	return 0;
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
				if(member[i].valid == 1) {
					char aaa[MAXLINE];
					sprintf(aaa, "%s %d\n", member[i].name, member[i].filecount);

					strcat(msg, aaa);
					int j;
					if(member[i].filecount) {
						for(j = 0; j < member[i].filecount; j++) {
							sprintf(aaa, "%s\n", member[i].files[j]);
							strcat(msg, aaa);
						}
					}
				}
			}
			pthread_mutex_unlock(&memberLock);
			write(clisock.clifd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "infos")) {
			puts("list all users' info");
			bzero(msg, sizeof(msg));
			strcpy(msg, "");

			pthread_mutex_lock(&memberLock);
			int i;
			char aaa[MAXLINE];
			for(i = 0; i < 1024; i++) {
				if(member[i].valid == 1) {
					
					sprintf(aaa, "%s %d\n", member[i].name, member[i].filecount);
					strcat(msg, aaa);
					sprintf(aaa, "%s %d\n", inet_ntoa(member[i].socketaddr.sin_addr), member[i].port);
					strcat(msg, aaa);
					int j;
					if(member[i].filecount) {
						for(j = 0; j < member[i].filecount; j++) {
							sprintf(aaa, "%s\n", member[i].files[j]);
							strcat(msg, aaa);
						}
					}
				}
			}
			pthread_mutex_unlock(&memberLock);
			listServerFile();
			sprintf(aaa, "%s %d\n", "server", serfileCount);
			strcat(msg, aaa);
			sprintf(aaa, "%s %d\n", "0.0.0.0", 0);
			strcat(msg, aaa);
			for(i = 0; i < serfileCount; i++) {
				sprintf(aaa, "%s\n", serfile[i]);
				strcat(msg, aaa);
			}

			write(clisock.clifd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "serf")) {
			puts("list all files at server");
			bzero(msg, sizeof(msg));
			strcpy(msg, "");
			strcat(msg, "server:\n");
			
			listServerFile();
			
			int i;
			for(i = 0; i < serfileCount; i++) {
				strcat(msg, serfile[i]);
				strcat(msg, "\n");
			}
			
			if(serfileCount == 0) {
				strcat(msg, "no file\n");
			}
			
			write(clisock.clifd ,msg, strlen(msg));
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "getf")) {
			char buf[MAXLINE];
			bzero(buf, sizeof(buf));
			read(clisock.clifd, buf, MAXLINE);
			int all, part;
			char fname[40];
			sscanf(buf, "%d%d", &part, &all);
			bzero(buf, sizeof(buf));
			read(clisock.clifd, buf, MAXLINE);
			sprintf(fname, "%s", buf);
			pthread_mutex_lock(&fileLock);
			char dd[80];
			sprintf(dd, "%s/%s", "serData", fname);
			file = fopen(dd, "r");
			fseek(file, 0, SEEK_END); // seek to end of file
			int size = ftell(file); // get current file pointer
			int thispart = size/all;
			sprintf(buf, "%d", thispart);
			write(clisock.clifd, buf, strlen(buf));

			char send[thispart];
			fseek(file, thispart*part, SEEK_SET);
			fread(send, 1, thispart, file);
			write(clisock.clifd, send, strlen(send));
			fclose(file);
			pthread_mutex_unlock(&fileLock);
			sprintf(buf, "%d", thispart*part);
			write(clisock.clifd, buf, strlen(buf));
			close(clisock.clifd);
			printf("Done getf\n");
		}
		if(!strcmp(msg, "gets")) {
			read(clisock.clifd, buf, MAXLINE);
			char dd[80];
			sprintf(dd, "%s/%s", "serData", buf);
			pthread_mutex_lock(&fileLock);
			file = fopen(dd, "r");
			fseek(file, 0, SEEK_END); // seek to end of file
			int size = ftell(file); // get current file pointer
			fclose(file);
			pthread_mutex_unlock(&fileLock);
			sprintf(buf, "%d", size);
			write(clisock.clifd, buf, strlen(buf));
			close(clisock.clifd);
			printf("Done gets\n");
		}
	}
}


void listServerFile() {
	pthread_mutex_lock(&fileLock);
	system("ls -1 serData/ | wc -l >serlist");
	file = fopen("serlist", "r");
	
	if(file != NULL) {
		fscanf(file, "%d", &serfileCount);
		fclose(file);
	}
	system("ls serData/ >serlist");
	file = fopen("serlist", "r");
	if(file != NULL) {
		int i;
		for(i = 0; i < serfileCount; i++) {
			fscanf(file, "%s", serfile[i]);
		}
		fclose(file);
	}
	pthread_mutex_unlock(&fileLock);
}
