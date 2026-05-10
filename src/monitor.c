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


int main(int argc, char **argv) {
	char *path = "../.monitor.pid";

	pid_t pid = getpid();
	printf("%d\n", pid);

	int file;
	if(stat(path, &st) == 0) {
		if((file = open(path, O_CREAT | O_RDWR, 0744)) < 0) {
			printf("error file\n");
			return 1;
		}
	} else {
		if((file = open(path, O_CREAT | O_RDWR, 0744)) < 0) {
			printf("error file\n");
			return 1;
		}

		fchmod(file, 0744);
	}

	char id[10];
	sprintf(id, "%d", getpid());
	write(file, id, strlen(id));

	signal(SIGINT, sigHandle);

	while(run) {

	}	
	remove(path);
	close(file);
	
	return 0;
}
