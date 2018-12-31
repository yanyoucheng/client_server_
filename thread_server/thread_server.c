/*
 * thread_server.c
 *
 *  Created on: Dec 30, 2018
 *      Author: root
 */

#include <arpa/inet.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define CLIENT_NUM 10
static void set_nonblock(int* sockfd)
{
	int flag = 1;
	setsockopt(*sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)); //no dely send
    int  opts;
	opts = fcntl(*sockfd,F_GETFL);
	if (opts < 0 )
	{
		perror( " fcntl(sock,GETFL) " );
	}
	opts  =  opts | O_NONBLOCK;
	if (fcntl(*sockfd,F_SETFL,opts) < 0 )
	{
		perror( " fcntl(sock,SETFL,opts) " );
	 }
}
static void *deal_conn(void *arg)
{
	int conn = (int)arg; // only int and long can conver to void*.
	int flag = 1;
	setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)); //no dely send, adapt small data
	//set_nonblock(&conn);
	char buf[1024];
	int numbytes;
	while(1)
	{
		printf("----------recv client %d info-------\n", conn);
		bzero(buf, sizeof(buf));
		if((numbytes = recv(conn, buf, sizeof(buf), 0)) > 0)
		{
			printf("%s", buf);
			bzero(buf, sizeof(buf));
		}
		printf("\n---------server send to client %d-------\n", conn);
		scanf("%s", buf);
		if((numbytes = send(conn, buf, sizeof(buf), 0)) > 0)
		{

		}
	}

}
int main()
{
	int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	int flag = 1, error = 0, client_fd[CLIENT_NUM+1], clients = -1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int));
	struct addrinfo* hints;
	getaddrinfo(NULL, "8000", NULL, &hints);
	char str[128];
	for(; hints != NULL; hints = hints->ai_next) {
		if(hints->ai_family ==  AF_INET6)
		{
			struct sockaddr_in6* addr = (struct sockaddr_in6*)hints->ai_addr;
			inet_ntop(AF_INET6, (void*)&addr->sin6_addr, str, 128);
			printf("ai_addr = %s\n", str);
			printf("ai_family = %d\n", hints->ai_family);
			printf("ai_socktype = %d\n", hints->ai_socktype);
		}
		else
		{
			struct sockaddr_in* addr = (struct sockaddr_in*)hints->ai_addr;
			printf("ai_addr = %s\n", inet_ntoa(addr->sin_addr));
			printf("ai_port = %d\n", ntohs(addr->sin_port));
			printf("ai_family = %d\n", hints->ai_family);
			printf("ai_socktype = %d\n", hints->ai_socktype);
			if(hints->ai_socktype == SOCK_STREAM)
				break;
		}
	}
	error = bind(sock, hints->ai_addr, sizeof(struct sockaddr));
	if(error < 0)
		perror("bind fail\n");
	error = listen(sock, CLIENT_NUM);
	if(error < 0)
		perror("listen fail\n");
	while(1)
	{
		int confd = accept(sock, NULL, 0);
		if(confd < 0)
		{
			sleep(3);
		}
		else
		{
			printf("%d", confd);
			pthread_t tid;
			pthread_create(&tid, NULL, deal_conn, confd);
		}
	}
	return 0;
}
