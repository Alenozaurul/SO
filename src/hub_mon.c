#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

struct stat st;


int main(int argc, char *argv[]) {
	pid_t pid;
	int pfd[2];

	if(pipe(pfd) < 0) {
		perror("Pipe error on monitor\n");
		exit(EXIT_FAILURE);
	}

	if( (pid = fork()) < 0 ) {
		perror("Fork error on starting a monitor\n");
		exit(EXIT_FAILURE);
	}

	if(pid == 0) {
		close(pfd[0]);

		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[1]);

		
		execl("./monitor", "monitor", NULL);
		perror("Execl error\n");

		exit(EXIT_FAILURE);
	}

	close(pfd[1]);

	char buffer[256];

	while(1) {
		ssize_t bytes = read(pfd[0], buffer, sizeof(buffer) - 1);
		if(bytes > 0) {		
			buffer[bytes] = '\0';
			printf("The monitor output is: \n\"%s\"\n", buffer);
		} else if(bytes == -1){
			perror("Couldn't read from monitor\n");
			exit(EXIT_FAILURE);
		} else if(bytes == 0) {
			break;
		}
	}


	close(pfd[0]);

	printf("Hub_Mon closed\n");
	exit(EXIT_SUCCESS);
}
