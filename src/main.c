#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

struct stat st = {0};

int openFile( char name[], char path[]) {
	char file_name[256];
	strcpy(file_name, path);
	strcat(file_name, name);

	int id;

	if(stat(file_name, &st) == 0) {
		if((id = open(file_name, O_CREAT | O_RDWR | O_APPEND, 0664)) == -1 ){
			printf("Error opening report file %s\n", name);
			return 3;
		}
	} else {
		if((id = open(file_name, O_CREAT | O_RDWR | O_APPEND, 0664)) == -1 ){
			printf("Error creating report file %s\n", name);
			return 3;
		}
	
		if(strstr(file_name, "district")) {
			chmod(file_name, 0640);
		} else {		
			chmod(file_name, 0664);
		}
	}

	return id;
}

char *checkARG(int argc, char argv[]) {
	if(argc != 5) {
		printf("Not enough arguments(5)\n");
		return NULL;
	}

	return argv;
}

void makeDir(char district[], char path[]) {
	strcpy(path, "../districts/");
       	strcat(path, district);
	strcat(path, "/");	

	if(mkdir(path, 0750) == 0) {
		printf("Creating district directory..\n");
		chmod(path, 0750);
	}
}
	

char *checkRole(int argc, char argv[]) {
	if(!strcmp(argv, "inspector"))
		return argv;
	else if(!strcmp(argv, "manager"))
		return argv;
	
	return NULL;
}

int main(int argc, char **argv) {
	char *role = checkRole(argc, argv[2]);
	char *district = checkARG(argc, argv[4]);
	if(!district || !role)
		return 1;

	char path[100];
	makeDir(district, path);

	int log_file = openFile("log", path);
	int report_file = openFile("reports.dat", path);
        int district_file = openFile("district.cfg", path);

	printf("%s %s\n%i %i %i\n", district, role, log_file, report_file, district_file);


	return 0;
}
