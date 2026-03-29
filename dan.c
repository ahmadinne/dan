#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "operations.h"

// Variables
char config_file[512];
char dotfiles_path[512];
char dan_entry_file[1024];

// Functions
void setup_env() {
	const char *home_dir = getenv("HOME");
	if (home_dir == NULL) {
		printf("Error: HOME Env variable not found.\n");
		exit(1);
	} 

	snprintf(config_file, sizeof(config_file), "%s/.config/dan/config", home_dir); // format our path

	char tmp_path[512];
	strncpy(tmp_path, config_file, sizeof(tmp_path) - 1); // create temporary copy of the path we can modify

	for (char *p = tmp_path + 1; *p != '\0'; p++) { // (mkdir -p), but in c, start at tmp_path + 1 to skip very first root '/'
		if (*p == '/') {
			*p = '\0'; // temporarily end the string at this folder
			mkdir(tmp_path, 0700); // create the folder
			*p = '/'; //put the slash back and continue down the path
		}
	}

	if (access(config_file, F_OK) == -1) { // after the loop finishes, all parent dir now exist.
		FILE *file = fopen(config_file, "w");
		if (file == NULL) {
			perror("[err] failed to create config file");
		}
	}
}

void load_config() {
	FILE *file = fopen(config_file, "r");
	if (file == NULL) return;

	char line[512];
	while (fgets(line, sizeof(line), file)) { // fgets read the dan config line by line into the 'line' buffer
		if (strncmp(line, "path = ", 7) == 0) { // strncmp compares the first 7 characters to see if it matches "path = "
			strcpy(dotfiles_path, line + 7); // copy everything after the 7 characters "path = " into our global variable
			dotfiles_path[strcspn(dotfiles_path, "\n")] = 0; // strip the hidden newline char '\n' that fgets include

			snprintf(dan_entry_file, sizeof(dan_entry_file), "%s/.dan", dotfiles_path); // format the dan entry files
			break; // already found the path, doesn't need to continue reading.
		}
	}
	fclose(file); // always close your file to free the memory
}

int get_local_path(const char *pkg_name, char *out_path) {
	FILE *df = fopen(dan_entry_file, "r");
	if (df == NULL) {
		return 0; //.dan file doesn't exist yet
	}

	char line[1024];
	char search_str[256];

	snprintf(search_str, sizeof(search_str), "%s =", pkg_name);
	int search_len = strlen(search_str);

	while (fgets(line, sizeof(line), df)) {
		if (strncmp(line, search_str, search_len) == 0) {
			strcpy(out_path, line + search_len + 1); // we use +1 to skip the space after equal(=) sign
			out_path[strcspn(out_path, "\n")] = 0; // strip hidden newline(\n) character

			fclose(df); // close da filee
			return 1; // success
		}
	}

	fclose(df); //close da filee
	return 0; // package not found in the file
}

void rmv_local_path(const char *pkg_name) {
	char tmp_file[2048];
	snprintf(tmp_file, sizeof(tmp_file), "%s.tmp", dan_entry_file);

	FILE *df = fopen(dan_entry_file, "r");
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

	remove(dan_entry_file);
	rename(tmp_file, dan_entry_file);
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
	if (strlen(dotfiles_path) == 0) {
		printf("error: not a dan directory: .dan\n");
		return;
	}

	const char *BLUE = "\033[1;34m";
	const char *GRAY = "\033[1;70m";
	const char *NONE = "\033[0m";
	int DISTANCE = 30;

	char display_path[512];
	const char *home = getenv("HOME");

	if (home != NULL && strncmp(dotfiles_path, home, strlen(home)) == 0) {
		snprintf(display_path, sizeof(display_path), "~%s", dotfiles_path + strlen(home));
	} else {
		strcpy(display_path, dotfiles_path);
	}

	printf("%s##%s[%sDan%s] %s\n", GRAY, NONE, BLUE, NONE, display_path);

	DIR *dir = opendir(dotfiles_path);
	if (dir == NULL) {
		printf("error: could not open dotfiles directory. \n");
		return;
	}

	struct dirent *entry;
	int is_empty = 1;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0 ||
			strcmp(entry->d_name, ".dan") == 0) continue;

		is_empty = 0;

		char full_path[1024];
		snprintf(full_path, sizeof(full_path), "%s/%s", dotfiles_path, entry->d_name);

		struct stat st;
		if (stat(full_path, &st) == 0) {
			if (S_ISDIR(st.st_mode)) {
				printf("%s-- %s  %-*s\n", GRAY, BLUE, DISTANCE, entry->d_name);
			} else if (S_ISREG(st.st_mode)) {
				printf("%s-- %s  %-*s\n", GRAY, NONE, DISTANCE, entry->d_name);
			}
		}
	}
	closedir(dir);

	if (is_empty) {
		printf("there's nothing inside your dotfiles(%s).\n", display_path);
	}
}

