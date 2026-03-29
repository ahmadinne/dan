#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "operations.h"

// Variables
char dan_config_path[512];
char dot_config_path[512];

// Functions
void setup_env() {
	const char *home_dir = getenv("HOME");
	if (home_dir == NULL) {
		printf("Error: HOME Env variable not found.\n");
		exit(1);
	} 

	// format our path
	snprintf(dan_config_path, sizeof(dan_config_path), "%s/.config/dan/config", home_dir);

	// create temporary copy of the path we can modify
	char tmp_path[512];
	strncpy(tmp_path, dan_config_path, sizeof(tmp_path) - 1);

	// mkdir -p but in c, start at tmp_path + 1 to skip very first root '/'
	for (char *p = tmp_path + 1; *p != '\0'; p++) {
		if (*p == '/') {
			*p = '\0'; // temporarily end the string at this folder
			mkdir(tmp_path, 0700); // create the folder
			*p = '/'; //put the slash back and continue down the path
		}
	}

	// after the loop finishes, all parent dir now exist.
	if (access(dan_config_path, F_OK) == -1) {
		FILE *file = fopen(dan_config_path, "w");
		if (file != NULL) {
			printf("[dan] created config file: %s\n", dan_config_path);
		} else {
			perror("[err] failed to create config file");
		}
	}
}

void load_config() {
	FILE *file = fopen(dan_config_path, "r");
	if (file == NULL) return;

	char line[512];
	// fgets read the dan config line by line into the 'line' buffer
	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "path = ", 7) == 0) { // strncmp compares the first 7 characters to see if it matches "path = "
			strcpy(dot_config_path, line + 7); // copy everything after the 7 characters "path = " into our global variable
			
			dot_config_path[strcspn(dot_config_path, "\n")] = 0; // strip the hidden newline char '\n' that fgets include
			break; // already found the path, doesn't need to continue reading.
		}
	}
	fclose(file); // always close your file to free the memory
}

int get_local_path(const char *pkg_name, char *out_path) {
	char dan_file[1024];
	snprintf(dan_file, sizeof(dan_file), "%s/.dan", dot_config_path);

	FILE *df = fopen(dan_file, "r");
	if (df == NULL) {
		return 0; //.dan file doesn't exist yet
	}

	char line[1024];
	char search_str[256];

	snprintf(search_str, sizeof(search_str), "%s =", pkg_name);
	int search_len = strlen(search_str);

	while (fgets(line, sizeof(line), df)) {
		if (strncmp(line, search_str, search_len) == 0) {
			strcpy(out_path, line + search_len + 1); // we use +1 to skip the space after equal sign

			out_path[strcspn(out_path, "\n")] = 0; // strip hidden newline character

			fclose(df);
			return 1; // success
		}
	}

	fclose(df);
	return 0; // package not found in the file
}

void rmv_local_path(const char *pkg_name) {
	char dan_file[1024];
	char tmp_file[1024];
	snprintf(dan_file, sizeof(dan_file), "%s/.dan", dot_config_path);
	snprintf(tmp_file, sizeof(tmp_file), "%s/.dan.tmp", dot_config_path);

	FILE *df = fopen(dan_file, "r");
	if (df == NULL) return;

	FILE *tmp = fopen(tmp_file, "w");
	if (tmp == NULL) {
		fclose(df);
		return;
	}

	char line[1024];
	char search_str[256];
	snprintf(search_str, sizeof(search_str), "%s =", pkg_name);
	int search_len = strlen(search_str);

	while (fgets(line, sizeof(line), df)) {
		if (strncmp(line, search_str, search_len) != 0) {
			fputs(line, tmp);
		}
	}

	fclose(df);
	fclose(tmp);

	remove(dan_file);
	rename(tmp_file, dan_file);
}

void help() {
	printf("dan\n");
	printf("a tiny and simple dotfile manager, now in c!\n\n");
	printf("usage: dan <operation> [...]\n");
	printf("operations:\n");
	printf("\tdan help\t\tShow the help page, list of operations and batch operations.\n");
    printf("\tdan init\t<path>\tInitialize current directory as dotfiles directory.\n");
    printf("\tdan list\t[pkg]\tShow existing folder or files inside dotfiles.\n");
    printf("\tdan sync\t[pkg]\tAdd specified folder or file into dotfiles.\n");
    printf("\tdan apply\t[pkg]\tApply specified folder or file from dotfiles into local.\n");
    printf("\tdan remove\t<pkg>\tRemove specified folder or file from dotfiles.\n");
}

