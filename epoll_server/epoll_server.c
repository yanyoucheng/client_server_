/*
 * epoll_server.c
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
#include <sys/epoll.h>
#define MAX_FD_SIZE 1024
#define CLIENT_NUM 10
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

	int epfd = epoll_create(MAX_FD_SIZE);
	struct epoll_event ev, ep[MAX_FD_SIZE];
	ev.data.fd = sock; ev.events = EPOLLIN|EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
	while(1)
	{

		int ready = epoll_wait(epfd, ep, MAX_FD_SIZE,-1);
		for(int i = 0; i < ready; i++)
		{
			if(ep[i].data.fd == sock)
			{
				int conn = accept(sock ,NULL ,0);
				if(conn > 0)
				{
					ev.data.fd = conn;
					ev.events = EPOLLIN;
					epoll_ctl(epfd, EPOLL_CTL_ADD, conn, &ev);
				}
			}
			else
			{
				int conn = ep[i].data.fd;
				char buf[1024];
				int numbytes;
				printf("-------------recv client %d-----------\n", conn);
				numbytes = recv(conn, buf, sizeof(buf), 0);
				if(numbytes < 0)
					perror("recv");
				else
					printf("%s", buf);
				printf("-------------send info to client %d----------\n", conn);
				bzero(buf, sizeof(buf));
				scanf("%s", buf);
				numbytes = send(conn, buf, sizeof(buf), 0);

			}

		}

	}

    return 0;
}
