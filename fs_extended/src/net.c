#include <net.h>
#include <manage.h>
#include <sys/socket.h>
#include <stdatomic.h>
#include <myutil.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <net.h>
#include <myutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>



char safe_read(int fd, void* buf, int len, void (*error_handler) (const char*)) {
    int ready = 0;
    while (ready < len) {
        uint actually_got = read(fd, buf + ready, len - ready);
        char err = errno != 0;
        error_handler("IO error");
        if (err || !actually_got)
            return 0;
        ready += actually_got;
    }
    return 1;
}

char safe_write(int fd, void* buf, int len, void (*error_handler) (const char*)) {
    int ready = 0;
    while (ready < len) {
        uint actually_put = write(fd, buf + ready, len - ready);
        char err = errno != 0;
        error_handler("IO error");
        if (err)
            return 0;
        ready += actually_put;
    }
    return 1;
}


__thread int thread_current_sockd;
char error_intercepted;
NetServerDescriptors global_nds;

void fsop_error(const char *s) {
    if (!error_intercepted) {
        error_intercepted = 1;
        String str = to_string(s);
        if (!slz_String_write(thread_current_sockd, &str)) {
            shutdown(thread_current_sockd, SHUT_RDWR);
        }
        free_string(str);
    }

    pthread_mutex_unlock(global_nds.mutex);
    pthread_exit(NULL);
}


void sighandler(int sigid) {
    if (thread_current_sockd > 0)
        close(thread_current_sockd);

    pthread_mutex_unlock(global_nds.mutex);
    pthread_exit(NULL);
}

void setup_sigsegv_catcher() {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = sighandler;
    sigaction(SIGSEGV, NULL, &act);
}


NetServerDescriptors initialize_server(int port) {
    NetServerDescriptors nds;
    nds.sockd = socket(AF_INET, SOCK_STREAM, 0);
    die("Unable to create socket");

    nds.addr.sin_family = AF_INET;
    nds.addr.sin_port = htons(port);
    nds.addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(nds.sockd, (struct sockaddr *) &nds.addr, sizeof(nds.addr));
    die("Unable to bind socket");

    nds.termination_flag = malloc(sizeof(atomic_bool));
    die("Memory allocation error");
    atomic_init(nds.termination_flag, 0);

    nds.mutex = malloc(sizeof(pthread_mutex_t));
    die("Memory allocation error");
    pthread_mutex_init(nds.mutex, NULL);

    return nds;
}

void server_destroy(NetServerDescriptors nds) {
    close(nds.sockd);
    free(nds.termination_flag);
    pthread_mutex_destroy(nds.mutex);
    free(nds.mutex);
}


struct ServeClientRequest {
    int sockd;
    FsDescriptors fds;
    pthread_mutex_t *mutex;
};

void write_empty_error(int sockd) {
    if (!error_intercepted) {
        String empty = to_string("");
        if (!slz_String_write(sockd, &empty)) {
            syslog(LOG_WARNING, "Connection problems");
            shutdown(sockd, SHUT_RDWR);
        }
        free_string(empty);
    }
}

