#ifndef OPERATIONS_H
#define OPERATIONS_H

void route_copy(const char *src_path, const char *dst_path);
void sync_item(const char *src_path, const char *dst_path);
int copy_file(const char *src_path, const char *dst_path);
int copy_dir(const char *src_path, const char *dst_path);
void remove_item(const char *path);
int remove_dir(const char *path);

#endif
