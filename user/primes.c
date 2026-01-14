#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 处理素数筛选的循环版本
void sieve(int input_fd)
{
    int prime;
    int n;

    // 持续处理,直到没有数据
    while (1)
    {
        // 读取第一个数(必然是素数)
        if (read(input_fd, &prime, sizeof(int)) == 0)
        {
            // 管道关闭,没有更多数据
            exit(0);
        }

        printf("prime %d\n", prime);

        // 创建新管道传递给子进程
        int p[2];
        pipe(p);

        int pid = fork();

        if (pid < 0)
        {
            fprintf(2, "fork failed\n");
            exit(1);
        }

        if (pid == 0)
        {
            // 子进程:成为新的筛子
            close(p[1]);     // 关闭写端
            close(input_fd); // 关闭旧的输入
            input_fd = p[0]; // 使用新管道作为输入
            // 继续循环处理
        }
        else
        {
            // 父进程:过滤数据并传递
            close(p[0]); // 关闭读端

            // 过滤剩余的数
            while (read(input_fd, &n, sizeof(int)) > 0)
            {
                if (n % prime != 0)
                {
                    write(p[1], &n, sizeof(int));
                }
            }

            // 完成过滤,关闭管道
            close(input_fd);
            close(p[1]);

            // 等待子进程完成
            wait(0);
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    int pid = fork();

    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        // 子进程:启动素数筛
        close(p[1]); // 关闭写端
        sieve(p[0]); // 开始筛选
        exit(0);
    }
    else
    {
        // 父进程:生成 2-35 的数字
        close(p[0]); // 关闭读端

        for (int i = 2; i <= 35; i++)
        {
            write(p[1], &i, sizeof(int));
        }

        close(p[1]); // 关闭写端,通知没有更多数据
        wait(0);     // 等待整个管道完成
        exit(0);
    }

    return 0;
}
