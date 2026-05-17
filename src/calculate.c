#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

struct stat st;


int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("argc\n");
		exit(EXIT_FAILURE);
	}

	char path[128] = "../districts/";

	char cfg_path[256];
	strcat(path, argv[1]);

	if(stat(path, &st) == 0) {
		strcpy(cfg_path, path);
		strcat(cfg_path, "/district.cfg");
		
		if(stat(cfg_path, &st) == 0) {	
			//to do scores
		}
		else
			printf("Config file can't be opened");
	} else {
		printf("District %s doesn't exist\n", argv[1]);
	}

	exit(EXIT_SUCCESS);
}
