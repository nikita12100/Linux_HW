#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_filesystem.h"


int main(int argc, char **argv) {
    char *help_text = "File system commands:\n"
                      " create\n"
                      " dump\n"
                      " list\n"
                      " push\n"
                      " pull\n"
                      " remove\n"
                      " delete\n"
                      " help\n"
                      " exit\n";

    printf("%s", help_text);

    char fs_name[1024] = "(none)";
    struct file_system *v;

    while (1) {
        printf("\n%s> ", fs_name);
        char command[1024];
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = 0;
        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "help") == 0) {
            printf("%s", help_text);
        } else if (strcmp("create", command) == 0) {
            printf("\nEnter file system name>");
            fgets(command, 1024, stdin);
            command[strlen(command) - 1] = 0;
            strcpy(fs_name, command);

            printf("\nEnter file system size(>=24)>");
            fgets(command, 1024, stdin);
            size_t size = atoi(command);
            v = create_fs(fs_name, size);
            if (!v) {
                printf("Error: cannot create virtual file system.\n");
                return 2;
            }
            unmount_fs(v);
            printf("%s created in bytes %zu\n", fs_name, size);
        } else if (strcmp("dump", command) == 0) {
            v = mount_fs(fs_name);
            if (!v) {
                printf("Error: cannot open virtual file system.\n");
                return 2;
            }

            dump_fs(v);

            unmount_fs(v);

        } else if (strcmp("list", command) == 0) {
            v = mount_fs(fs_name);
            if (!v) {
                printf("Error: cannot open virtual file system.\n");
                return 2;
            }

            list_files_fs(v);

            unmount_fs(v);

        } else if (strcmp("push", command) == 0) {
            v = mount_fs(fs_name);
            if (!v) {
                printf("Error: cannot open virtual file system.\n");
                return 2;
            }

            char source_file_name[1024];
            printf("\nEnter source file name>");
            fgets(source_file_name, 1024, stdin);
            source_file_name[strlen(source_file_name) - 1] = 0;

            char destination_file_name[1024];
            printf("\nEnter destination file name>");
            fgets(destination_file_name, 1024, stdin);
            destination_file_name[strlen(destination_file_name) - 1] = 0;

            printf("Pushing new file, result: %d\n", copy_to_fs(v, source_file_name, destination_file_name));

            unmount_fs(v);

        } else if (strcmp("pull", command) == 0) {

            v = mount_fs(fs_name);
            if (!v) {
                printf("Error: cannot open virtual file system.\n");
                return 2;
            }

            char source_file_name[1024];
            printf("\nEnter source file name>");
            fgets(source_file_name, 1024, stdin);
            source_file_name[strlen(source_file_name) - 1] = 0;

            char destination_file_name[1024];
            printf("\nEnter destination file name>");
            fgets(destination_file_name, 1024, stdin);
            destination_file_name[strlen(destination_file_name) - 1] = 0;


            printf("Downloading file, result: %d\n", copy_from_fs(v, source_file_name, destination_file_name));

            unmount_fs(v);

        } else if (strcmp("remove", command) == 0) {
            v = mount_fs(fs_name);
            if (!v) {
                printf("Error: cannot open virtual file system.\n");
                return 2;
            }

            char deleted_file_name[1024];
            printf("\nEnter deleted file name>");
            fgets(deleted_file_name, 1024, stdin);
            deleted_file_name[strlen(deleted_file_name) - 1] = 0;

            delete_file_fs(v, deleted_file_name);

            unmount_fs(v);

        } else if (strcmp("delete", command) == 0) {
            delete_fs(fs_name);
            strcpy(fs_name, "(none)");

        } else {
            printf("invalid command `%s`\n", command);
        }

    }

    return 0;
}
