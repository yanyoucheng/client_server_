/*
 * client.c
 *
 *  Created on: Dec 30, 2018
 *      Author: root
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
int main()
{
	int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	int flag = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)); //no dely send
	struct addrinfo *hints = 0;
	hints = (struct addrinfo*)malloc(sizeof(*hints));
	hints->ai_socktype = SOCK_STREAM;
	getaddrinfo("127.0.0.1", "8000", NULL, &hints);
	char str[128] = {0};
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
	int conn = connect(sock, hints->ai_addr, sizeof(struct sockaddr));
	if(conn  < 0)
		perror("conn fail!\n");
	char buf[1024];
	int numbytes = 0;
	fd_set read_set;
	fd_set write_set;
	while(1)
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_SET(sock, &read_set);
		FD_SET(sock, &write_set);
		printf("--------Enter info to send--------\n");
		numbytes = 0;
		bzero(buf, sizeof(buf));
		select(sock+1, NULL, &write_set, NULL, 0);
		scanf("%s", buf);
		while((numbytes = send(sock, buf + numbytes, sizeof(buf) - numbytes, 0)) >0);
		socklen_t sendbuflen = 0;
		printf("---------Wait recv info--------\n");
		select(sock+1, &read_set, NULL, NULL, 0);
		numbytes = 0;
		bzero(buf, sizeof(buf));
		while((numbytes = recv(sock, buf, sizeof(buf), 0)) > 0)
		{
			printf("%s",buf);
			bzero(buf, sizeof(buf));
		}
	}
	return 0;
}
