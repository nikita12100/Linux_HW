#pragma once

#include <stddef.h>
#include <fs.h>
#include <myutil.h>

#define FLG_DIRECTORY (1 << 0)
#define FLG_FILE (1 << 1)

typedef struct FsDescriptors {
	void* container;
	SuperBlock* superblock;
	unsigned char* lookup_table;
	int root_inode;
} FsDescriptors;

typedef struct DirectoryContent {
	int items_count;
	Directory* items;
} DirectoryContent;

FsDescriptors open_fs(const char*);
FsDescriptors init_fs(const char*, int);
void close_fs(FsDescriptors);
int init_new_file(FsDescriptors, unsigned char);
int read_file(FsDescriptors, int, int, int, void*);
void purge_file(FsDescriptors, int);
void write_file(FsDescriptors, int, int, int, void*);
DirectoryContent read_directory(FsDescriptors, int);
void free_directory(DirectoryContent);
int locate_path(FsDescriptors, Path);
void append_directory(FsDescriptors, int, Directory);
int remove_from_directory(FsDescriptors fs, int, const char*);
int get_blocks_required(FsDescriptors, int);
int get_file_size(FsDescriptors, int);
int* trace_file_blocks(FsDescriptors, int);
