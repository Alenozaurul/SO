#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

struct stat st = {0};

char role[16];
char username[32];
char district[32];
char command[32];

int log_file, report_file, district_file;

typedef struct Report {
       	int id;
	int level;
 	float x, y;
	time_t timestamp;
	char username[32];
	char category[32];
	char description[128];
} Report;


char *getRole(int argc, char *argv[]) {
	for(int i = 1; i < argc - 1; ++i) {
		if(!strcmp(argv[i], "--role")) {
			return argv[i + 1];
		}
	}

	printf("Role couldn't be found\n");
	return "";
}

char *getUsername(int argc, char *argv[]) {
	for(int i = 1; i < argc - 1; ++i) {
		if(!strcmp(argv[i], "--user")) {
			return argv[i + 1];
		}
	}
	
	return "unknown";
}

char *getDistrict(int argc, char *argv[]) {
	char commands[7][32] = {"--list", "--remove_report", "--add", "--filter", "--view",
					"--update", "--remove_district"};

	for(int i = 1; i < argc - 1; ++i) {
		for(int j = 0; j < argc - 1; ++j) {
			if(!strcmp(argv[i], commands[j])) {
				strcpy(command, argv[i]);
				return argv[i + 1];
			}
		}
	}
	
	printf("Command not found( add, list, view, filter, update, remove_report, remove_district)\n");
       	return "";	
}

int openFile(char *path, char *name) {
	int fd;
	
	char filename[160];
	strcpy(filename, path);
	strcat(filename, name);

	if(stat(filename, &st) == 0) {
		if( (fd = open(filename, O_RDWR)) == -1 ) {
			perror("Cannot open %s\n");
			return -1;
		}
	} else {
		if( (fd = open(filename,O_CREAT | O_RDWR, 0664)) == -1 ) {
			perror("Cannot create %s\n");
			return -1;
		}

		if(!strstr(filename, "report")) {
			if(chmod(filename, 0640) != 0) {
				perror("Cannot change permission when creating file\n");
				return -1;
			}
		} else {
			if(chmod(filename, 0664) != 0) {
				perror("Cannot change permission when creating file\n");
				return -1;
			}
		}
	}

	return fd;
}

int openDistrict(char *path) {
	if(mkdir(path, 0750) == 0) {
		printf("Created %s \n", path);
		if(chmod(path, 0750) != 0) {
			perror("Cannot change permission when creating district directory\n");
			return 1;
		}
	}

	log_file = openFile(path, "log");
	report_file = openFile(path, "report.dat");
	district_file = openFile(path, "district.cfg");

	if(log_file == -1 || report_file == -1 || district_file == -1)
		return 1;

	return 0;
}

void closeFiles() {
	close(log_file);
	close(report_file);
	close(district_file);
}

void printReport(Report report) {
	printf("Report id: %i\n", report.id);
	printf("Username: %s\n", report.username);
	printf("Coordinates: %.2f, %.2f\n", report.x, report.y);
	printf("Category: %s\n", report.category);
	printf("Severity level: %i\n", report.level);
	printf("Timestamp: %s", ctime(&report.timestamp));
	printf("Description: %s\n\n", report.description);
}

int checkRole(char role_to_check[]) {
	if(!strcmp("manager", role))
		return 1;
	if(!strcmp(role, role_to_check))
		return 1;

	printf("Role doesn't have permissions\n");
	return 0;
}