void dlist() {
	printf("[Stub] Listing dotfiles...\n");
}

void dinit(int argc, char *argv[]) {
	char *choice = argv[2];
	char curdir[512];
	char target[1024];
	char dan_file[2048];

	// get current working directory
	if (getcwd(curdir, sizeof(curdir)) == NULL) {
		perror("[err] failed to get current directory");
		return;
	}

	// determine the target path
	if (argc < 3) {
		snprintf(target, sizeof(target), "%s", curdir);
	} else {
		snprintf(target, sizeof(target), "%s/%s", curdir, choice);
	}

	// write the path to dan config
	FILE *cfg = fopen(dan_config_path, "w");
	if (cfg == NULL) {
		perror("[err] failed to open dan config");
		return;
	} else {
		fprintf(cfg, "path = %s\n", target);
		fclose(cfg);
	}

	// create the dot config (.dan) file
	snprintf(dan_file, sizeof(dan_file), "%s/.dan", target);
	FILE *dan = fopen(dan_file, "w");
	if (dan != NULL) {
		fclose(dan);
	}

	printf("[dan] initialized path at: %s\n", target);
}

void dsync(int argc, char *argv[]) {
	if (argc < 3) {
		printf("error: no target specified (use 'dan help' for help)\n");
		return;
	}

	for (int i = 2; i < argc; i++) {
		char *target = argv[i];
		char dest[1024];
		char dummy_path[1024];

		snprintf(dest, sizeof(dest), "%s/%s", dot_config_path, target);
		sync_item(target, dest);

		if (get_local_path(target, dummy_path) == 0) {
			char dan_file[1024];
			snprintf(dan_file, sizeof(dan_file), "%s/.dan", dot_config_path);

			FILE *df = fopen(dan_file, "a");
			if (df != NULL) {
				char curdir[512];
				if (getcwd(curdir, sizeof(curdir)) != NULL) {
					fprintf(df, "%s = %s/%s\n", target, curdir, target);
				}

				fclose(df);
			}
		} else {
			// printf("[dan] '%s' is already tracked in .dan, skipping path saving\n", target);
		}
	}
}

void dapply(int argc, char *argv[]) {
	if (argc < 3) {
		printf("error: no target specified (use 'dan help' for help)\n");
		return;
	}

	for (int i = 2; i < argc; i++) {
		char *target = argv[i];
		char src_path[1024];
		char local_dst_path[1024];

		snprintf(src_path, sizeof(src_path), "%s/%s", dot_config_path, target);

		if (get_local_path(target, local_dst_path)) {
			printf("[dan] applying %s to %s...\n", target, local_dst_path);

			route_copy(src_path, local_dst_path);
			printf("successfully applied %s!\n", target);
		} else {
			printf("error: could not find the original path for '%s' inside .dan", target);
		}
	}
}

void dremove(int argc, char *argv[]) {
	if (argc < 3) {
		printf("error: no target specified (usee 'dan help' for help)\n");
		return;
	}

	for (int i = 2; i < argc; i++) {
		char *target = argv[i];
		char target_path[1024];

		snprintf(target_path, sizeof(target_path), "%s/%s", dot_config_path, target);
		printf("[dan] removing %s from dotfiles...\n", target);

		remove_item(target_path);
		rmv_local_path(target);

		printf("successfully removed %s!\n", target);
	}
}

void dcheck() {
	printf("[Stub] Checking if current directory is a dan directory...\n");
}

// Run!
int main(int argc, char *argv[]) {
	setup_env();
	load_config();

	// if argument less than 2, run dcheck
	if (argc < 2) {
		dcheck();
		return 0;
	}

	// first argument is the option
	char *option = argv[1];

	if (strcmp(option, "help") == 0) {
		help();
	} else if (strcmp(option, "list") == 0) {
		dlist();
	} else if (strcmp(option, "init") == 0) {
		dinit(argc, argv);
	} else if (strcmp(option, "sync") == 0) {
		dsync(argc, argv);
	} else if (strcmp(option, "remove") == 0) {
		dremove(argc, argv);
	} else if (strcmp(option, "apply") == 0) {
		dapply(argc, argv);
	} else {
		printf("wrong options (use 'dan help' for options list)\n");
		return 1;
	}

	return 0;
}
