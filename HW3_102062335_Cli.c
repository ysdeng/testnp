#include "HW3_102062335.h"
/*
get finished
put TODO QQ
*/
int port;

typedef struct {
	int connfd;
	char name[30];
	char ip[100];
	int port;
	char files[200][40];
	int filecount;
} Member;
Member self;

typedef struct {
	int connfd;
	int all;
	int part;
	char name[40];
}sendinfo;

FILE *file;
pthread_mutex_t fileLock = PTHREAD_MUTEX_INITIALIZER;
char mmmmmax[MAXLINE];
void help();
void updatef(int sockfd);
void *tcpListener();
void *doSomething(void *arg);
void *recf( void * arg);
char user[40];
int filefinished[200];

char serIP[40];
int serPort;

int fileadd;

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

	sprintf(serIP, "%s", argv[1]);
	serPort = atoi(argv[2]);

	char msg[MAXLINE];
	fileadd = 0;
	while(1) {
		if(fileadd) {
			updatef(sockfd);
			fileadd = 0;
		}
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
		if(!strcmp(msg, "listf")) {
			//send cmd to server
			write(sockfd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
			read(sockfd, msg, MAXLINE);
			printf("%s", msg);
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "infos")) {
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
		if(!strcmp(msg, "connect")) {
			char ip[40];
			scanf("%s", ip);
			struct sockaddr_in dstaddr;
			int dstfd;

			dstfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&dstaddr, sizeof(dstaddr));
			dstaddr.sin_port = htons(1500);
			dstaddr.sin_family = AF_INET;
			dstaddr.sin_addr.s_addr = inet_addr(ip);
			connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));

			read(dstfd, msg, MAXLINE);
			puts(msg);
			bzero(msg, sizeof(msg));
		}
		if(!strcmp(msg, "get")) {
			char filename[40];
			bzero(msg, sizeof(msg));
			// get file list
			write(sockfd, "infos", strlen("infos"));
			read(sockfd, msg, MAXLINE);
			//write into a real file
			FILE *flist;
			flist = fopen("list", "w");
			if(flist != NULL) {
				fprintf(flist, "%s", msg);
				fclose(flist);
			}
			//read back into mem struct
			Member member[100];
			flist = fopen("list", "r");
			int idx = 0;
			if(flist != NULL) {
				while(~fscanf(flist, "%s%d", member[idx].name, &member[idx].filecount)) {
					fscanf(flist, "%s%d", member[idx].ip, &member[idx].port);
					if(!strcmp(member[idx].name, "server")) {
						sprintf(member[idx].ip, "%s", serIP);
						member[idx].port = serPort;
					}
					int i;
					for(i = 0; i < member[idx].filecount; i++)
						fscanf(flist, "%s", member[idx].files[i]);
					if(member[idx].filecount)
						idx++;
				}
				fclose(flist);
			}
			
			//print out all infos to check correct or not
			int j, k;
			for(j = 0; j < idx; j++) {
				printf("%s %d\n", member[j].name, member[j].filecount);
				printf("%s %d\n", member[j].ip, member[j].port);
				for(k = 0; k < member[j].filecount; k++)
					printf("%s\n", member[j].files[k]);
			}

			//which user want...
			printf("you want>> ");
			scanf("%s", filename);

			Member tmp[100];
			int count = 0;
			for(j = 0; j < idx; j++) {
				for(k = 0; k < member[j].filecount; k++) {
					if(!strcmp(filename, member[j].files[k])) {
						printf("%s %d\n", member[j].name, member[j].filecount);
						printf("%s %d\n", member[j].ip, member[j].port);
						printf("%s\n", member[j].files[k]);
						sprintf(tmp[count].name, "%s", member[j].name);
						sprintf(tmp[count].ip, "%s", member[j].ip);
						tmp[count].port = member[j].port;
						count++;
					}
				}
			}
			if(count == 0) {
				printf("no this file\n");
				continue;
			}
			pthread_t aa;

			struct sockaddr_in dstaddr;
			int dstfd;
			dstfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&dstaddr, sizeof(dstaddr));
			if(!strcmp(tmp[0].name, "server"))
				dstaddr.sin_port = htons(tmp[0].port);
			else
				dstaddr.sin_port = htons(1500);
			dstaddr.sin_family = AF_INET;
			dstaddr.sin_addr.s_addr = inet_addr(tmp[0].ip);
			connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));
			write(dstfd, "gets", strlen("gets"));
			sleep(1);
			write(dstfd, filename, strlen(filename));
			bzero(msg, sizeof(msg));
			read(dstfd, msg, MAXLINE);
			int size = atoi(msg);
			int from, end;
			from = 0, end = 0;
			bzero(mmmmmax, sizeof(mmmmmax));
			for(j = 0; j < count; j++) {

				from = end;
				end = from+size/count+1;
				if(end>size) end = size;

				struct sockaddr_in dstaddr;
				int dstfd;

				dstfd = socket(AF_INET, SOCK_STREAM, 0);
				bzero(&dstaddr, sizeof(dstaddr));

				if(!strcmp(tmp[j].name, "server"))
					dstaddr.sin_port = htons(tmp[j].port);
				else
					dstaddr.sin_port = htons(1500);

				dstaddr.sin_family = AF_INET;
				dstaddr.sin_addr.s_addr = inet_addr(tmp[j].ip);
				connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));

				printf("%s\n", tmp[j].name);
				write(dstfd, "getf", strlen("getf"));
				sleep(1);
				sprintf(msg, "%d %d", from, end);
				write(dstfd, msg, strlen(msg));

				sendinfo *d;
		        d = malloc(sizeof(sendinfo));
		        d->connfd = dstfd;
		        d->all = count;
		        d->part = j;
		        sprintf(d->name, "%s", filename);
				pthread_create(&(aa), NULL, &recf, d);
			}
			printf("wait....\n");
			sleep(5);
			updatef(sockfd);
			printf("Done....\n");
		}
		if(!strcmp(msg, "put")) {
			char filename[40];
			bzero(msg, sizeof(msg));
			// get file list
			write(sockfd, "infos", strlen("infos"));
			read(sockfd, msg, MAXLINE);
			//write into a real file
			FILE *flist;
			flist = fopen("list", "w");
			if(flist != NULL) {
				fprintf(flist, "%s", msg);
				fclose(flist);
			}
			//read back into mem struct
			Member member[100];
			flist = fopen("list", "r");
			int idx = 0;
			if(flist != NULL) {
				while(~fscanf(flist, "%s%d", member[idx].name, &member[idx].filecount)) {
					fscanf(flist, "%s%d", member[idx].ip, &member[idx].port);
					if(!strcmp(member[idx].name, "server")) {
						sprintf(member[idx].ip, "%s", serIP);
						member[idx].port = serPort;
					}
					int i;
					for(i = 0; i < member[idx].filecount; i++)
						fscanf(flist, "%s", member[idx].files[i]);
					if(member[idx].filecount)
						idx++;
				}
				fclose(flist);
			}
			
			//print out all infos to check correct or not
			int j, k;
			for(j = 0; j < idx; j++) {
				printf("%s %d\n", member[j].name, member[j].filecount);
				printf("%s %d\n", member[j].ip, member[j].port);
				for(k = 0; k < member[j].filecount; k++)
					printf("%s\n", member[j].files[k]);
			}

			//which user want...
			printf("which u put>> ");
			scanf("%s", filename);

			Member tmp[100];
			int count = 0;
			int flag = 0;
			for(j = 0; j < idx; j++) {
				for(k = 0; k < member[j].filecount; k++) {
					if(!strcmp(filename, member[j].files[k])) {
						printf("%s %d\n", member[j].name, member[j].filecount);
						printf("%s %d\n", member[j].ip, member[j].port);
						printf("%s\n", member[j].files[k]);
						sprintf(tmp[count].name, "%s", member[j].name);
						sprintf(tmp[count].ip, "%s", member[j].ip);
						tmp[count].port = member[j].port;
						
						if(!strcmp(tmp[count].name, user)) flag = 1;

						count++;
					}
				}
			}
			if(flag == 0) {
				printf("u don't have this file\n");
				continue;
			}

			//send to who
			char who[40];
			char rip[40];
			int rport;

			printf("send to [usr/ser] >> ");
			scanf("%s", who);
			if(!strcmp(who, "ser")) {
				sprintf(rip, "%s", serIP);
				rport = serPort;
			}
			else if(!strcmp(who, "usr")){
				write(sockfd, "listu", strlen("listu"));
				bzero(msg, sizeof(msg));
				read(sockfd, msg, MAXLINE);
				printf("%s", msg);
				bzero(msg, sizeof(msg));
				printf("reciever ip>> ");
				scanf("%s", rip);
				//printf("reciever port>> ");
				//scanf("%d", &rport);
				rport = 1500;
			}
			else {
				printf("wrong reciever.\n");
				continue;
			}

			pthread_mutex_lock(&fileLock);
			char dd[80];
			sprintf(dd, "%s/%s", user, filename);
			file = fopen(dd, "r");
			fseek(file, 0, SEEK_END); 
			int size = ftell(file);
			fclose(file);
			pthread_mutex_unlock(&fileLock);
			printf("file size: %d\n", size);

			if(count == 1) {
				printf("send myself\n");

				struct sockaddr_in dstaddr;
				int dstfd;

				dstfd = socket(AF_INET, SOCK_STREAM, 0);
				bzero(&dstaddr, sizeof(dstaddr));

				dstaddr.sin_port = htons(rport);

				dstaddr.sin_family = AF_INET;
				dstaddr.sin_addr.s_addr = inet_addr(rip);
				connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));

				write(dstfd, "recv", strlen("recv"));
				sleep(1);
				write(dstfd, filename, strlen(filename));
				sleep(1);
				sprintf(msg, "%d %d", 0, size);
				write(dstfd, msg, strlen(msg));
				sleep(1);
				char dd[80];
				sprintf(dd, "%s/%s", user, filename);
				pthread_mutex_lock(&fileLock);
				file = fopen(dd, "r");
				char bufff[MAXLINE];
				fread(bufff, 1, MAXLINE, file);
				fclose(file);
				pthread_mutex_unlock(&fileLock);
				write(dstfd, bufff, strlen(bufff));
				printf("Done self send\n");
			}
			else {
				int selfpart;

				int from, end;
				int self_from, self_end;
				from = end = 0;

				for(i = 0; i < count; i++) {

					from = end;
					end = from+size/count+1;
					if(end>size) end = size;

					if(!strcmp(tmp[i].name, user)) {
						selfpart = i;
						self_from = from;
						self_end = end;
						continue;
					}
					struct sockaddr_in dstaddr;
					int dstfd;

					dstfd = socket(AF_INET, SOCK_STREAM, 0);
					bzero(&dstaddr, sizeof(dstaddr));
					if(!strcmp(tmp[i].name, "server"))
						dstaddr.sin_port = htons(serPort);
					else
						dstaddr.sin_port = htons(1500);
					dstaddr.sin_family = AF_INET;
					if(!strcmp(tmp[i].name, "server"))
						dstaddr.sin_addr.s_addr = inet_addr(serIP);
					else
						dstaddr.sin_addr.s_addr = inet_addr(tmp[i].ip);
					connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));
					write(dstfd, "sendf", strlen("sendf"));
					sleep(1);
					//send dst infos[ip, port] | filename
					sprintf(msg, "%s %d %s %d %d", rip, rport, filename, from, end);
					write(dstfd, msg, strlen(msg));
					sleep(1);
				}
				
				printf("i send from %d to %d\n", self_from, self_end);
				struct sockaddr_in dstaddr;
				int dstfd;

				dstfd = socket(AF_INET, SOCK_STREAM, 0);
				bzero(&dstaddr, sizeof(dstaddr));

				dstaddr.sin_port = htons(rport);

				dstaddr.sin_family = AF_INET;
				dstaddr.sin_addr.s_addr = inet_addr(rip);
				connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));

				write(dstfd, "recv", strlen("recv"));
				sleep(1);
				write(dstfd, filename, strlen(filename));
				sleep(1);
				sprintf(msg, "%d", self_from);
				write(dstfd, msg, strlen(msg));
				sleep(1);
				char dd[80];
				char msgff[MAXLINE];
				
				sprintf(dd, "%s/%s", user, filename);
				pthread_mutex_lock(&fileLock);

				file = fopen(dd, "r");
				fseek(file, self_from, SEEK_SET);
				int ss = self_end - self_from + 1;
				bzero(msg, sizeof(msg));
				char send[MAXLINE];
				fread(send, 1, ss, file);
				fclose(file);
				pthread_mutex_unlock(&fileLock);
				write(dstfd, send, strlen(send));
				printf("%s\n", send);
				printf("Done self send\n");
				sleep(3);
				write(dstfd, "c", strlen("c"));
			}
			sleep(5);
			puts("Done put...");
			/*
			struct sockaddr_in dstaddr;
			int dstfd;

			dstfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&dstaddr, sizeof(dstaddr));

			dstaddr.sin_port = htons(rport);

			dstaddr.sin_family = AF_INET;
			dstaddr.sin_addr.s_addr = inet_addr(rip);
			connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));
			write(dstfd, "c", strlen("c"));
			*/
			
		}
		if(!strcmp(msg, "serf")) {
			write(sockfd, msg, strlen(msg));
			bzero(msg, sizeof(msg));
			read(sockfd, msg, MAXLINE);
			printf("%s", msg);
			bzero(msg, sizeof(msg));
		}
	}
	return 0;
}