void addReport(){
	if(!checkRole("inspector")) {
		char msg[256];
		sprintf(msg, "User %s tried to add a report for district %s\n", username, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	Report report;
	printf("Category: ");
	scanf("%s", report.category);
	printf("Severity level: ");
	scanf("%i", &report.level);
	printf("X, Y: ");
	scanf("%f %f", &report.x, &report.y);
	printf("Description: ");
	getchar();
	fgets(report.description, sizeof(report.description), stdin);
	
	report.timestamp = time(NULL);
	strcpy(report.username, username);


	off_t offset = lseek(report_file, -sizeof(Report), SEEK_END);
	if(offset == -1)
		report.id = 1;
	else { 
		Report last;
		read(report_file, &last, sizeof(Report));
		report.id = last.id + 1;
	}

	lseek(report_file, 0, SEEK_END);
	write(report_file, &report, sizeof(report));

	char msg[256];
	sprintf(msg, "Report %d has been added to district %s by %s\n", report.id, district, report.username);
	write(log_file, &msg, strlen(msg));
}

void listReports() {
	if(!checkRole("inspector")) {
		char msg[256];
		sprintf(msg, "User %s tried to list all reports from district %s\n", username, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	if(fstat(report_file, &st) == -1) {
		perror("Cannot access report file in listReports\n");
		return;
	}
	
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
	printf("Permissions: %s\nSize: %ld\nLast modified: %s\n", out, st.st_size, ctime(&st.st_mtime));

	Report report;
	while(read(report_file, &report, sizeof(Report)) == sizeof(Report)) {
		printReport(report);
	}

	char msg[256];
	sprintf(msg, "Reports from district %s have been listed by %s\n", district, report.username);
	write(log_file, &msg, strlen(msg));	
}

void viewReport(int argc, char *argv[]) {
	int id = 0;

	if(!checkRole("inspector")) {
		char msg[256];
		sprintf(msg, "User %s tried to view report %d from district %s\n", username, id, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	if(fstat(report_file, &st) == -1) {
		perror("Cannot access report file in viewReport\n");
		return;
	}
	
	for(int i = 1; i < argc - 2; ++i) {
		if(!strcmp(argv[i], "--view")) {
			id = atoi(argv[i + 2]);
		}
	}
	
	Report report;
	int found = 0;
	while(read(report_file, &report, sizeof(report)) == sizeof(Report)) {
		if(report.id == id) {
			found = 1;
			break;
		}
	}	
	
	if(!found) {
	       	printf("Id couldn't be found\n");
		
		char msg[256];
		sprintf(msg, "User %s tried to view report %d from district %s but id failed\n",
			       	username, id, district);
		write(log_file, &msg, strlen(msg));	
       		
		return;
	}

	printReport(report);
	
	char msg[100];
	sprintf(msg, "Report %d view by %s\n", id, report.username);
	write(log_file, &msg, strlen(msg));	
}

void updateTreshold(int argc, char *argv[]) {
	if(!checkRole("manager")) {
		char msg[256];
		sprintf(msg, "User %s tried to update the treshold from district %s\n", username, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	int value = 0;
	for(int i = 1; i < argc - 2; ++i) {
		if(!strcmp(argv[i], "--update")) {
			value = atoi(argv[i + 2]);
		}
	}

	if(value < 1 || value > 3) {
		printf("Severity level is between 1 and 3\n");
		
		char msg[256];
		sprintf(msg, "User %s tried to update the treshold from district %s with invalid value\n",
			       	username, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	write(district_file, &value, sizeof(value));
	ftruncate(district_file, sizeof(value));
	
	char msg[100];
	sprintf(msg, "Treshold updated by %s\n", username);
	write(log_file, &msg, strlen(msg));	
}

int getOperation(char string[]) {
	char *p = strtok(string, ":");
	p = strtok(NULL, ":");

	if(!strcmp(p, "=="))
		return 1;
	if(!strcmp(p, "!="))
		return 2;
	if(!strcmp(p, "<"))
		return 3;
	if(!strcmp(p, "<="))
		return 4;
	if(!strcmp(p, ">"))
		return 5;
	if(!strcmp(p, ">="))
		return 6;

	return 0;
}

char *getCondition(char string[]) {
	return strtok(string, ":");
}

int handleInt(int op, int number, int cas, Report report) {
	if(cas == 1) {
		switch(op) {
			case (1):
				if(number == report.timestamp)
					return 1;
				break;
			case (2):
				if(number != report.timestamp)
					return 1;
				break;
			case (3):
				if(number > report.timestamp)
					return 1;
				break;
			case (4):
				if(number >= report.timestamp)
					return 1;
				break;
			case (5):
				if(number < report.timestamp)
					return 1;
				break;
			case (6):
				if(number >= report.timestamp)
					return 1;
				break;
			default:
				return 0;
		}
	} else if(cas == 0) {
		switch(op) {
			case (1):
				if(number == report.level)
					return 1;
				break;
			case (2):
				if(number != report.level)
					return 1;
				break;
			case (3):
				if(number > report.level)
					return 1;
				break;
			case (4):
				if(number >= report.level)
					return 1;
				break;
			case (5):
				if(number < report.level)
					return 1;
				break;
			case (6):
				if(number <= report.level)
					return 1;
				break;
			default:
				return 0;
		}
	}

	return 0;
}	

int handleStr(int op, char string[], int cas, Report report) {
	if(cas == 2) {
		int n = strcmp(report.username, string);
		
		if(op == 1 || op == 3 || op == 6)
			if(n == 0)
				return 1;
		if(op == 3 || op == 4)
			if(n < 0)
				return 1;
		if(op == 5 || op == 6)
			if(n > 0)
				return 1;
		if(op == 2 && n != 0)
			return 1;	
	} else if(cas == 3) {
		int n = strcmp(report.category, string);
//		printf("\n%i\n", n);

		if(op == 1 || op == 3 || op == 6)
			if(n == 0)
				return 1;
		if(op == 3 || op == 4)
			if(n < 0)
				return 1;
		if(op == 5 || op == 6)
			if(n > 0)
				return 1;
		if(op == 2 && n != 0)
			return 1;	
	}

	return 0;
}

void filterReports(int argc, char *argv[]) {
	if(!checkRole("inspector")) {
		char msg[256];
		sprintf(msg, "User %s tried to filter reports from district %s\n", username, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	int level = 0;
	char insp[32] = "";
	char category[32] = "";
	time_t timestamp = 0;

	int condition[4] = {0}; // == 1   != 2   < 3   <= 4   > 5    >= 6
				//  0 = severity level   1 = timestamp   2 = inspector   3 = category
	for(int i = 1; i < argc - 2; ++i) {
		if(!strcmp(argv[i], "--filter")) {
			for(int j = i + 2; j < argc; ++j) {
				
				char *repl = strdup(argv[j]);
				
				if(!strcmp(getCondition(repl), "severity")) { 
					char *replace = strdup(argv[j]);
					condition[0] = getOperation(replace);
					free(replace);

					char *p = strtok(argv[j], ":");
					p = strtok(NULL, ":");	
					p = strtok(NULL, ":");	
					level = atoi(p);

				}	
				
				free(repl);
				repl = strdup(argv[j]);
				
				if(!strcmp(getCondition(repl), "timestamp")) {
					char *replace = strdup(argv[j]);
					condition[1] = getOperation(replace);		
					free(replace);

					char *p = strtok(argv[j], ":");
					p = strtok(NULL, ":");	
					p = strtok(NULL, ":");	
				
					timestamp = atoi(p);

					/*
					struct tm tm_time = {0};
					if(strptime(p, "%Y-%m-%d %H:%M:%S", &tm_time) == NULL) {
						perror("Cannot set time\n");
						return;
					}	

					timestamp = mktime(&tm_time);
					*/
				}
				
				free(repl);
				repl = strdup(argv[j]);
				
				if(!strcmp(getCondition(repl), "inspector")) { 
					char *replace = strdup(argv[j]);
					condition[2] = getOperation(replace);		
					free(replace);

					char *p = strtok(argv[j], ":");
					p = strtok(NULL, ":");	
					p = strtok(NULL, ":");	
					strcpy(insp, p);	
				}
					
				free(repl);
				repl = strdup(argv[j]);
				
				if(!strcmp(getCondition(repl), "category")) { 
					char *replace = strdup(argv[j]);
					condition[3] = getOperation(replace);	
					free(replace);
					
					char *p = strtok(argv[j], ":");
					p = strtok(NULL, ":");	
					p = strtok(NULL, ":");	
					strcpy(category, p);	
				}	
				
				free(repl);
			}		
		}
	}
	Report report;

	while(read(report_file, &report, sizeof(Report)) == sizeof(Report)) {
		int show = 1;

		for(int i = 0; i < 4; ++i) {
			if(!condition[i]) 
				continue;


			switch(i) {
				case 0:
					if(!handleInt(condition[i], level, i, report))
						show = 0;
					break;	
				case 1:
					if(!handleInt(condition[i], timestamp, i, report))
						show = 0;
					break;	
				case 2:
					if(!handleStr(condition[i], insp, i, report))
						show = 0;
					break;	
				case 3:
					if(!handleStr(condition[i], category, i, report))
						show = 0;
					break;
				default: ;
			}	 
		}

		if(show)
			printReport(report);
	}

	printf("abracadabra\n");

	char msg[100];
	sprintf(msg, "Reports filtered by %s\n", username);
	write(log_file, &msg, strlen(msg));	
}

void removeReport(int argc, char *argv[]) {
	int id;
	for(int i = 0; i < argc - 2; ++i) {
		if(!strcmp(argv[i], "--remove_report")) {
			id = atoi(argv[i + 2]);
			
			if(id <= 0) {
				printf("Invalid id\n");
				return;
			}

			break;
		}
	}

	if(!checkRole("manager")) {
		char msg[256];
		sprintf(msg, "User %s removed report %d from district %s\n", username, id, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}
	
	Report report;
	long cursor = -1;

	while(read(report_file, &report, sizeof(Report)) == sizeof(Report)) {
		if(id == report.id) {
			cursor = lseek(report_file, -sizeof(Report), SEEK_CUR);
			break;
		}
	}

	if(fstat(report_file, &st) == -1) {
		perror("Error fstat in removeReport\n");
		return;
	}

	if(cursor != -1) {
		Report next;
		long sread = cursor + sizeof(Report);
		long swrite = cursor;

		while(sread < st.st_size) {
			lseek(report_file, sread, SEEK_SET);

			if(read(report_file, &next, sizeof(Report)) != sizeof(Report)) 
				break;

			lseek(report_file, swrite, SEEK_SET);
			write(report_file, &next, sizeof(Report));

			sread += sizeof(Report);
			swrite += sizeof(Report);
		}

		ftruncate(report_file, st.st_size - sizeof(Report));
	}

	char msg[128];
	sprintf(msg, "User %s removed report %d from district %s\n", username, id, district);
	write(log_file, &msg, strlen(msg));	
}

void removeDistrict() {
	if(!checkRole("manager")) {
		char msg[256];
		sprintf(msg, "User %s tried to remove the district %s\n", username, district);
		write(log_file, &msg, strlen(msg));	

		return;
	}

	pid_t pid;
	if( (pid = fork()) < 0 ) {
		perror("Fork error on creating child for the removing a district\n");
		return;
	}

	if(!pid) {
		char remove[128] = "rm -drf ../districts/";
		strcat(remove, district);

		system(remove);

		exit(EXIT_SUCCESS);
	}

	char msg[128];
	sprintf(msg, "District %s removed by %s\n", district, username);
	write(log_file, &msg, strlen(msg));	
}



void handleCommand(int argc, char *argv[]) {
	if(!strcmp("--add", command))
		addReport();
	if(!strcmp("--list", command))
		listReports();
	if(!strcmp("--view", command))
		viewReport(argc, argv);
	if(!strcmp("--update", command))
		updateTreshold(argc, argv);
	if(!strcmp("--filter", command))
		filterReports(argc, argv);
	if(!strcmp("--remove_report", command))
		removeReport(argc, argv);
	if(!strcmp("--remove_district", command))
		removeDistrict();
}


int main(int argc, char *argv[]) {
	if(argc < 5) {
		printf("Usage: ./main --role role_name --user user_name --command command_name\n");
		exit(EXIT_FAILURE);
	}

	strcpy(role, getRole(argc, argv));
	strcpy(username, getUsername(argc, argv));
	strcpy(district, getDistrict(argc, argv)); // this also gets the command and checks if its valid

	

	char path[128] = "../districts/";
	strcat(path, district);
	strcat(path, "/");

	if(openDistrict(path) || !strcmp(role, "") || !strcmp(district, "")) {
		closeFiles();
		exit(EXIT_FAILURE);	
	}

	handleCommand(argc, argv);

	closeFiles();
	exit(EXIT_SUCCESS);
}
