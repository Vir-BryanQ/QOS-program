#ifndef __HEAD_GLOBAL_H
#define __HEAD_GLOBAL_H

#define NULL ((void *)0)

#define UNUSED __attribute__((unused))      // 该属性可以避免编译器产生警告信息
#define PACKED __attribute__((packed))      // 该属性可以避免定义结构体时编译器为了对齐在各个成员之间添加空隙

#define DIV_ROUND_UP(val0, val1)  (((val0) + (val1) - 1) / (val1))        // 上取整除法

#endif

