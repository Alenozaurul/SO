#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

struct stat st = {0};

int main(int argc, char **argv) {
	char command[32];

	while(strcmp(command, "exit")) {
		printf("Command:");
		if(scanf("%s", command) == EOF) 
			break;

		if(!strcmp(command, "exit")) {
			continue;
		} else if(!strcmp(command, "start_monitor")) {
			pid_t pid;
			if((pid = fork()) < 0 ) {
		       		perror("Fork error on start_monitor\n");
	 	      		exit(EXIT_FAILURE);
			}	

			if(pid == 0) {
				execl("./hub_mon", "hub_mon", NULL);
				perror("Execl failed\n");
				exit(EXIT_FAILURE);
			}

			sleep(1);
		} else if(!strcmp(command, "calculate_scores")) {
			char *district = NULL;
			char buffer[1024];

			getchar();
			printf("Districts: ");
			if(fgets(buffer, sizeof(buffer), stdin) != NULL){
				district = strtok(buffer, " ");
				int end = 0;

				while(district != NULL){ 
					printf("%s\n", district);

					if(strchr(district, '\n')) {
						district[strlen(district) - 1] = '\0';
						end = 1;
					}
					char link[64] = "active_reports-";
					strcat(link, district);
					
					district = strtok(NULL, " ");

					if(stat(link, &st) < 0) {
						printf("District not found\n");
						//perror("Cannot find district\n");
						continue;
					}

					pid_t pid;
					int pfd[2];
			
					if(pipe(pfd) < 0){
						perror("Pipe error on reading a district\n");
						exit(EXIT_FAILURE);
					}
			
					if( (pid = fork()) < 0 ) {
						perror("Fork error in calculate_scores\n");
						exit(EXIT_FAILURE);
					}

					if(pid == 0) {
						close(pfd[0]);
						dup2(pfd[1], STDOUT_FILENO);
						close(pfd[1]);
				
						execl("./scorer", "scorer", link, NULL);
						
						perror("Execl failed on calculate_scores\n");
						exit(1);
					} else {
						wait(NULL);
						close(pfd[1]);

						char buffer[1024];
						ssize_t bytes = read(pfd[0], buffer, sizeof(buffer) - 1);
						if(bytes > 0) { 
							buffer[bytes] = '\0';
							printf("%s", buffer);
						} else if(bytes == -1) {
							perror("Couldn't read from scorer");
							break;
						} else if(bytes == 0) {
							break;
						}

						close(pfd[0]);
					}
					if(end)
						break;	
				}
			} 
		} else {
			printf("Commands are: start_monitor, calculate_scores, exit\n");
		}
	}

	exit(EXIT_SUCCESS);
}
