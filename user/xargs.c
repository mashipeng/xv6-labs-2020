#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

static void run_line(int argc, char *argv[], char *line)
{
    // 组装参数：先放固定命令及其参数(argv[1..])，再追加本行解析出的参数
    char *args[MAXARG];
    int n = 0;
    // 固定基参数
    for (int i = 1; i < argc && n < MAXARG - 1; i++)
    {
        args[n++] = argv[i];
    }
    int base = n; // 记录基参数个数

    // 将 line 按空白分词，指针直接指向 line 中各 token
    char *p = line;
    while (*p)
    {
        // 跳过前导空白
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == 0)
            break;
        if (n >= MAXARG - 1)
            break;
        // 记录一个 token 起点
        args[n++] = p;
        // 走到 token 末尾
        while (*p && *p != ' ' && *p != '\t')
            p++;
        if (*p == 0)
            break;
        *p = 0; // 以 0 结束 token
        p++;
    }
    args[n] = 0;

    // 若本行没有提供任何额外参数，则不执行(与实验期望一致)
    if (n <= base)
        return;

    int pid = fork();
    if (pid < 0)
    {
        fprintf(2, "xargs: fork failed\n");
        exit(1);
    }
    if (pid == 0)
    {
        exec(args[0], args);
        fprintf(2, "xargs: exec %s failed\n", args[0]);
        exit(1);
    }
    wait(0);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "usage: xargs command [args]\n");
        exit(1);
    }

    char line[1024];
    int len = 0;
    char ch;

    // 从标准输入读到换行，逐行处理
    while (1)
    {
        int r = read(0, &ch, 1);
        if (r < 1)
        {
            // EOF：处理可能残留的最后一行(无换行结尾)
            if (len > 0)
            {
                line[len] = 0;
                run_line(argc, argv, line);
            }
            break;
        }
        if (ch == '\n')
        {
            line[len] = 0;
            run_line(argc, argv, line);
            len = 0;
        }
        else
        {
            if (len < (int)sizeof(line) - 1)
            {
                line[len++] = ch;
            }
            // 否则超长行截断，多余的字符丢弃直到换行
        }
    }
    exit(0);
}