void *recf( void * arg) {
	sendinfo info = *((sendinfo*) arg);
	free(arg);
	char buf[MAXLINE];

	sleep(1);
	write(info.connfd, info.name, strlen(info.name));

	read(info.connfd, buf, MAXLINE);
	
	int start, end;
	sscanf(buf, "%d", &start);
	printf("rcver: start %d", start);

	char buff[MAXLINE];
	bzero(buff, sizeof(buff));
	read(info.connfd, buff, sizeof(buff));
	puts(buff);
	
	bzero(buf, sizeof(buf));
	
	pthread_mutex_lock(&fileLock);
	char dd[80];
	sprintf(dd, "%s/%s", user, info.name);
	file = fopen(dd, "w");
	fseek(file, start, SEEK_SET);
	fwrite(buff, 1, strlen(buff), file);
	//fclose(file);
	pthread_mutex_unlock(&fileLock);

	puts("recf Done...");
	return NULL;
}

void *doSomething(void *arg) {
	int connfd = *((int*) arg);
	free(arg);
	char buf[MAXLINE];

	while(1) {
		bzero(buf, sizeof(buf));
		read(connfd, buf, MAXLINE);
		if(!strcmp(buf, "gets")) {
			bzero(buf, sizeof(buf));
			read(connfd, buf, MAXLINE);			
			char dd[80];
			sprintf(dd, "%s/%s", user, buf);

			pthread_mutex_lock(&fileLock);
			file = fopen(dd, "r");
			fseek(file, 0, SEEK_END); // seek to end of file
			int size = ftell(file); // get current file pointer
			fclose(file);
			pthread_mutex_unlock(&fileLock);
			sprintf(buf, "%d", size);
			printf("%s\n", buf);
			write(connfd, buf, strlen(buf));
			close(connfd);
			printf("Done gets\n");
			return NULL;
		}
		if(!strcmp(buf, "getf")) {
			char buf[MAXLINE];
			bzero(buf, sizeof(buf));
			read(connfd, buf, MAXLINE);
			int end, from;
			char fname[40];
			sscanf(buf, "%d%d", &from, &end);
			printf("send %d/%d\n", from, end);

			sprintf(buf, "%d", from);
			write(connfd, buf, strlen(buf));
			bzero(buf, sizeof(buf));
			
			read(connfd, buf, MAXLINE);
			sprintf(fname, "%s", buf);

			pthread_mutex_lock(&fileLock);
			char dd[80];
			sprintf(dd, "%s/%s", "serData", fname);
			file = fopen(dd, "r");

			sleep(1);
			char send[MAXLINE];
			fseek(file, from, SEEK_SET);
			int ss = end - from +1;
			fread(send, 1, ss, file);
			write(connfd, send, strlen(send));
			fclose(file);
			pthread_mutex_unlock(&fileLock);
			close(connfd);
			printf("Done getf\n");
			return NULL;
		}
		if(!strcmp(buf, "sendf")) {
			// i need know dst's ip, port
			// i need know filename
			puts("sendf");
			bzero(buf, sizeof(buf));
			char dip[40];
			int dport;
			char filename[40];
			int from, end;
			read(connfd, buf, MAXLINE);
			sscanf(buf, "%s%d%s%d%d", dip, &dport, filename, &from, &end);
			printf("to %s %d, file: %s\n", dip, dport, filename);
			
			bzero(buf, sizeof(buf));
			printf("sendf: from %d to %d\n", from, end);

			struct sockaddr_in dstaddr;
			int dstfd;

			dstfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&dstaddr, sizeof(dstaddr));

			dstaddr.sin_port = htons(dport);

			dstaddr.sin_family = AF_INET;
			dstaddr.sin_addr.s_addr = inet_addr(dip);
			connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));

			write(dstfd, "recv", strlen("recv"));
			sleep(1);
			write(dstfd, filename, strlen(filename));
			sleep(1);
			sprintf(buf, "%d", from);
			write(dstfd, buf, strlen(buf));
			sleep(1);
			char dd[80];
			
			sprintf(dd, "%s/%s", user, filename);
			pthread_mutex_lock(&fileLock);
			char bufff[MAXLINE];
			file = fopen(dd, "r");
			fseek(file, from, SEEK_SET);
			int ss = end-from+1;
			fread(bufff, 1, ss, file);
			fclose(file);
			pthread_mutex_unlock(&fileLock);
			write(dstfd, bufff, strlen(bufff));
			puts("Done sendf");
			return NULL;
		}
		if(!strcmp(buf, "recv")) {
			bzero(buf, sizeof(buf));
			read(connfd, buf, MAXLINE);
			char dd[80];
			sprintf(dd, "%s/%s", user, buf);
			bzero(buf, sizeof(buf));
			int start, end;
			read(connfd, buf, MAXLINE);
			puts(buf);
			sscanf(buf, "%d", &start);
			bzero(buf, sizeof(buf));
			char aa[MAXLINE];
			read(connfd, aa, MAXLINE);

			pthread_mutex_lock(&fileLock);

			file = fopen(dd, "w");
			fseek(file, start, SEEK_SET);
			//int ss = end-start+1;
			fwrite(aa, 1, strlen(aa), file);
			//fclose(file);
			pthread_mutex_unlock(&fileLock);
			close(connfd);
			printf("Done recv\n");
			return NULL;
		}
		if(!strcmp(buf, "c")) {
			pthread_mutex_lock(&fileLock);
			fclose(file);
			pthread_mutex_unlock(&fileLock);

			struct sockaddr_in dstaddr;
			int dstfd;
			dstfd = socket(AF_INET, SOCK_STREAM, 0);
			bzero(&dstaddr, sizeof(dstaddr));

			dstaddr.sin_port = htons(serPort);

			dstaddr.sin_family = AF_INET;
			dstaddr.sin_addr.s_addr = inet_addr(serIP);
			connect(dstfd, (SA*)&dstaddr, sizeof(dstaddr));
			updatef(dstfd);
			return NULL;
		}
	}
}

void help () {
	printf("we provide following service\n");
	printf("login: login with a name\n");
	printf("logout: terminal the service\n");
	printf("listu: list online user's infos\n");
	printf("listf: list online user's files\n");
	printf("updatef: update local files\n");
	printf("connect [ip] [port]: connect to other user\n");
	printf("get [file]: get [file] from others\n");
	//printf("put [file] [usr/ser]: put [file] to [usr/ser]\n");
	printf("infos: get all users' infos\n");
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
	sleep(1);
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
			sleep(1);
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
	pthread_t tid;
	int c = 0;
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
        int *confd;
        confd = malloc(sizeof(int));
        *confd = connfd;
        pthread_create(&(tid), NULL, &doSomething, (void *)confd);
	}
}