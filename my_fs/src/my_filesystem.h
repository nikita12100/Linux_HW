#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define BLOCK_SIZE 2048
#define MAX_NAME_LENGTH 24
#define RESET_BLOCK_SIZE 256

struct file_system
{
	FILE* pFile;
	unsigned int blocks_count;
	int* table;
	struct inode* inodes;
};

struct superblock
{
	size_t size;
	size_t total_block_count;
	size_t free_blocks_count;
};

struct inode
{
	char name[MAX_NAME_LENGTH];
	unsigned int file_size;
	int first_file_block;
};

struct file_system* mount_fs(const char* file_name);
struct file_system* create_fs(const char* file_name, const size_t filesize);
void unmount_fs(struct file_system* fs);
void delete_fs(const char* file_name);
void dump_fs(const struct file_system* fs);
void list_files_fs(const struct file_system* fs);
int copy_from_fs(const struct file_system* fs, const char* source_file_name, const char* dest_file_name);
int copy_to_fs(const struct file_system* fs, const char* source_file_name, const char* dest_file_name);
void delete_file_fs(const struct file_system* fs, const char* file_name);
size_t get_free_blocks_count(const struct file_system* fs);
size_t get_block_position_in_bytes(const struct file_system* fs, size_t block_number);
size_t block_count_from_space(const size_t total_space_size);
int compute_required_blocks_count(const int file_size, const int block_size);

#endif