void dinit(int argc, char *argv[]) {
	char *choice = argv[2];
	char curdir[512];
	char target[1024];

	if (getcwd(curdir, sizeof(curdir)) == NULL) { // get current working directory
		perror("[err] failed to get current directory");
		return;
	}

	if (argc < 3) { // determine the target path
		snprintf(target, sizeof(target), "%s", curdir);
	} else {
		snprintf(target, sizeof(target), "%s/%s", curdir, choice);
	}

	printf(":: Checking configuration file...\n");
	char prompt[2048];
	if (strlen(dotfiles_path) > 0) { // Check if path already set
		snprintf(prompt, sizeof(prompt), "  Dotfiles path already set (%s)\n\n:: Rewrite to '%s'?", dotfiles_path, target);
	} else {
		snprintf(prompt, sizeof(prompt), ":: Initialize dotfiles path at '%s'?", target);
	}

	if (!confirmation(prompt)) return; // confirmation safety

	FILE *cfg = fopen(config_file, "w"); // write the path to dan config
	if (cfg == NULL) {
		perror("[err] failed to open dan config");
		return;
	} else {
		fprintf(cfg, "path = %s\n", target);
		fclose(cfg);
	}

	FILE *dan = fopen(dan_entry_file, "w"); // create the dot config (.dan) file
	if (dan != NULL) {
		fclose(dan);
	}

	printf("[dan] initialized path at: %s\n", target);
}

void dsync(int argc, char *argv[]) {
	if (argc < 3) {
		FILE *df = fopen(dan_entry_file, "r");
		if (df == NULL) {
			printf("nothing to sync yet. Start by syncing a file!\n");
			return;
		}

		char prompt[1024];
		snprintf(prompt, sizeof(prompt), "[dan] Proceed to sync all?");
		if (!confirmation(prompt)) return;

		char line[1024];
		while (fgets(line, sizeof(line), df)) {
			char pkg_name[256];
			char local_path[1024];

			int parsed_count = sscanf(line, "%255s = %1023[^\n]", pkg_name, local_path);

			if (parsed_count == 2) {
                char dest_path[1024];
                snprintf(dest_path, sizeof(dest_path), "%s/%s", dotfiles_path, pkg_name);
                sync_item(local_path, dest_path);
            }
		}
		fclose(df);
		return;
	}

	for (int i = 2; i < argc; i++) {
		char *target = argv[i];
		char dst_path[1024];
		char src_path[1024];
		int alr_tracked = 0;

		snprintf(dst_path, sizeof(dst_path), "%s/%s", dotfiles_path, target);

		if (get_local_path(target, src_path)) {
			alr_tracked = 1;
		} else {
			char curdir[512];
			if (getcwd(curdir, sizeof(src_path)) != NULL) {
				if (strcmp(target, ".") == 0) {
					snprintf(src_path, sizeof(src_path), "%s", curdir);
				} else {
					snprintf(src_path, sizeof(src_path), "%s/%s", curdir, target);
				}
			} else {
				perror("error: failed to get current directory");
				continue;
			}
		}

		if (sync_item(src_path, dst_path) == 0) {
			if (!alr_tracked) {
				FILE *df = fopen(dan_entry_file, "a");
				if (df != NULL) {
					fprintf(df, "%s = %s\n", target, src_path);
					fclose(df);
				}
			}
		}
	}
}

void dapply(int argc, char *argv[]) {
	if (argc < 3) {
		FILE *df = fopen(dan_entry_file, "r");
		if (df == NULL) {
			printf("nothing to apply. Your dotfiles is empty!\n");
			return;
		}

		char line[1024];
		while(fgets(line, sizeof(line), df)) {
			char pkg_name[256];
			char local_path[1024];

			int parsed_count = sscanf(line, "%255s = %1023[^\n]", pkg_name, local_path);

			if (parsed_count == 2) {
				char src_path[1024];
				snprintf(src_path, sizeof(src_path), "%s/%s", dotfiles_path, pkg_name);
				route_copy(src_path, local_path);
			}
		}
	}

	for (int i = 2; i < argc; i++) {
		char *target = argv[i];
		char src_path[1024];
		char local_dst_path[1024];

		snprintf(src_path, sizeof(src_path), "%s/%s", dotfiles_path, target);

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

		snprintf(target_path, sizeof(target_path), "%s/%s", dotfiles_path, target);

		char prompt[1024];
		snprintf(prompt, sizeof(prompt), ":: Confirm remove '%s'?", target);
		if (!confirmation(prompt)) continue;

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
