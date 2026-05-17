#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>


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
			char district[32];
			
			printf("District: ");
			while( (scanf("%s", district) != EOF ) && (strcmp(district, "exit") != 0) ){
				pid_t pid;
			
				if( (pid = fork()) < 0 ) {
					perror("Fork error in calculate_scores\n");
					exit(EXIT_FAILURE);
				}

				if(pid == 0) {
					execl("./calculate", "calculate", district, NULL);
					exit(1);
				} else  		
					wait(NULL);

				printf("District: ");	
			} 
		} else {
			printf("Commands are: start_monitor, calculate_scores, exit\n");
		}
	}

	exit(EXIT_SUCCESS);
}
