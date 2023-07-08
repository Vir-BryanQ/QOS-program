#ifndef __FS_FS_H
#define __FS_FS_H

#include "stdint.h"

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 4

#define MAX_PATH_LEN 256
#define MAX_FILENAME_LEN 32

typedef enum file_type
{
    FT_UNKNOWN,         // 未知文件类型
    FT_REGULAR,         // 普通文件类型
    FT_DIRECTORY        // 目录文件类型
} file_type;

// 存储一个文件的相关信息
struct stat
{
    uint32_t i_no;
    uint32_t i_size;
    file_type f_type;
};

// 目录表中的目录项
typedef struct dentry
{
    uint32_t i_no;          // 目录项对应文件的inode编号
    char filename[MAX_FILENAME_LEN];     // 文件名
    file_type f_type;       // 文件类型
} dentry;

#endif
