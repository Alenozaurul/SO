#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define nrc 7

struct stat st = {0};

typedef struct Report {
	int id;
	char user[20];
	float x, y;
	char category[16];
	int level;  //1  2  3
	time_t timestamp;
	char description[100];
} Report;

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
	
		if(!strstr(file_name, "report")) {
			chmod(file_name, 0640);
		} else {		
			chmod(file_name, 0664);
		}
	}

	return id;
}

void makeDir(char district[], char path[]) {
	strcpy(path, "../districts/");
       	strcat(path, district);
	strcat(path, "/");	

	if(mkdir(path, 0750) == 0) {
		printf("Created district directory..\n");
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

void addDistrict(int report_file, char user[]) {
	Report report;

	printf("Creating report: \n");
	printf("X: ");
	scanf("%f", &report.x);
	printf("Y: ");
	scanf("%f", &report.y);
	printf("Category: ");
	scanf("%15s", report.category);
	printf("Severity level: ");
	scanf("%i", &report.level);
	printf("Description: ");
	getchar();
	fgets(report.description, sizeof(report.description), stdin);

	report.id = 1;
	strcpy(report.user, user);
	report.timestamp = time(NULL);

	write(report_file, &report, sizeof(report));
}

void view(){;}
void removeDistrict(){;}
void updateThreshold(){;}
void filter(){;}

void list(int report_file) {
	fstat(report_file, &st);
	char out[10];

	out[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
	out[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
	out[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';

	out[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
	out[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
	out[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
	
	out[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
	out[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
	out[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';

	out[9] = '\0';

	printf("Permissions: %s  Size: %ld  Last modified: %s", out, st.st_size, ctime(&st.st_mtime));

	Report report;
	while(read(report_file, &report, sizeof(report)) == sizeof(Report)) {
		printf("%i\n", report.id);
		printf("%s\n", report.user);
		printf("%f %f\n", report.x, report.y);
		printf("%s\n", report.category);
		printf("%i\n", report.level);
		printf("%s", ctime(&report.timestamp));
		printf("%s\n\n", report.description);
	}
}

int checkFilePermission(int id, char role[], int r_id) {
	if(fstat(id, &st) != 0) {
		perror("fstat");
		return 1;
	}

	if(!strcmp(role, "manager")) {
		if(!(st.st_mode & S_IRUSR)) 
			return 1;
		if(!(st.st_mode & S_IWUSR)) 
			return 1;
	} else {
		if(!(st.st_mode & S_IRGRP))
			return 1;
		if(r_id == id)
			if(!(st.st_mode & S_IWGRP))
				return 1;
	}

	return 0;
}

void chooseCommand(char command[], int report_file, char user[]) {
	if(!strcmp("--add", command))
		addDistrict(report_file, user);
	else if(!strcmp("--list", command))
		list(report_file);
	else if(!strcmp("--remove", command))
		removeDistrict();
	else if(!strcmp("--view", command))
		view();
	else if(!strcmp("--update", command))
		updateThreshold();
	else if(!strcmp("--filter", command))
		filter();
	else printf("Command Error\n");
}

int main(int argc, char **argv) {
	if(argc < 5) {
		printf("Not enough arguments(5)\n");
		return 3;
	}
	
	char *role = checkRole(argc, argv[2]);
	if(!role) {
		printf("Choose role manager / inspector\n");
		return 1;
	}
	
	char district[32];
	char commands[nrc][16] = {"--list", "--add", "--remove", "--filter", "--view", "--update"};
	char command[16];
	char user[20] = "noname";

	for(int i = 3; i < argc; ++i) {
		int found = 0;
		int f = 0;

		if(!found)
			for( int j = 0; j < nrc; ++j) {
				if(!strcmp(argv[i], commands[j])) {
					strcpy(command, commands[j]);
					strcpy(district, argv[i + 1]);
	
					found = 1;
					break;
				}
			}
			
		if(!f)
			if(!strcmp("--user", argv[i])) {
				f = 1;
				strcpy(user, argv[i + 1]);
			}
	}

	char path[100];
	makeDir(district, path);

	int log_file = openFile("log", path);
	int report_file = openFile("reports.dat", path);
        int district_file = openFile("district.cfg", path);

	if(checkFilePermission(log_file, role, report_file) > 0) {
		printf("Role doesn t have permission to access file\n");
		return 4;
	}
	if(checkFilePermission(district_file, role, report_file) > 0) {
		printf("Role doesn t have permission to access file\n");
		return 4;
	}
	if(checkFilePermission(report_file, role, report_file) > 0) {
		printf("Role doesn t have permission to access file\n");
		return 4;
	}
	
	chooseCommand(command, report_file, user);



	close(report_file);
	close(district_file);
	close(log_file);
	printf("DONE\n");
	return 0;
}
