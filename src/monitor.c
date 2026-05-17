#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

static volatile int run = 1;
struct stat st = {0};

void sigHandle() {
	printf("\nClosing monitor...\n");
	run = 0;
}

void sigusr(int sig) {
	if(sig == SIGUSR1) 
		printf("received sigusr1\n");
}

int main(int argc, char **argv) {
	int fd;
	pid_t pid = getpid();
	char *pid_path = "../.monitor_pid";

	setvbuf(stdout, NULL, _IONBF, 0);

	if(stat(pid_path, &st) == 0) {
		printf("An monitor already exists\nMonitor closed");
		exit(EXIT_FAILURE);
	} else {
		if((fd = open(pid_path, O_CREAT | O_RDWR, 0744)) < 0) {
			printf("error file\n");
			return 1;
		}

		fchmod(fd, 0744);
	}

	write(fd, &pid, sizeof(pid_t));
	printf("Monitor created");

	while(1) {	
		;
	}

	remove(pid_path);
	close(fd);
	
	printf("Monitor is closed");	
	
	exit(EXIT_SUCCESS);
}
