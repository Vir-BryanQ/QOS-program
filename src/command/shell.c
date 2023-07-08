#include "fs.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "syscall.h"
#include "global.h"

#define MAX_ARG_CNT 16      // 最大参数数量
#define MAX_CMD_LEN 512     // 最大的命令长度
#define MAX_PATH_CNT 16     // 最多支持的路径参数数量

#define TOKEN_0 ' '           // 参数之间的分隔符
#define TOKEN_1 '\t'           // 参数之间的分隔符

char cwd[MAX_PATH_LEN];         // 当前工作目录
char prev_wd[MAX_PATH_LEN];     // 切换到当前目录前所在的目录

char cmd_line[MAX_CMD_LEN];  // 用户输入的命令
char *argv[MAX_ARG_CNT];    // 由参数构成的字符串数组
int32_t argc = 0;              // 参数数量

int32_t status;             // 上一条命令执行完成后的退出状态

void print_prompt(void);      // 显示命令提示符
void cmd_parse(const char *cmd);         // 解析用户输入的命令
int32_t cmd_execute(const char *cmd);     // 执行用户命令
uint32_t read_line(char *buf, const uint32_t size);     // 读取一行字符到大小为size的buf中


// 显示命令提示符
void print_prompt(void)
{
    printf("[vir-bryanq-dylan@localhost %s]$ ", cwd);
}   

// shell内建命令
void builtin_pwd(void);
void builtin_cd(void);
void builtin_ls(void);
void builtin_clear(void);
void builtin_mkdir(void);
void builtin_rmdir(void);
void builtin_rm(void);
void builtin_touch(void);
void builtin_ps(void);
void builtin_exit(void);

// 解析用户输入的命令
void cmd_parse(const char *cmd)
{
    memset(argv, 0, sizeof(argv));
    argc = 0;
    char *next = cmd_line;
    while (*next)
    {
        // 寻找非分隔符
        while (*next == TOKEN_0 || *next == TOKEN_1)
        {
            next++;
        }
        if (*next == 0)
        {
            // 处理字符串最后几个字符为分隔符的情况
            return;
        }
        if (argc == MAX_ARG_CNT)
        {
            // 参数数量已达上限
            argc = -1;
            return;
        }
        argv[argc++] = next;

        // 寻找下一个分隔符
        while (*next && *next != TOKEN_0 && *next != TOKEN_1)
        {
            next++;
        }
        if (*next == 0)
        {
            return;
        }
        *(next++) = 0;
    }
}  

