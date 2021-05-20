#pragma once

#include <manage.h>
#include <myutil.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <string.h>



#define _if0(...)
#define _if1(...) __VA_ARGS__
#define _if(val, ...) _if ## val(__VA_ARGS__)
#define _ifnot0(...) __VA_ARGS__
#define _ifnot1(...)
#define _ifnot(val, ...) _ifnot ## val(__VA_ARGS__)

#define _SLZ_ARRAY_READER_IMPL(field, count_field, struct_size, error_handler) {\
    if (!safe_read(fd, &(count_field), sizeof(count_field), error_handler)) return 0;\
    if (count_field) {\
        field = malloc(struct_size * (count_field));\
        error_handler("Memory allocation failed");\
        if (!(field)) return 0;\
        if (!safe_read(fd, (field), struct_size * (count_field), error_handler)) return 0;\
    } else {\
        field = NULL;\
    }\
}

#define _SLZ_ARRAY_WRITER_IMPL(field, count_field, struct_size, error_handler) {\
    if (!(field) && (count_field)) return 0;\
    if (!safe_write(fd, &(count_field), sizeof(count_field), error_handler)) return 0;\
    if (count_field)\
        if (!safe_write(fd, (field), struct_size * (count_field), error_handler)) return 0;\
}


#define _SLZ_ARRAY(field_name, count_field_name, elem_type, error_handler) {\
    if (!strcmp("read", context)) {\
        _SLZ_ARRAY_READER_IMPL(value->field_name, value->count_field_name, sizeof(elem_type), error_handler)\
    } else if (!strcmp("write", context)) {\
        _SLZ_ARRAY_WRITER_IMPL(value->field_name, value->count_field_name, sizeof(elem_type), error_handler)\
    }\
}

#define _SLZ_SERIALIZER_READER_IMPL(type_name, value, error_handler, raw, ...) \
    _ifnot(raw, {\
        if (!safe_read(fd, value, sizeof(type_name), error_handler)) return 0;\
    })\
    __VA_ARGS__

#define _SLZ_SERIALIZER_READER(type_name, func_name, error_handler, raw, body, ...) char func_name(int fd, type_name* value)\
_if(body, {\
    const char* context = "read";\
    _SLZ_SERIALIZER_READER_IMPL(type_name, value, error_handler, raw, __VA_ARGS__)\
    return 1;\
});

#define _SLZ_SERIALIZER_WRITER_IMPL(type_name, value, error_handler, raw, ...) \
    _ifnot(raw, {\
        if (!safe_write(fd, value, sizeof(type_name), error_handler)) return 0;\
    })\
    __VA_ARGS__

#define _SLZ_SERIALIZER_WRITER(type_name, func_name, error_handler, raw, body, ...) char func_name(int fd, type_name* value)\
_if(body, {\
    const char* context = "write";\
    _SLZ_SERIALIZER_WRITER_IMPL(type_name, value, error_handler, raw, __VA_ARGS__)\
    return 1;\
});

#define _SLZ_FIELD(field_type_name, field_name, error_handler, ...) {\
    if (!strcmp("read", context)) {\
        _SLZ_SERIALIZER_READER_IMPL(field_type_name, &value->field_name, error_handler, 0, __VA_ARGS__)\
    } else if (!strcmp("write", context)) {\
        _SLZ_SERIALIZER_WRITER_IMPL(field_type_name, &value->field_name, error_handler, 0, __VA_ARGS__)\
    }\
}


#define _SLZ_SERIALIZER(type_name, func_name_prefix, error_handler, ...) \
_SLZ_SERIALIZER_READER(type_name, func_name_prefix ## _read, error_handler, 0, 1, __VA_ARGS__)\
_SLZ_SERIALIZER_WRITER(type_name, func_name_prefix ## _write, error_handler, 0, 1, __VA_ARGS__)

#define _SLZ_SERIALIZER_RAW(type_name, func_name_prefix, ...) \
_SLZ_SERIALIZER_READER(type_name, func_name_prefix ## _read, _unused, 1, 1, __VA_ARGS__)\
_SLZ_SERIALIZER_WRITER(type_name, func_name_prefix ## _write, _unused, 1, 1, __VA_ARGS__)

#define _SLZ_SERIALIZER_DEF(type_name, func_name_prefix) \
_SLZ_SERIALIZER_READER(type_name, func_name_prefix ## _read, _unused, _unused, 0)\
_SLZ_SERIALIZER_WRITER(type_name, func_name_prefix ## _write, _unused, _unused, 0)



char safe_read(int fd, void* buf, int len, void (*error_handler) (const char*));
char safe_write(int fd, void* buf, int len, void (*error_handler) (const char*));



_SLZ_SERIALIZER_DEF(String, slz_String)

typedef struct NetServerDescriptors {
    struct sockaddr_in addr;
    int sockd;
    atomic_bool* termination_flag;
    pthread_mutex_t* mutex;
} NetServerDescriptors;

enum FsOpType {
    FSOP_Read,
    FSOP_Write,
    FSOP_Create,
    FSOP_Remove,
    FSOP_Stat,
    FSOP_ReadDir
};


typedef struct FsOpRead {
    String path;
    int offset;
    int length;
} FsOpRead_args;

typedef struct FsOpWrite {
    String path;
    int offset;
    int length;
    char* data;
} FsOpWrite_args;

typedef struct FsOpCreate {
    String path;
    unsigned char flags;
} FsOpCreate_args;

typedef struct FsOpRemove {
    String path;
} FsOpRemove_args;

typedef struct FsOpStat {
    String path;
} FsOpStat_args;

typedef struct FsOpReadDir {
    String path;
} FsOpReadDir_args;


typedef struct NetFsOperation {
    enum FsOpType type;
    union {
        FsOpRead_args Read_args;
        FsOpWrite_args Write_args;
        FsOpCreate_args Create_args;
        FsOpRemove_args Remove_args;
        FsOpStat_args Stat_args;
        FsOpReadDir_args ReadDir_args;
    };
} NetFsOperation;

typedef struct FsOpStatResponse {
    int size;
    int blocks_count;
    int* blocks;
} FsOpStat_response;
_SLZ_SERIALIZER_DEF(FsOpStat_response, slz_FsOpStatResponse)

_SLZ_SERIALIZER_DEF(DirectoryContent, slz_DirectoryContent)


_SLZ_SERIALIZER_DEF(FsOpRead_args, slz_FsOpRead);
_SLZ_SERIALIZER_DEF(FsOpWrite_args, slz_FsOpWrite);
_SLZ_SERIALIZER_DEF(FsOpCreate_args, slz_FsOpCreate);
_SLZ_SERIALIZER_DEF(FsOpRemove_args, slz_FsOpRemove);
_SLZ_SERIALIZER_DEF(FsOpStat_args, slz_FsOpStat);
_SLZ_SERIALIZER_DEF(FsOpReadDir_args, slz_FsOpReadDir);
_SLZ_SERIALIZER_DEF(NetFsOperation, slz_NetFsOperation)

NetServerDescriptors initialize_server(int port);
void server_listen_connections(NetServerDescriptors, FsDescriptors);
void server_destroy(NetServerDescriptors);


int initialize_client(const char* address);
void destroy_client(int);
