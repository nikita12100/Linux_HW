#include "my_filesystem.h"


struct file_system* create_fs(const char* file_name, const size_t filesize)
{
	FILE* pFile;
	char reset_block[RESET_BLOCK_SIZE];
	size_t remaining_bytes = 0;
    size_t bytes_to_save = 0;
    size_t blocks_count = 0;
    size_t i = 0;
	struct file_system* fs;
	struct superblock spblock;

	if(!(pFile = fopen(file_name, "wb")))
	{
			return NULL;
	}

	memset(reset_block, 0, sizeof(reset_block));

	remaining_bytes = filesize;
	while(remaining_bytes > 0)
	{
		bytes_to_save = remaining_bytes < RESET_BLOCK_SIZE ? remaining_bytes : RESET_BLOCK_SIZE;
		fwrite(reset_block, 1, bytes_to_save, pFile);
		remaining_bytes -= bytes_to_save;
	}

	// assign space to blocks and get blocks count
	blocks_count = block_count_from_space(filesize);

	spblock.total_block_count = blocks_count;
	spblock.free_blocks_count = blocks_count;
	spblock.size = filesize;
	fseek(pFile, 0, SEEK_SET);
	fwrite(&spblock, sizeof(spblock), 1, pFile);

	fs = (struct file_system*)malloc(sizeof(struct file_system));
	fs->pFile = pFile;
	fs->blocks_count = blocks_count;
	fs->table = (int*)malloc(blocks_count* sizeof(int));
	fs->inodes = (struct inode*)malloc(blocks_count* sizeof(struct inode));

	// reset i-nodes and table
	for(i = 0 ; i < blocks_count; i++)
	{
		fs->table[i] = -2;
		fs->inodes[i].first_file_block = -1;
	}

	return fs;
}

// Mount file system
struct file_system* mount_fs(const char* file_name)
{
	FILE* pFile;
	size_t file_size = 0;
	struct file_system* fs;
	struct superblock spblock;

	if(!(pFile = fopen(file_name, "r+b")))
    {
		return NULL;
    }
	fseek(pFile, 0, SEEK_END);
	file_size = ftell(pFile);
	if(file_size < sizeof(struct superblock))
	{
		fclose(pFile);
		return NULL;
	}
	fs = malloc(sizeof(struct file_system));
	// go to file begin
	fseek(pFile, 0, SEEK_SET);
	if(fread(&spblock, sizeof(spblock), 1, pFile) <= 0)
	{
		fclose(pFile);
		return NULL;
	}
	fs->blocks_count = spblock.total_block_count;

	// allocate memory for i-nodes table and read i-nodes from the file
	fs->inodes = malloc(sizeof(struct inode)* fs->blocks_count);
	if(!fread(fs->inodes, sizeof(struct inode), fs->blocks_count, pFile))
	{
		fclose(pFile);
		return NULL;
	}

	// allocate memory for fat table and read fat table from the file
	fs->table = malloc(sizeof(int)* fs->blocks_count);
	if(!fread(fs->table, sizeof(int), fs->blocks_count, pFile))
	{
		fclose(pFile);
		return NULL;
	}
	fs->pFile = pFile;

	return fs;
}

// Unmount file system
void unmount_fs(struct file_system* fs)
{
	int position_first_byte = 0;
	int position_table = 0;
	// reposition stream position indicator to first byte behind superblock
	fseek(fs->pFile, sizeof(struct superblock), SEEK_SET);
	// write all inodes to the file system
    position_first_byte = fwrite(fs->inodes, sizeof(struct inode), fs->blocks_count, fs->pFile);
	// write allocation table to the file
    position_table = fwrite(fs->table, sizeof(int), fs->blocks_count, fs->pFile);

	fclose(fs->pFile);
	free(fs->table);
	free(fs->inodes);
	free(fs);
	fs = NULL;
}

// Delete file system
void delete_fs(const char* file_name)
{
	unlink(file_name);
}

