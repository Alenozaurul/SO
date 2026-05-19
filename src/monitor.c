#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

static volatile char *pth = NULL;
static volatile int pfd = -1;

struct stat st = {0};

void sigintHandler(int sig) {
	if(pth != NULL)
		remove((const char *)pth);
	if(pfd > 0)
		close(pfd);
	
	printf("Monitor is closed\n");
	exit(EXIT_SUCCESS);	
}

void sigusrHandler(int sig) {
	printf("A report has been added\n");
}

int main(int argc, char **argv) {

	struct sigaction sa = {0};

	sa.sa_handler = sigintHandler;
	if(sigaction(SIGINT, &sa, NULL) == -1) {
		perror("Error on SIGINT\n");
		exit(EXIT_FAILURE);
	}

	sa.sa_handler = sigusrHandler;
	if(sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("Error on SIGUSR1\n");
		exit(EXIT_FAILURE);
	}

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
		
		pth = pid_path;
		pfd = fd;
		fchmod(fd, 0744);
	}

	write(fd, &pid, sizeof(pid_t));
	printf("Monitor created\n");

	while(1) {	
		sleep(1);
	}



	remove(pid_path);
	close(fd);
	
	printf("Monitor is closed");	
	
	exit(EXIT_SUCCESS);
}
