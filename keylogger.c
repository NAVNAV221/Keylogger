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

static void interrupt_handler(int sig)
{
	stop = 1;
}

int main(int argc, char *argv[])
{
	char *device = "/dev/input/event4";
	char dev_name[256];
	int fd = -1, status, i;
	size_t rb;
	struct input_event events[64];
	rb = read(fd, events, sizeof(struct input_event)*64);

	signal(SIGINT, interrupt_handler);

	while (!stop) {

		if(rb < (int) sizeof(struct input_event)){
			fprintf(stderr, "Error bytes reading\n");
			CLEAN(EXIT_FAILURE);
		}

		if(fd = open(device, O_RDONLY) < 0){
			fprintf(stderr, "Can't open the device\n");
			CLEAN(EXIT_FAILURE);
		}

		if(ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name) < 0){
			fprintf(stderr, "Can't figure the name of the device");
			CLEAN(EXIT_FAILURE);
		}

		for(i = 0; i < (int) (rb / sizeof(struct input_event)); i++){
			unsigned int type, code;

			type = events[i].type;
			code = events[i].code;
			
			if(type == EV_KEY){
				printf("Event: time %ld.%06ld, ", events[i].time.tv_sec, events[i].time.tv_usec);
				printf("TypeName: EV_KEY, Code: %d\n", code);
			}
		}
		ioctl(fd, EVIOCGRAB, (void*)0);
		CLEAN(EXIT_SUCCESS);
	}


cleanup:
	if(fcntl(fd, F_GETFD) != -1 || errno != EBADF)
		close(fd);

	if(events != NULL)
		memset(events, 0, sizeof(struct input_event)*64);

	return status;
}