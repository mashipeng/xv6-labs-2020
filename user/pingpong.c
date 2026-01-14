#include "kernel/types.h"
#include "user/user.h"

int main()
{
    int p2c[2]; // 父到子管道
    int c2p[2]; // 子到父管道
    pipe(p2c);
    pipe(c2p);

    int pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        // 子进程
        char buf;
        // 从父进程读取
        read(p2c[0], &buf, 1);
        printf("%d: received ping\n", getpid());
        // 向父进程写回
        write(c2p[1], "x", 1);
        close(p2c[0]);
        close(p2c[1]);
        close(c2p[0]);
        close(c2p[1]);
        exit(0);
    }
    else
    {
        // 父进程
        // 向子进程写入
        write(p2c[1], "x", 1);
        char buf;
        // 从子进程读取
        read(c2p[0], &buf, 1);
        printf("%d: received pong\n", getpid());
        close(p2c[0]);
        close(p2c[1]);
        close(c2p[0]);
        close(c2p[1]);
        wait(0);
        exit(0);
    }
}