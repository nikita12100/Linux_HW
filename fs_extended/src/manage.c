#include <manage.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <myutil.h>


void* open_and_map(const char* filename, int size) {
	int fd = open(filename, O_RDWR | (size ? O_CREAT : 0), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	die("Error occurred while opening file");
	if (size) {
		ftruncate(fd, size);
		die("Error occurred while preparing file");
	} else {
		struct stat statbuf;
		fstat(fd, &statbuf);
		die("Error occurred while opening file");
		size = statbuf.st_size;
	}
	void* container = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	die("Error occurred while opening file");
	close(fd);
	die("Internal error (open_and_map)");
	return container;
}

void mark_block_in_lookup_table(struct FsDescriptors fs, const int block_index, char occupied) {
	int byte_index = block_index / 8;
	int bit_index = block_index % 8;

	if (block_index >= fs.superblock->total_block_count)
		die_fatal("Internal error (mark_block_in_lookup_table)");
	if (occupied)
		fs.lookup_table[byte_index] |= (1 << bit_index);
	else
		fs.lookup_table[byte_index] &= ~(1 << bit_index);
}

int get_next_free_block_index(struct FsDescriptors fs) {
	int lookup_table_size = (fs.superblock->total_block_count + 7) / 8;
	for (int i = 0; i < lookup_table_size; ++i) {
		unsigned char cur_byte = fs.lookup_table[i];
		if (~cur_byte == 0)
			continue;
		for (int k = 0; k < 8; ++k) {
			if (!(cur_byte & (1 << k))) {
				return i * 8 + k;
			}
		}
	}
	die_fatal("No space");
}

FsDescriptors prepare_descriptors(void* container, char clean_lookup_table) {
	SuperBlock* sblock = (SuperBlock*) container;

	if (!check_types_size(*sblock))
		die_fatal("The filesystem is incompatible with your platfrom");

	int lookup_table_size = (sblock->total_block_count + 7) / 8;
	int metadata_size = sizeof(SuperBlock) + lookup_table_size;
	int metadata_pages_count = (metadata_size + sblock->size - 1) / sblock->size;

	FsDescriptors res = {
		container,
		sblock,
		(unsigned char*) (container + sizeof(SuperBlock)),
		metadata_pages_count
	};

	if (clean_lookup_table) {
		memset(res.lookup_table, 0, lookup_table_size);

		for (int i = 0; i < metadata_pages_count; ++i)
			mark_block_in_lookup_table(res, i, 1);
	}

	return res;
}

FsDescriptors init_fs(const char* filename, int size) {
	if (size < sizeof(SuperBlock))
		size = sizeof(SuperBlock);
	int page_size = get_page_size();
	int pages_count = (size + page_size - 1) / page_size;
	size = pages_count * page_size;

	void* container = open_and_map(filename, size);
	SuperBlock* superblock = (SuperBlock*) container;
	superblock->size = page_size;
	superblock->total_block_count = pages_count;
	*superblock = get_superblock_params();

	FsDescriptors fs = prepare_descriptors(container, 1);
	int root_inode = init_new_file(fs, FLG_DIRECTORY);
	if (root_inode != fs.root_inode) {
		die_fatal("Internal error (init_fs)");
	}
	return fs;
}

struct FsDescriptors open_fs(const char* filename) {
	void* container = open_and_map(filename, 0);
	return prepare_descriptors(container, 0);
}

void close_fs(struct FsDescriptors fs) {
	munmap(fs.container, fs.superblock->total_block_count * fs.superblock->size);
	die("Error occured while saving data");
}

int init_new_file(struct FsDescriptors fs, const unsigned char flags) {
	int block_index = get_next_free_block_index(fs);
	mark_block_in_lookup_table(fs, block_index, 1);
	void* block = fs.container + block_index * fs.superblock->size;
    struct FirstINode* inode = (struct FirstINode*) block;
	inode->size = 0;
	inode->node.first_file_block = 0;
	inode->last_inode = block_index;
	inode->flags = flags;
	return block_index;
}

struct INode* get_inode(struct FsDescriptors fs, const int block_index) {
	if (!block_index)
		return NULL;
	if (block_index >= fs.superblock->total_block_count)
		die_fatal("Internal error (get_inode)");

	void* block = fs.container + block_index * fs.superblock->size;
	return (struct INode*) block;
}

struct INode* get_next_inode(struct FsDescriptors fs, INode* inode) {
	return get_inode(fs, inode->first_file_block);
}

void purge_blocks(struct FsDescriptors fs, int block_index) {
    struct INode* inode = get_inode(fs, block_index);
	while (inode) {
		mark_block_in_lookup_table(fs, block_index, 0);
		block_index = inode->first_file_block;
		inode = get_next_inode(fs, inode);
	}
}

void purge_file(struct FsDescriptors fs, int block_index) {
    struct FirstINode* inode = (struct FirstINode*) get_inode(fs, block_index);
	if (inode->flags & FLG_DIRECTORY) {
        struct DirectoryContent content = read_directory(fs, block_index);
		for (int i = 0; i < content.items_count; ++i) {
			purge_file(fs, content.items[i].inode);
		}
		free_directory(content);
	}

	purge_blocks(fs, block_index);
}

int get_blocks_required(struct FsDescriptors fs, int size) {
	const int first_size = fs.superblock->size - sizeof(struct FirstINode);
	const int common_size = fs.superblock->size - sizeof(struct INode);
	if (size <= first_size)
		return 1;
	return (size - first_size + common_size - 1) / common_size + 1;
}

void truncate_file(struct FsDescriptors fs, int block_index, int new_size) {
    struct FirstINode* inode = (struct FirstINode*) get_inode(fs, block_index);

	const int old_size = inode->size;
	const int old_blocks_required = get_blocks_required(fs, old_size);
	const int new_blocks_required = get_blocks_required(fs, new_size);

	inode->size = new_size;

	if (new_blocks_required > old_blocks_required) {
		block_index = inode->last_inode;
		for (int i = old_blocks_required; i < new_blocks_required; ++i) {
			int next_block = get_next_free_block_index(fs);
			mark_block_in_lookup_table(fs, next_block, 1);
            struct INode* current_inode = get_inode(fs, block_index);
			current_inode->first_file_block = next_block;
			block_index = next_block;
		}
		inode->last_inode = block_index;
	} else if (new_blocks_required < old_blocks_required) {
        struct INode* current_inode = (struct INode*) inode;
		for (int i = 1; i < new_blocks_required; ++i) {
			block_index = current_inode->first_file_block;
			current_inode = get_next_inode(fs, current_inode);
		}
		purge_blocks(fs, current_inode->first_file_block);
		inode->last_inode = block_index;
	}
}

int read_file(FsDescriptors fs, int block_index, int offset, int length, void* buffer) {
	if (!length)
		return 0;

	FirstINode* inode = (FirstINode*) get_inode(fs, block_index);
	const int size = inode->size;
	if (offset == size)
		return 0;
	if (offset > size)
		die_fatal("Invalid position in file");
	if (offset + length > size)
		length = size - offset;

	const int first_size = fs.superblock->size - sizeof(struct FirstINode);
	const int common_size = fs.superblock->size - sizeof(struct INode);
	int block_inode_offset = sizeof(struct FirstINode);
    struct INode* current_inode = (struct INode*) inode;

	if (offset > first_size) {
		offset -= first_size;
		block_index = current_inode->first_file_block;
		current_inode = get_next_inode(fs, current_inode);
		block_inode_offset = sizeof(struct INode);
	}

	while (offset > common_size) {
		offset -= common_size;
		block_index = current_inode->first_file_block;
		current_inode = get_next_inode(fs, current_inode);
	}

	int len = length;
	while (len) {
		const int available_len = fs.superblock->size - block_inode_offset - offset;
		const int copy_len = MIN(available_len, len);

		memcpy(buffer, fs.container + block_index * fs.superblock->size + offset + block_inode_offset, copy_len);

		block_inode_offset = sizeof(struct INode);
		offset = 0;
		len -= copy_len;
		buffer += copy_len;

		if (len) {
			block_index = current_inode->first_file_block;
			current_inode = get_next_inode(fs, current_inode);
		}
	}

	return length;
}

int read_entire_file(struct FsDescriptors fs, int block_index, void* buffer) {
    struct FirstINode* inode = (struct FirstINode*) get_inode(fs, block_index);
	return read_file(fs, block_index, 0, inode->size, buffer);
}

void write_file_unchecked(struct FsDescriptors fs, int block_index, int offset, int length, void* buffer) {
	if (!length)
		return;

    struct FirstINode* inode = (struct FirstINode*) get_inode(fs, block_index);
	const int size = inode->size;
	if (offset > size)
		die_fatal("Invalid position in file");

	if (length + offset > size)
		truncate_file(fs, block_index, length + offset);

	const int first_size = fs.superblock->size - sizeof(struct FirstINode);
	const int common_size = fs.superblock->size - sizeof(struct INode);

	int block_inode_offset = sizeof(struct FirstINode);
	INode* current_inode = (struct INode*) inode;

	if (offset > first_size) {
		offset -= first_size;
		block_index = current_inode->first_file_block;
		current_inode = get_next_inode(fs, current_inode);
		block_inode_offset = sizeof(struct INode);
	}

	while (offset > common_size) {
		offset -= common_size;
		block_index = current_inode->first_file_block;
		current_inode = get_next_inode(fs, current_inode);
	}

	while (length) {
		const int available_len = fs.superblock->size - block_inode_offset - offset;
		const int copy_len = MIN(available_len, length);

		memcpy(fs.container + block_index * fs.superblock->size + offset + block_inode_offset, buffer, copy_len);

		block_inode_offset = sizeof(struct INode);
		offset = 0;
		length -= copy_len;
		buffer += copy_len;

		if (length) {
			block_index = current_inode->first_file_block;
			current_inode = get_next_inode(fs, current_inode);
		}
	}
}

void write_file(FsDescriptors fs, int block_index, int offset, int length, void* buffer) {
	FirstINode* inode = (FirstINode*) get_inode(fs, block_index);
	if (!(inode->flags & FLG_FILE))
		die_fatal("It is not file");
	write_file_unchecked(fs, block_index, offset, length, buffer);
}

void append_file_unchecked(FsDescriptors fs, int block_index, int length, void* buffer) {
	FirstINode* inode = (FirstINode*) get_inode(fs, block_index);
	write_file_unchecked(fs, block_index, inode->size, length, buffer);
}

void append_file(FsDescriptors fs, int block_index, int length, void* buffer) {
	FirstINode* inode = (FirstINode*) get_inode(fs, block_index);
	write_file(fs, block_index, inode->size, length, buffer);
}

DirectoryContent read_directory(FsDescriptors fs, int dir_block_index) {
	FirstINode* inode = (FirstINode*) get_inode(fs, dir_block_index);
	if (!(inode->flags & FLG_DIRECTORY))
		die_fatal("It is not directory");

	const int cur_dir_size = inode->size;
	if (cur_dir_size % sizeof(Directory))
		die_fatal("Internal error (read_directory)");

	void* buffer = malloc(cur_dir_size);
	read_file(fs, dir_block_index, 0, cur_dir_size, buffer);

	return (DirectoryContent) {
		cur_dir_size / sizeof(Directory),
		(Directory*) buffer
	};
}

void free_directory(DirectoryContent dir) {
	free(dir.items);
}

void append_directory(FsDescriptors fs, int dir_block_index, Directory item) {
	FirstINode* inode = (FirstINode*) get_inode(fs, dir_block_index);
	if (!(inode->flags & FLG_DIRECTORY))
		die_fatal("It is not directory");
	append_file_unchecked(fs, dir_block_index, sizeof(Directory), &item);
}

int find_directory_item(DirectoryContent content, const char* name) {
	for (int i = 0; i < content.items_count; ++i) {
		if (strcmp(name, content.items[i].name) == 0)
			return i;
	}
	die_fatal("No such file or directory");
}

int locate_path(FsDescriptors fs, Path path) {
	int next_inode = fs.root_inode;
	for (int i = 0; i < path.count; ++i) {
		DirectoryContent root = read_directory(fs, next_inode);
		int item_index = find_directory_item(root, path.parts[i]);
		Directory item = root.items[item_index];
		next_inode = item.inode;
		free_directory(root);
	}
	return next_inode;
}

int remove_from_directory(FsDescriptors fs, int dir_block_index, const char* name) {
	DirectoryContent root = read_directory(fs, dir_block_index);
	int item_index = find_directory_item(root, name);

	truncate_file(fs, dir_block_index, 0);
	for (int i = 0; i < root.items_count; ++i)
		if (i != item_index)
			append_directory(fs, dir_block_index, root.items[i]);

	free_directory(root);
	return item_index;
}

int get_file_size(FsDescriptors fs, int block_index) {
	FirstINode* inode = (FirstINode*) get_inode(fs, block_index);
	return inode->size;
}

int* trace_file_blocks(FsDescriptors fs, int block_index) {
	INode* inode = get_inode(fs, block_index);
	int count = get_blocks_required(fs, ((FirstINode*) inode)->size);
	int* res = calloc(count, sizeof(int));
	int* p = res;
	*(p++) = block_index;
	do {
		*(p++) = inode->first_file_block;
		inode = get_next_inode(fs, inode);
	} while (--count);
	return res;
}
