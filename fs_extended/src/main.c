#include <manage.h>
#include <net.h>
#include <myutil.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <signal.h>


void check_socket_status(int sockd) {
    String err;
    if (!slz_String_read(sockd, &err)) {
        die_fatal("Connection problems");
    }

    if (err.length - 1) {
        die_fatal(err.string);
    }

    free_string(err);
}

int try_read(int sockd, const char *path, int offset, int length, char *buffer) {
    NetFsOperation op;
    op.type = FSOP_Read;
    op.Read_args.path = to_string(path);
    op.Read_args.length = length;
    op.Read_args.offset = offset;

    if (!slz_NetFsOperation_write(sockd, &op)) {
        die_fatal("Connection problems");
    }

    free_string(op.Read_args.path);

    check_socket_status(sockd);

    String response;
    if (!slz_String_read(sockd, &response)) {
        die_fatal("Connection problems");
    }

    memcpy(buffer, response.string, response.length);
    int real_len = response.length;
    free_string(response);
    return real_len;
}

void create(int sockd, const char *s_path, unsigned char flags) {
    NetFsOperation op;
    op.type = FSOP_Create;
    op.Create_args.path = to_string(s_path);
    op.Create_args.flags = flags;
    if (!slz_NetFsOperation_write(sockd, &op)) {
        die_fatal("Connection problems");
    }
    if ((op).type == FSOP_Write) {
        free((op).Write_args.data);
    }
    if ((op).type == FSOP_Read) {
        free_string((op).Read_args.path);
    }
    if ((op).type == FSOP_Write) {
        free_string((op).Write_args.path);
    }
    if ((op).type == FSOP_Create) {
        free_string((op).Create_args.path);
    }
    if ((op).type == FSOP_Remove) {
        free_string((op).Remove_args.path);
    }
    if ((op).type == FSOP_Stat) {
        free_string((op).Stat_args.path);
    }
    if ((op).type == FSOP_ReadDir) {
        free_string((op).ReadDir_args.path);
    }


    check_socket_status(sockd);
}

int main(int argc, char **argv) {
    char *help_text = "Example backend command:\n"
                      "./my_fs backend run (port) (path_to_fs)\n"
                      "./my_fs backend init (path_to_fs) (size)\n\n"
                      "Example client command:\n"
                      "./my_fs client (address) (command) (arguments) \n"
                      "Where example of commands with arguments: \n"
                      "  mkdir (path) \n"
                      "  mkfile (path) \n"
                      "  write (path) \n"
                      "  rm (path) \n"
                      "  ls (path) \n"
                      "  cat (path) \n";

    printf("%s\n", help_text);

//    if (argc != 5) {
//        printf("%s\n", help_text);
//        exit(0);
//    }

    if (!strcasecmp(argv[1], "backend")) {

        if (!strcasecmp(argv[2], "init")) {
            unsigned long long size;
            if (!sscanf(argv[4], "%llu", &size)) {
                die_fatal("Invalid argument");
            }

            FsDescriptors fs = init_fs(argv[3], size);
            printf("Ready: %lu blocks of %lu bytes\n", fs.superblock->total_block_count, fs.superblock->size);
            close_fs(fs);
            return 0;
        }

        if (!strcasecmp(argv[2], "run")) {
            int port;
            if (!sscanf(argv[3], "%d", &port)) {
                die_fatal("Invalid argument");
            }

            FsDescriptors descriptors = open_fs(argv[4]);
            NetServerDescriptors net = initialize_server(port);

            printf("Initialization succeeded. Daemonization...");

            daemon(0, 0);
            die("Daemonization failed");

            struct sigaction act;
            memset(&act, 0, sizeof(struct sigaction));
            act.sa_handler = SIG_IGN;
            act.sa_flags = 0;
            sigaction(SIGABRT, NULL, &act);

            openlog("rfsdaemon", LOG_PID, LOG_DAEMON);
            configure_error_logging(0, 1);

            server_listen_connections(net, descriptors);

            close_fs(descriptors);
            server_destroy(net);
            closelog();
            return 0;
        }
    }

    if (!strcasecmp(argv[1], "client")) {
        if (strcasecmp("mkfile", argv[3]) == 0) {
            int sockd = initialize_client(argv[2]);
            create(sockd, argv[4], FLG_FILE);
            destroy_client(sockd);
        }
        if (strcasecmp("mkdir", argv[3]) == 0) {
            int sockd = initialize_client(argv[2]);
            create(sockd, argv[4], FLG_DIRECTORY);
            destroy_client(sockd);
        }
        if (strcasecmp("ls", argv[3]) == 0) {
            NetFsOperation op;
            op.type = FSOP_ReadDir;
            op.ReadDir_args.path = to_string(argv[4]);

            int sockd = initialize_client(argv[2]);
            if (!slz_NetFsOperation_write(sockd, &op))
                die_fatal("Connection problems");

            check_socket_status(sockd);
            DirectoryContent content;
            if (!slz_DirectoryContent_read(sockd, &content)) {
                die_fatal("Connection problems");
            }

            for (int i = 0; i < content.items_count; ++i) {
                printf("%lu %s\n", content.items[i].inode, content.items[i].name);
            }

            free_directory(content);
            destroy_client(sockd);
        }
        if (strcasecmp("cat", argv[3]) == 0) {
            int sockd = initialize_client(argv[2]);
            const int buf_size = 1024 * 1024 * 2;
            void *buffer = malloc(buf_size);
            int read_bytes;
            int total_read_bytes = 0;

            while (read_bytes = try_read(sockd, argv[4], total_read_bytes, buf_size, buffer)) {
                fwrite(buffer, 1, read_bytes, stdout);
                total_read_bytes += read_bytes;
            }

            free(buffer);
            destroy_client(sockd);
        }
        if (strcasecmp("write", argv[3]) == 0) {
            int sockd = initialize_client(argv[2]);
            int offset = 0;
            NetFsOperation op;
            op.type = FSOP_Write;
            op.Write_args.path = to_string(argv[4]);

            const int buf_size = 1024 * 1024 * 2;
            void *buffer = malloc(buf_size);
            op.Write_args.data = buffer;

            while (!feof(stdin)) {
                int read_bytes = fread(buffer, 1, buf_size, stdin);
                if (ferror(stdin)) {
                    die_fatal("Error occurred while reading from input stream");
                }

                op.Write_args.offset = offset;
                op.Write_args.length = read_bytes;
                if (!slz_NetFsOperation_write(sockd, &op)) {
                    die_fatal("Connection problems");
                }
                check_socket_status(sockd);

                offset += read_bytes;
            }

            free(buffer);
            free_string(op.Write_args.path);
            destroy_client(sockd);
        }
        if (strcasecmp("rm", argv[3]) == 0) {
            NetFsOperation op;
            op.type = FSOP_Remove;
            op.Remove_args.path = to_string(argv[4]);

            int sockd = initialize_client(argv[2]);
            if (!slz_NetFsOperation_write(sockd, &op))
                die_fatal("Connection problems");

            check_socket_status(sockd);

            destroy_client(sockd);
        }
    }

    exit(0);
}