// 执行用户命令
int32_t cmd_execute(const char *cmd)
{
    cmd_parse(cmd);
    if (argc == -1)
    {
        printf("Too many arguments!\n");
        return 0;
    }
    if (!argv[0])
    {
        // 若用户只输入了分隔符
        return 0;
    }
    
    if (!strcmp(argv[0], "pwd"))
    {
        builtin_pwd();
        return 0;
    }
    else if (!strcmp(argv[0], "cd"))
    {
        builtin_cd();
        return 0;
    }
    else if (!strcmp(argv[0], "ls"))
    {
        builtin_ls();
        return 0;
    }
    else if (!strcmp(argv[0], "clear"))
    {
        builtin_clear();
        return 0;
    }
    else if (!strcmp(argv[0], "mkdir"))
    {
        builtin_mkdir();
        return 0;
    }
    else if (!strcmp(argv[0], "rmdir"))
    {   
        builtin_rmdir();
        return 0;
    }
    else if (!strcmp(argv[0], "rm"))
    {
        builtin_rm();
        return 0;
    }
    else if (!strcmp(argv[0], "touch"))
    {
        builtin_touch();
        return 0;
    }
    else if (!strcmp(argv[0], "ps"))
    {
        builtin_ps();
        return 0;
    }
    else if (!strcmp(argv[0], "echo"))
    {
        printf("%d\n", status);
        return 0;
    }
    else if (!strcmp(argv[0], "exit"))
    {
        builtin_exit();
        return 0;
    }
    else
    {
        if (strchr(argv[0], '/'))
        {
            // 有斜杠的当可执行文件路径处理
            struct stat buf;
            if (!stat(argv[0], &buf))
            {
                if (!fork())
                {
                    if (execv(argv[0], argv) == -1)
                    {
                        fd_redirect(stdout, 1);
                        printf("shell: command not found: %s\n", argv[0]);
                        exit(-1);
                    }
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                fd_redirect(stdout, 1);
                printf("shell: no that file or directory: %s\n", argv[0]);
                return 0;
            }
        }
        else
        {
            // 无斜杠的当外部命令处理
            if (strlen("/bin/") + strlen(argv[0]) >= 64)
            {
                fd_redirect(stdout, 1);
                printf("shell: command too long\n");
                return 0;
            }
            char path[64];
            strcpy(path, "/bin/");
            strcat(path, argv[0]);

            struct stat buf;
            if (!stat(path, &buf))
            {
                if (!fork())
                {
                    if (execv(path, argv) == -1)
                    {
                        fd_redirect(stdout, 1);
                        printf("shell: command not found: %s\n", argv[0]);
                        exit(-1);
                    }
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                fd_redirect(stdout, 1);
                printf("shell: command not found: %s\n", argv[0]);
                return 0;
            }
        }
    }

    return 0;
}     

// 读取一行字符到大小为size的buf中
uint32_t read_line(char *buf, const uint32_t size)
{
    uint32_t i = 0;
    while (i < size - 1)
    {
        read(stdin, buf + i, 1);
        switch (buf[i])
        {
            case '\r':
            case '\n':
            {
                putchar(buf[i]);
                buf[i] = 0;
                return i;
            }
            case '\b':
            {
                if (i)
                {
                    putchar(buf[i]);
                    --i;
                }
                break;
            }
            case ('l' - 'a'):
            {
                clear();
                buf[i] = 0;
                print_prompt();
                printf("%s", buf);
                break;
            }
            case ('u' - 'a'):
            {
                while (i)
                {
                    putchar('\b');
                    --i;
                }
                break;
            }
            case ('d' - 'a'):
            {
                putchar('\n');
                builtin_exit();
            }
            default:
            {
                putchar(buf[i]);
                ++i;
                break;
            }
        }
    }
    buf[i] = 0;
    return size - 1;
}

// 命令解释器
int main(void)
{
    getcwd(cwd, MAX_PATH_LEN);
    strcpy(prev_wd, cwd);
    clear();
    while (1)
    {
        print_prompt();
        read_line(cmd_line, MAX_CMD_LEN);       // 读取用户输入
        char *p = strrchr(cmd_line, '\n');
        if (p)
        {
            // 去除末尾的换行符
            *p = 0;
        }
        if (cmd_line[0] == 0)
        {
            // 若用户只输入了一个回车
            continue;
        }
        if (strchr(cmd_line, '|'))
        {
            uint32_t subproc_cnt = 0;
            char *cmd = cmd_line;
            char *pipe_token = strchr(cmd, '|');
            *pipe_token = 0;

            uint32_t pipe_fd[2], _pipe_fd[2];
            pipe(pipe_fd);
            fd_redirect(stdout, pipe_fd[1]);

            if (cmd_execute(cmd))
            {
                ++subproc_cnt;
            }
            cmd = pipe_token + 1;

            while ((pipe_token = strchr(cmd, '|')))
            {
                *pipe_token = 0;

                fd_redirect(stdin, pipe_fd[0]);
                _pipe_fd[0] = pipe_fd[0];
                _pipe_fd[1] = pipe_fd[1];
                pipe(pipe_fd);
                fd_redirect(stdout, pipe_fd[1]);

                if (cmd_execute(cmd))
                {
                    ++subproc_cnt;
                }
                cmd = pipe_token + 1;

                close(_pipe_fd[0]);
                close(_pipe_fd[1]);
            }

            fd_redirect(stdin, pipe_fd[0]);
            fd_redirect(stdout, 1);

            if (cmd_execute(cmd))
            {
                ++subproc_cnt;
            }

            close(pipe_fd[0]);
            close(pipe_fd[1]);

            while (subproc_cnt)
            {
                wait(&status);
                --subproc_cnt;
            }

            fd_redirect(stdin, 0);
        }
        else
        {
            if (cmd_execute(cmd_line))
            {
                wait(&status);
            }
        }
    }

    return 0;
}      


/***     以下是shell的内建命令       ***/

void builtin_pwd(void)
{
    printf("%s\n", cwd);
}

void builtin_cd(void)
{   
    if (argv[1])
    {
        if (!strcmp(argv[1], "-"))
        {
            // 若是 cd - 命令，则切换到最近一次所在的目录
            chdir(prev_wd);
        }
        else
        {
            chdir(argv[1]);
        }
    }
    else
    {
        // 若未给cd提供参数，默认回到根目录
        chdir("/");
    }

    // 保存当前所在目录
    strcpy(prev_wd, cwd);

    // 修改当前工作目录后要修改对应的字符串数组cwd
    getcwd(cwd, MAX_PATH_LEN);
}

void builtin_ls(void)
{
    // 标记用户输入的命令中是否有某个选项
    uint8_t arg_l = 0;
    uint8_t arg_d = 0;
    uint8_t arg_a = 0;

    // 记录用户提供的路径参数
    char *arg_path[MAX_PATH_CNT] = { NULL };

    // 解析选项参数和路径参数
    uint32_t argv_idx = 1;
    uint32_t arg_path_idx = 0;
    while (argv[argv_idx])
    {
        if (argv[argv_idx][0] == '-'  && argv[argv_idx][1] != 0)
        {
            uint32_t arg_idx = 1;
            while (argv[argv_idx][arg_idx])
            {
                switch (argv[argv_idx][arg_idx])
                {
                    case 'l':
                    {
                        arg_l = 1;
                        break;
                    }
                    case 'd':
                    {
                        arg_d = 1;
                        break;
                    }
                    case 'a':
                    {
                        arg_a = 1;
                        break;
                    }
                    default:
                    {
                        printf("ls: invalid option -- %c\n", argv[argv_idx][arg_idx]);
                        return;
                    }
                }
                arg_idx++;
            }
        }
        else
        {
            arg_path[arg_path_idx++] = argv[argv_idx];
        }
        
        argv_idx++;
    }

    uint32_t arg_path_cnt = arg_path_idx;

    // 如果用户未提供路径，默认输出当前路径
    if (!arg_path_cnt)
    {
        arg_path[0] = cwd;
        arg_path_cnt = 1;
    }

    // 逐个路径解析输出
    struct stat buf;
    for (arg_path_idx = 0; arg_path_idx < arg_path_cnt; ++arg_path_idx)
    {
        if (!stat(arg_path[arg_path_idx], &buf))
        {
            if (buf.f_type == FT_REGULAR)
            {
                if (arg_l)
                {
                    pad_print(&buf.i_no, 'u', 8);
                    printf("-   ");
                    pad_print(&buf.i_size, 'u', 12);
                    printf("%s\n", arg_path[arg_path_idx]);
                }
                else
                {
                    printf(arg_path_idx == arg_path_cnt - 1 ? "%s\n" : "%s ", arg_path[arg_path_idx]);
                }
            }
            else if (buf.f_type == FT_DIRECTORY)
            {
                if (arg_d)
                {
                    if (arg_l)
                    {
                        pad_print(&buf.i_no, 'u', 8);
                        printf("d   ");
                        pad_print(&buf.i_size, 'u', 12);
                        printf("%s\n", arg_path[arg_path_idx]);
                    }
                    else
                    {
                        printf(arg_path_idx == arg_path_cnt - 1 ? "%s\n" : "%s ", arg_path[arg_path_idx]);
                    }
                }
                else
                {
                    if (arg_path_cnt > 1)
                    {
                        printf("%s:\n", arg_path[arg_path_idx]);   
                    }
                    
                    dir *pdir = opendir(arg_path[arg_path_idx]);
                    dentry *pdentry;
                    char sub_path[MAX_PATH_LEN];
                    while ((pdentry = readdir(pdir)))
                    {
                        if (!arg_a && (!strcmp(pdentry->filename, ".") || !strcmp(pdentry->filename, "..")))
                        {
                            continue;
                        }

                        if (arg_l)
                        {
                            strcpy(sub_path, arg_path[arg_path_idx]);
                            strcat(sub_path, "/");
                            strcat(sub_path, pdentry->filename);

                            if (!stat(sub_path, &buf))
                            {
                                pad_print(&buf.i_no, 'u', 8);
                                printf(buf.f_type == FT_DIRECTORY ? "d   " : "-   ");
                                pad_print(&buf.i_size, 'u', 12);
                                printf("%s\n", pdentry->filename);
                            }
                            else
                            {
                                printf("shell: should not be here\n");
                                while (1);
                            }
                        }
                        else
                        {
                            pad_print(pdentry->filename, 's', 16);
                        }
                    }

                    if (!arg_l)
                    {
                        printf("\n");
                    }

                    if (arg_path_cnt > 1 && arg_path_idx < arg_path_cnt - 1)
                    {
                        printf("\n");   
                    }

                    closedir(pdir);
                }
            }
        }
        else
        {
            printf("ls: unable to find '%s'\n", arg_path[arg_path_idx]);
        }
    }
}

void builtin_clear(void)
{
    clear();
}

void builtin_mkdir(void)
{
    if (!argv[1])
    {
        printf("mkdir: missing operand\n");
        return;
    }

    for (uint32_t i = 1; argv[i]; ++i)
    {
        if (!mkdir(argv[i]))
        {
            printf ("mkdir: '%s' created\n", argv[i]);
        }
    }
}

void builtin_rmdir(void)
{
    if (!argv[1])
    {
        printf("rmdir: missing operand\n");
        return;
    }

    for (uint32_t i = 1; argv[i]; ++i)
    {
        if (!rmdir(argv[i]))
        {
            printf("rmdir: '%s' removed\n", argv[i]);
        }
    }
}

void builtin_rm(void)
{
    // 标记用户输入的命令中是否有某个选项
    uint8_t arg_f = 0;

    // 记录用户提供的路径参数
    char *arg_path[MAX_PATH_CNT] = { NULL };

    // 解析选项参数和路径参数
    uint32_t argv_idx = 1;
    uint32_t arg_path_idx = 0;
    while (argv[argv_idx])
    {
        if (argv[argv_idx][0] == '-'  && argv[argv_idx][1] != 0)
        {
            uint32_t arg_idx = 1;
            while (argv[argv_idx][arg_idx])
            {
                switch (argv[argv_idx][arg_idx])
                {
                    case 'f':
                    {
                        arg_f = 1;
                        break;
                    }
                    default:
                    {
                        printf("rm: invalid option -- %c\n", argv[argv_idx][arg_idx]);
                        return;
                    }
                }
                arg_idx++;
            }
        }
        else
        {
            arg_path[arg_path_idx++] = argv[argv_idx];
        }
        
        argv_idx++;
    }

    uint32_t arg_path_cnt = arg_path_idx;

    // 如果用户未提供路径
    if (!arg_path_cnt)
    {
        printf("rm: missing operand\n");
        return;
    }

    struct stat buf;
    for (arg_path_idx = 0; arg_path_idx < arg_path_cnt; ++arg_path_idx)
    {
        if (!stat(arg_path[arg_path_idx], &buf))
        {
            if (buf.f_type == FT_REGULAR)
            {
                if (!arg_f)
                {
                    char sel[8];
                    printf("Confirm to remove the regular file '%s'? ", arg_path[arg_path_idx]);
                    read_line(sel, 8);
                    if (sel[0] != 'y' && sel[0] != 'Y')
                    {
                        continue;
                    }
                }
                unlink(arg_path[arg_path_idx]);
            }
            else
            {
                printf("rm: unable to remove '%s': it's a directory\n", arg_path[arg_path_idx]);
            }
        }
        else
        {
            printf("rm: unable to find '%s': no that file or directory\n", arg_path[arg_path_idx]);
        }
    }
}

void builtin_touch(void)
{
    if (!argv[1])
    {
        printf("touch: missing operand\n");
        return;
    }

    struct stat buf;
    for (uint32_t i = 1; argv[i]; ++i)
    {
        if (stat(argv[i], &buf))
        {
            uint32_t fd = open(argv[i], O_CREAT);
            if (fd != -1)
            {
                close(fd);
            }
        }
    }
}

void builtin_ps(void)
{
    ps();
}

void builtin_exit(void)
{
    exit(0);
}