// Show memory map
void dump_fs(const struct file_system* fs)
{
	size_t occupied_blocks_count;
	int index;
	printf("\n i-nodes list:\n");
	unsigned int i;
	for(i = 0; i < fs->blocks_count; i++)
	{
		printf("\n\nId: %d\n", i);
		printf("Name: %s\n", fs->inodes[i].name);
		printf("File size: %d\n", fs->inodes[i].file_size);
		printf("First data block: %d\n\n", fs->inodes[i].first_file_block);
	}
	printf("Files occupy blocks:\n");
	occupied_blocks_count = 0;
	for(i = 0; i < fs->blocks_count; i++)
	{
		if(fs->table[i] > -2)
			occupied_blocks_count++;
		index = fs->inodes[i].first_file_block;
		if(index >= 0)
		{
			printf("%s\t", fs->inodes[i].name);
			while(index >= 0)
			{
				printf("%d, ", (int)index);
				index = fs->table[index];
			}
			printf("\n");
		}
	}
	printf("\nTable:\n");
	for(i = 0; i < fs->blocks_count; i++)
	{
		printf("%d, ", fs->table[i]);
	}
	printf("\n");
	printf("Free blocks count: %d\n\n", (int)(fs->blocks_count - occupied_blocks_count));
}

// List all files in the fat file system
void list_files_fs(const struct file_system* fs)
{
	size_t i;
	printf("File system contains:\n");
	for(i = 0; i < fs->blocks_count; i++)
	{
		if(fs->inodes[i].first_file_block >= 0)
		{
			printf("%s\n", fs->inodes[i].name);
		}
	}
}

// Copy file to fat file system
int copy_to_fs(const struct file_system* fs, const char* source_file_name, const char* dest_file_name)
{
	size_t required_blocks_count, i, free_block_index, old_index;
	int source_file_length = 0;
	FILE* sFile;
	struct inode* new_file_inode;
	char buffer[BLOCK_SIZE];
	int read_size = 0;

	// open the source file
	sFile = fopen(source_file_name, "rb");
	if(sFile == NULL)
    {
		return -1;
    }
	// check if there is file with the same name in file system
	for(i = 0; i < fs->blocks_count; i++)
	{
		if(fs->inodes[i].first_file_block >=0 && strncmp(fs->inodes[i].name, dest_file_name, MAX_NAME_LENGTH) == 0)
        {
			return -2;
        }
	}
	fseek(sFile, 0, SEEK_END);
	source_file_length = ftell(sFile);
	fseek(sFile, 0, SEEK_SET);
	required_blocks_count = compute_required_blocks_count(source_file_length, BLOCK_SIZE);
	int a = get_free_blocks_count(fs);
	if(get_free_blocks_count(fs) < required_blocks_count)
    {
		return -3;
    }

	// find free i-node
	for(i = 0; i < fs->blocks_count; i++)
	{
		if(fs->inodes[i].first_file_block == -1)
		{
			new_file_inode = &(fs->inodes[i]);
			break;
		}
	}
	new_file_inode->file_size = source_file_length;
	strncpy(new_file_inode->name, dest_file_name, MAX_NAME_LENGTH);
	free_block_index = -1;
	old_index = free_block_index;
	if(source_file_length != 0)
	{
		int remaining_file_length = source_file_length;
		while(remaining_file_length > 0)
		{
			// find new free block
			for(i = free_block_index + 1; i < fs->blocks_count; i++)
			{
				if(fs->table[i] == -2)
				{
					free_block_index = i;
					break;
				}
			}
			if(old_index != -1)
            {
				fs->table[old_index] = free_block_index;
            }
			if(old_index == -1)
            {
				new_file_inode->first_file_block = free_block_index;
            }
			old_index = free_block_index;
			read_size = fread(buffer, 1, sizeof(buffer), sFile);
			int b = get_block_position_in_bytes(fs, free_block_index);
			int er;
			er = ferror(fs->pFile);
			fseek(fs->pFile, b, SEEK_SET);
			er = ferror(fs->pFile);
			int ab;
			ab = fwrite(buffer, 1, read_size, fs->pFile);
			er = ferror(fs->pFile);
			remaining_file_length -= BLOCK_SIZE;
		}
		fs->table[old_index] = -1;
	}
	else
	{
		for(i = 0; i < fs->blocks_count; i++)
		{
			if(fs->table[i] == -2)
			{
				free_block_index = i;
				break;
			}
		}
		new_file_inode->first_file_block = free_block_index;
		fs->table[free_block_index] = -1;
	}
	fclose(sFile);
	return 0;
}

