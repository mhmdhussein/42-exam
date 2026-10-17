/* wrong: puts "client %d: " only before the first line of a message */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct client
{
	int id;
	char *buffer;
} client;

int globalId = 0;
fd_set afd, rfd, wfd;
int sockfd;
client clients[1024];

void fatal()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void sendToAll(int fd, char *buffer)
{
	for (int i = 0; i < 1024; i++)
	{
		if (FD_ISSET(i, &wfd) && i != sockfd && i != fd)
			send(i, buffer, strlen(buffer), 0);
	}
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void setupServer(int port)
{
	struct sockaddr_in servaddr;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		fatal();
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		fatal();
	if (listen(sockfd, 10) != 0)
		fatal();
	FD_ZERO(&afd);
	FD_SET(sockfd, &afd);
}

void acceptClient()
{
	int connfd;
	socklen_t len;
	struct sockaddr_in cli;
	char header[1024];

	len = sizeof(cli);
	connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
	if (connfd < 0)
		fatal();
	FD_SET(connfd, &afd);
	clients[connfd].id = globalId++;
	clients[connfd].buffer = NULL;
	sprintf(header, "server: client %d just arrived\n", clients[connfd].id);
	sendToAll(connfd, header);
}

void readClient(int fd)
{
	char *temp;
	char buffer[1024];
	int received;

	received = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (received <= 0)
	{
		char header[100];
		sprintf(header, "server: client %d just left\n", clients[fd].id);
		if (clients[fd].buffer)
		{
			free(clients[fd].buffer);
			clients[fd].buffer = NULL;
		}
		FD_CLR(fd, &afd);
		close(fd);
		sendToAll(fd, header);
	}
	else
	{
		char header[100];
		buffer[received] = 0;
		clients[fd].buffer = str_join(clients[fd].buffer, buffer);
		sprintf(header, "client %d: ", clients[fd].id);
		int first = 1;
		while (extract_message(&(clients[fd].buffer), &temp) == 1)
		{
			if (first)
				sendToAll(fd, header);
			first = 0;
			sendToAll(fd, temp);
			if (temp)
				free(temp);
		}
	}
}

void loop()
{
	while (1)
	{
		wfd = afd;
		rfd = afd;
		if (select(1024, &rfd, &wfd, NULL, NULL) == -1)
			fatal();
		for (int i = 0; i < 1024; i++)
		{
			if (FD_ISSET(i, &rfd))
			{
				if (i == sockfd)
					acceptClient();
				else
					readClient(i);
			}
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		return (1);
	}
	setupServer(atoi(argv[1]));
	loop();
}