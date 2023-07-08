#include "stdio.h"
#include "syscall.h"
#include "fs.h"

int main(int argc, char *argv[])
{
    if (!argv[1])
    {
        
        return -1;
    }

    char *buf = (char *)malloc(512);
    if (!buf)
    {
        printf("cat: memory allocation failed\n");
        return -1;
    }

    int32_t fd;
    int32_t read_done;
    int32_t ret = 0;
    for (uint32_t i = 1; argv[i]; ++i)
    {
        fd = open(argv[i], O_RDONLY);
        if (fd == -1)
        {
            ret = -1;
            printf("cat: unable to open the file '%s'\n", argv[i]);
            continue;
        }
        
        while ((read_done = read(fd, buf, 511)))
        {
            buf[read_done] = 0;
            printf("%s\n", buf);
        }

        close(fd);
    }
    return ret;
}