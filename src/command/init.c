#include "syscall.h"
#include "global.h"
#include "debug.h"

int main(void)
{
    if (!fork())
    {
        if (execv("/bin/shell", NULL) == -1)
        {
            panic_spin(__FILE__, __LINE__, __func__, "Fail to load /bin/shell");
        }
    }
    else
    {
        while (1)
        {
            int32_t status;
            wait(&status);
        }
    }

    return 0;
}