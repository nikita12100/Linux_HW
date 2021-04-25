#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <net.h>
#include <manage.h>

void catch(const char* s) {
    warn(s);
}

_SLZ_SERIALIZER_RAW(NetFsOperation, slz_NetFsOperation, {
    _SLZ_FIELD(enum FsOpType, type, catch);
    if (!strcmp("read", context)) {
        if (value->type == FSOP_Read) {
            slz_FsOpRead_read(fd, &value->Read_args);
        }
        if (value->type == FSOP_Write) {
            slz_FsOpWrite_read(fd, &value->Write_args);
        }
        if (value->type == FSOP_Create) {
            slz_FsOpCreate_read(fd, &value->Create_args);
        }
        if (value->type == FSOP_Remove) {
            slz_FsOpRemove_read(fd, &value->Remove_args);
        }
        if (value->type == FSOP_Stat) {
            slz_FsOpStat_read(fd, &value->Stat_args);
        }
        if (value->type == FSOP_ReadDir) {
            slz_FsOpReadDir_read(fd, &value->ReadDir_args);
        }
    } else if (!strcmp("write", context)) {
        if (value->type == FSOP_Read) {
            slz_FsOpRead_write(fd, &value->Read_args);
        }
        if (value->type == FSOP_Write) {
            slz_FsOpWrite_write(fd, &value->Write_args);
        }
        if (value->type == FSOP_Create) {
            slz_FsOpCreate_write(fd, &value->Create_args);
        }
        if (value->type == FSOP_Remove) {
            slz_FsOpRemove_write(fd, &value->Remove_args);
        }
        if (value->type == FSOP_Stat) {
            slz_FsOpStat_write(fd, &value->Stat_args);
        }
        if (value->type == FSOP_ReadDir) {
            slz_FsOpReadDir_write(fd, &value->ReadDir_args);
        }
    }
})

_SLZ_SERIALIZER(FsOpRead_args, slz_FsOpRead, catch, _SLZ_ARRAY(path.string, path.length, char, catch))
_SLZ_SERIALIZER(FsOpWrite_args, slz_FsOpWrite, catch, _SLZ_ARRAY(path.string, path.length, char, catch) _SLZ_ARRAY(data, length, char, catch))
_SLZ_SERIALIZER(FsOpCreate_args, slz_FsOpCreate, catch, _SLZ_ARRAY(path.string, path.length, char, catch))
_SLZ_SERIALIZER(FsOpRemove_args, slz_FsOpRemove, catch, _SLZ_ARRAY(path.string, path.length, char, catch))
_SLZ_SERIALIZER(FsOpStat_args, slz_FsOpStat, catch, _SLZ_ARRAY(path.string, path.length, char, catch))
_SLZ_SERIALIZER(FsOpReadDir_args, slz_FsOpReadDir, catch, _SLZ_ARRAY(path.string, path.length, char, catch))
_SLZ_SERIALIZER(String, slz_String, catch, _SLZ_ARRAY(string, length, char, catch))
_SLZ_SERIALIZER(FsOpStat_response, slz_FsOpStatResponse, catch, _SLZ_ARRAY(blocks, blocks_count, size_t, catch))
_SLZ_SERIALIZER(DirectoryContent, slz_DirectoryContent, catch, _SLZ_ARRAY(items, items_count, DirectoryItem, catch))
