#include "stdio.h"
#include "debug.h"

// 输出错误信息并进入自旋状态
void panic_spin(const char *filename, const int line, const char *func, const char *condition)
{
    printf("\n");
    printf("! ! ! ! ! panic ! ! ! ! !\n");
    printf("File: %s\n", filename);
    printf("Line: %d\n", line);
    printf("Function: %s\n", func);
    printf("Condition: %s\n", condition);

    while (1);
}