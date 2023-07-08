#ifndef __HEAD_STDIO_H
#define __HEAD_STDIO_H

#include "stdint.h"

#define stdin 0
#define stdout 1
#define stderr 2

extern uint32_t printf(const char *format, ...);                // 往屏幕上打印字符串
extern uint32_t sprintf(char *buf, const char *format, ...);    // 往buf中传送字符串
extern uint32_t pad_print(void *data, char format, uint32_t pad_size);  // 以填充空格的方式以指定格式输出数据

#endif