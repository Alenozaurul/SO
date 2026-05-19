#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define n 32

typedef struct Inspector {
	char name[32];
	int score;
} Inspector;

typedef struct Report {
	int id;
	int level;
	float x, y;
	time_t t;
	char username[32];
	char category[32];
	char description[128];
} Report;

int main(int argc, char *argv[]) {
	if(argc != 2) {
		exit(EXIT_FAILURE);
	}

	char *district = argv[1];
	int fd;

	if( (fd = open(district, O_RDONLY)) == -1 ) {
		perror("Cannot open report symlink\n");
		exit(EXIT_FAILURE);
	}

	Report report;
	Report arr[n] = {0};
	long cursor = -1;

	while(read(fd, &report, sizeof(Report)) == sizeof(Report)) {
		int i;
		int is = 0;
		
		cursor = lseek(fd, 0, SEEK_CUR);

		for(i = 0; i < n && arr[i].t != 0; ++i) {
			if(!strcmp(arr[i].username, report.username)){
				is = 1;
				break;
			}
		}

		if(!is) {
			arr[i] = report;
			int sum = report.level;

			while(read(fd, &report, sizeof(Report)) == sizeof(Report)) {
				if(!strcmp(report.username, arr[i].username)) {
					sum += report.level;
				}
			}

			printf("Inspector: %s  Score: %d\n", arr[i].username, sum);
		} 
		
		lseek(fd, cursor, SEEK_SET);
	}

	close(fd);
	exit(EXIT_SUCCESS);
}
