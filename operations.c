#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

typedef const char* String;

void route_copy(String src_path, String dst_path) {
	struct stat st;
	if (stat(src_path, &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
			copy_dir(src_path, dst_path);
		} else if (S_ISREG(st.st_mode)) {
			copy_file(src_path, dst_path);
		}
	}
}

int copy_file(String src_path, String dst_path) {
	// rb means read binary, and wb means write binary
	FILE *src = fopen(src_path, "rb");
	if (src == NULL) {
		printf("[err] could not open source file: %s\n", src_path);
		return -1;
	}

	FILE *dst = fopen(dst_path, "wb");
	if (dst == NULL) {
		printf("[err] could not open destination file: %s\n", dst_path);
		return -1;
	}

	char buffer[4096]; // a buffer is just a temporary bucket to hold data while we move it. 4096 (4KB) is a standard, efficient bucket size.
	size_t bytes_read;

	// fread fills the bucket. It returns the number of bytes it successfully get.
	// when it hits the end of the file, it returns 0, and the loop stops.
	while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
		fwrite(buffer, 1, bytes_read, dst);
	}

	// clean up the files, ladies and gentleman.
	fclose(src);
	fclose(dst);

	return 0;
}

int copy_dir(String src_dir, String dst_dir) {
	DIR *dir = opendir(src_dir); // open source dir
	if (dir == NULL) {
		perror("[err] failed to open source directory");
		return -1;
	}

	mkdir(dst_dir, 0700); // create the destination directory if doesn't exist

	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL) {
		// IMPORTANT: skip the current and parent dir (. & ..), if not skipped it'll loop forever.
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// construct the path!
		char src_path[1024];
		char dst_path[1024];
		snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
		snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, entry->d_name);

		// check if the item is a file or folder
		route_copy(src_path, dst_path);
	}

	closedir(dir);
	return 0;
}

void sync_item(String src_path, String dst_path) {
	struct stat st;
	
	if (stat(src_path, &st) == 0) { // check if the source path actually exists
		if (S_ISDIR(st.st_mode)) {
            printf("[Dan] Syncing directory: %s -> %s\n", src_path, dst_path);
			route_copy(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            printf("[Dan] Syncing file: %s -> %s\n", src_path, dst_path);
			route_copy(src_path, dst_path);
        }
    } else {
        printf("error: Target '%s' does not exist in the current directory.\n", src_path);
    }
}

int remove_directory(String path) {
	DIR *dir = opendir(path);
	if (dir == NULL) return -1;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

		char full_path[1024];
		snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

		remove_item(full_path);
	}
	closedir(dir);
	return rmdir(path);
}

void remove_item(String path) {
	struct stat st;
	if (stat(path, &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
			remove_directory(path);
		} else if (S_ISREG(st.st_mode)) {
			remove(path);
		}
	}
}
