#include <fs.h>

SuperBlock get_superblock_params() {
	return (SuperBlock) {
		.int_size = sizeof(int),
		.superblock_type_size = sizeof(SuperBlock),
		.inode_type_size = sizeof(INode),
		.inode_type_size = sizeof(FirstINode),
		.directory_size = sizeof(Directory)
	};
}

char check_types_size(const SuperBlock vals) {
    SuperBlock mine = get_superblock_params();
	return mine.int_size == vals.int_size &&
			mine.superblock_type_size == vals.superblock_type_size &&
			mine.inode_type_size == vals.inode_type_size &&
			mine.inode_type_size == vals.inode_type_size &&
			mine.directory_size == vals.directory_size;
}