// Copy file from fat file system to linux
int copy_from_fs(const struct file_system* fs, const char* source_file_name, const char* dest_file_name)
{
	FILE* dFile;
	size_t i = 0;
    size_t current_block = 0;
    size_t size_to_read = 0;
	char buffer[BLOCK_SIZE];
	struct inode* current_file_node;

	if(!(dFile = fopen(dest_file_name, "wb")))
    {
		return -1;
    }

	current_file_node = NULL;
	for(i = 0 ; i < fs->blocks_count; i++)
	{
		if(fs->inodes[i].first_file_block >= 0 && strncmp(fs->inodes[i].name, source_file_name, MAX_NAME_LENGTH) == 0)
		{
			current_file_node = &(fs->inodes[i]);
			break;
		}
	}
	if(current_file_node == NULL)
    {
		return -2;
    }

	// copy file block by block
	current_block = current_file_node->first_file_block;
	while(current_block != -1)
	{
		if(fs->table[current_block] == -1)
        {
			size_to_read = (int)((current_file_node->file_size)%BLOCK_SIZE);	// size to read for the last block
        }
		else {
			size_to_read = BLOCK_SIZE;
		}
		fseek(fs->pFile, get_block_position_in_bytes(fs, current_block), SEEK_SET);
		// read current block
		if(fread(buffer, 1, size_to_read, fs->pFile) != size_to_read)
		{
			fclose(dFile);
			return -3;
		}
		fwrite(buffer, 1, size_to_read, dFile);
		current_block = fs->table[current_block];
	}
	fclose(dFile);

	return 0;
}

// Delete file system
void delete_file_fs(const struct file_system* fs, const char* file_name)
{
	int i = 0;
	int index = 0;
	int new_index = 0;
	char* emptyName = "";
	for(i = 0; i < fs->blocks_count; i++)
	{
		// if file found, reset FAT table and clear i-node
		if(fs->inodes[i].first_file_block >= 0 && strncmp(fs->inodes[i].name, file_name, MAX_NAME_LENGTH) == 0)
		{
			fs->inodes[i].file_size = 0;
			strncpy(fs->inodes[i].name, emptyName, MAX_NAME_LENGTH);
			index = fs->inodes[i].first_file_block;
			do
			{
				new_index = fs->table[index];
				fs->table[index] = -2;
				index = new_index;
			} while(index >= 0);
			fs->inodes[i].first_file_block = -1;

			return;
		}
	}
}

// Return free blocks in file system
size_t get_free_blocks_count(const struct file_system* fs)
{
	int i = 0;
	size_t free_blocks = 0;
	for(i = 0; i < fs->blocks_count; i++)
	{
		if(fs->table[i] == -2)
        {
			free_blocks++;
        }
	}

	return free_blocks;
}

// Return number of blocks to cover given space
size_t block_count_from_space(const size_t total_space_size)
{
//	each i-node has >= 1 corresponding block and >= 1 value
//	in table. Not all i-node must be utilized.
	return (total_space_size - sizeof(struct superblock)) /
			(sizeof(struct inode) + sizeof(size_t) + BLOCK_SIZE);
}

// Return byte-position of the given block in the file
size_t get_block_position_in_bytes(const struct file_system* fs, size_t block_number)
{
	return sizeof(struct superblock) + sizeof(struct inode)* fs->blocks_count +
			sizeof(int)* fs->blocks_count + block_number* BLOCK_SIZE;
}

// Return number of blocks to be occupied by the new file
int compute_required_blocks_count(const int file_size, const int block_size)
{
	int blocks_required = (size_t)(file_size/BLOCK_SIZE);
	if(file_size % block_size != 0 || file_size == 0)
		blocks_required++;
	return blocks_required;
}