void process_operation(int sockd, FsDescriptors fds, NetFsOperation *op) {
    if ((op)->type == FSOP_Read) {
        Path path = split_path((const char *) &(op)->Read_args.path.string);
        int inode = locate_path(fds, path);

        char* data = malloc((unsigned long) &(op)->Read_args.length);
        die("Memory allocation failed");

        int read = read_file(fds, inode, (int) &(op)->Read_args.offset, (int) &(op)->Read_args.length, data);
        String result = {read, data};

        write_empty_error(sockd);
        if (!slz_String_write(sockd, &result)) {
            syslog(LOG_WARNING, "Connection problems");
            shutdown(sockd, SHUT_RDWR);
        }

        free_string(result);
        free_path(path);
    }
    if ((op)->type == FSOP_Write) {
        Path path = split_path((const char *) &(op)->Write_args.path.string);
        int inode = locate_path(fds, path);

        write_file(fds, inode, (int) &(op)->Write_args.offset, (int) &(op)->Write_args.length, &(op)->Write_args.data);

        write_empty_error(sockd);
        free_path(path);
    }
    if ((op)->type == FSOP_Create) {
        Path path = split_path(&(op)->Create_args.path.string);

        --path.count;
        int dir_inode = locate_path(fds, path);
        const char* name = path.parts[path.count];
        ++path.count;

        if (strlen(name) >= FILE_NAME_LENGTH)
            die_fatal("Too long file name");

        int file_inode = init_new_file(fds, &(op)->Create_args.flags);
        Directory item = {.inode = file_inode};
        for (int i = 0; *name; ++name)
            item.name[i++] = *name;
        append_directory(fds, dir_inode, item);

        write_empty_error(sockd);

        free_path(path);
    }
    if ((op)->type == FSOP_Remove) {
        Path path = split_path(&(op)->Remove_args.path.string);

        --path.count;
        int dir_inode = locate_path(fds, path);
        const char* name = path.parts[path.count];
        ++path.count;

        int file_inode = remove_from_directory(fds, dir_inode, name);
        purge_file(fds, file_inode);

        write_empty_error(sockd);

        free_path(path);
    }
    if ((op)->type == FSOP_Stat) {
        Path path = split_path(&(op)->Stat_args.path.string);

        int inode = locate_path(fds, path);
        int size = get_file_size(fds, inode);
        int blocks = get_blocks_required(fds, size);

        FsOpStat_response response;

        response.size = size;
        response.blocks_count = blocks;
        response.blocks = trace_file_blocks(fds, inode);

        write_empty_error(sockd);
        if (!slz_FsOpStatResponse_write(sockd, &response)) {
            syslog(LOG_WARNING, "Connection problems");
            shutdown(sockd, SHUT_RDWR);
        }

        free((void*) response.blocks);
        free_path(path);
    }
    if ((op)->type == FSOP_ReadDir) {
        Path path = split_path((const char *) &(op)->ReadDir_args.path.string);

        int dir_inode = locate_path(fds, path);
        DirectoryContent content = read_directory(fds, dir_inode);

        write_empty_error(sockd);
        if (!slz_DirectoryContent_write(sockd, &content)) {
            syslog(LOG_WARNING, "Connection problems");
            shutdown(sockd, SHUT_RDWR);
        }

        free_directory(content);
        free_path(path);
    }

}

bool serve_next_operation(struct ServeClientRequest *req) {
    NetFsOperation op;
    if (!slz_NetFsOperation_read(req->sockd, &op)) {
        return 0;
    }

    pthread_mutex_lock(req->mutex);
    error_intercepted = 0;
    intercept_errors(fsop_error);
    process_operation(req->sockd, req->fds, &op);
    intercept_errors(NULL);
    pthread_mutex_unlock(req->mutex);

    if ((op).type == FSOP_Write) {
        free((op).Write_args.data);
    }
    if((op).type==FSOP_Read){
        free_string((op).Read_args.path);
    }
    if((op).type==FSOP_Write){
        free_string((op).Write_args.path);
    }
    if((op).type==FSOP_Create){
        free_string((op).Create_args.path);
    }
    if((op).type==FSOP_Remove){
        free_string((op).Remove_args.path);
    }
    if((op).type==FSOP_Stat){
        free_string((op).Stat_args.path);
    }
    if((op).type==FSOP_ReadDir){
        free_string((op).ReadDir_args.path);
    }
    return 1;
}

void *serve_client(void *args) {
    struct ServeClientRequest *req = args;
    thread_current_sockd = req->sockd;

    while (serve_next_operation(req));

    close(req->sockd);
    free(args);
    return NULL;
}

void server_listen_connections(NetServerDescriptors nds, FsDescriptors fds) {
    global_nds = nds;
    setup_sigsegv_catcher();

    listen(nds.sockd, 5);
    die("Socket error");

    while (!atomic_load(nds.termination_flag)) {
        int fd = accept(nds.sockd, NULL, 0);
        if (warn("Problem accepting client"))
            continue;

        struct ServeClientRequest *req = malloc(sizeof(struct ServeClientRequest));
        die("Memory allocation error");
        req->sockd = fd;
        req->fds = fds;
        req->mutex = nds.mutex;

        pthread_t thr;
        pthread_create(&thr, NULL, serve_client, req);
        pthread_detach(thr);
    }
}


char* split_address(const char* addr, int* port) {
    char* pos = strchr(addr, ':');
    if (!pos) {
        die_fatal("Invalid address");
    }
    if (!sscanf(pos + 1, "%d", port)) {
        die_fatal("Invalid address");
    }

    char* host = malloc(pos - addr + 1);
    die("Memory allocation failed");

    memcpy(host, addr, pos - addr);
    *(host + (pos - addr)) = 0;

    return host;
}

int initialize_client(const char* address) {
    int port;
    char* host = split_address(address, &port);

    int sockd = socket(AF_INET, SOCK_STREAM, 0);
    die("Socket error");

    struct hostent* hostent = gethostbyname(host);
    in_addr_t* ip = (in_addr_t*) (*hostent->h_addr_list);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr = *ip;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    connect(sockd, (const struct sockaddr*) &addr, sizeof(struct sockaddr_in));
    die("Socket error");

    free(host);

    return sockd;
}

void destroy_client(int sockd) {
    close(sockd);
}

