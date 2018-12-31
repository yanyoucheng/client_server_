/*
 * select_server.c
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
#include  <fcntl.h>

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
    fd_set read_set, write_set, select_set;
    char buf[1024];
    int numbytes;
    struct timeval timeout={3,0};
	while(1)
    {
		FD_ZERO(&select_set);
		FD_SET(sock, &select_set);
		error = select(sock+1, &select_set, NULL, NULL, &timeout);
		if(error > 0)
		{
			if(FD_ISSET(sock, &select_set))
			{
				clients += 1;
				client_fd[clients] = accept(sock ,NULL , 0);
				if(client_fd[clients] < 0)
					perror("accept fail\n");
				printf("accept success, %d\n", client_fd[clients]);
				int flag = 1;
				setsockopt(client_fd[clients], IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)); //no dely send
			   int  opts;
				opts = fcntl(client_fd[clients],F_GETFL);
				 if (opts < 0 )
				{
					perror( " fcntl(sock,GETFL) " );
				}
				opts  =  opts | O_NONBLOCK;
				 if (fcntl(client_fd[clients],F_SETFL,opts) < 0 )
				{
					perror( " fcntl(sock,SETFL,opts) " );
				 }													// must set sockt opt non-block, if not, it will cause select block.
			}
		}
		FD_ZERO(&read_set);
		for(int i = 0; i < clients+1; i++)
		{
			FD_SET(client_fd[i], &read_set);
		}
		error = select(client_fd[clients]+1, &read_set, NULL, NULL, 0);
		printf("------------enter recv, error=%d-----------\n", error);
		if(error > 0)
		{
			for(int i = 0; i < clients+1; i++)
			{
				if(FD_ISSET(client_fd[i], &read_set))
				{
					printf("--------recv client %d info----------\n", client_fd[i]);
					numbytes = 0;
					bzero(buf, sizeof(buf)); // must bzero buf
					while((numbytes = recv(client_fd[i], buf, sizeof(buf), 0)) > 0)
					{
						printf("%s", buf);
						bzero(buf, sizeof(buf)); // must bzero buf
					}
				}

			}
		}
		printf("\n---------enter send--------------\n");
		FD_ZERO(&write_set);
		for(int i = 0; i < clients+1; i++)
		{
			FD_SET(client_fd[i], &write_set);
		}
		error = select(client_fd[clients]+1, NULL, &write_set, NULL, 0);
		if(error > 0)
		{
			for(int i = 0; i < clients+1; i++)
			{
				if(FD_ISSET(client_fd[i], &write_set))
				{
					printf("--------send client %d info----------\n", client_fd[i]);
					bzero(buf, sizeof(buf));
					scanf("%s", buf);
					numbytes = 0;
					while((numbytes = send(client_fd[i], buf+numbytes, sizeof(buf)-numbytes, 0)) > 0);
					printf("--------send complete-------------\n");
				}

			}
		}

    }
	return 0;
}

