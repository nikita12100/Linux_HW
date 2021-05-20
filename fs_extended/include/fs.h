#pragma once

#include <stddef.h>

#define FILE_NAME_LENGTH 256

typedef struct INode {
    char name[FILE_NAME_LENGTH];
    unsigned int file_size;
	int first_file_block;
} INode;

typedef struct FirstINode {
	INode node;
	int size;
	unsigned char flags;
	int last_inode;
} FirstINode;

typedef struct Directory {
	char name[FILE_NAME_LENGTH];
	int inode;
} Directory;

typedef struct SuperBlock {
	int size;
	int total_block_count;
    unsigned int int_size;
    unsigned int superblock_type_size;
    unsigned int inode_type_size;
    unsigned int directory_size;
} SuperBlock;


char check_types_size(SuperBlock);
SuperBlock get_superblock_params();
