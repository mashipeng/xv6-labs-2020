#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 拼接路径: parent + '/' + name  (确保不超出 buf 大小)
static int buildpath(char *buf, int bufsize, const char *parent, const char *name)
{
    int lenp = strlen(parent);
    int lenn = strlen(name);
    if (lenp + 1 + lenn + 1 > bufsize)
    {
        return -1; // too long
    }
    memmove(buf, parent, lenp);
    buf[lenp] = '/';
    memmove(buf + lenp + 1, name, lenn);
    buf[lenp + 1 + lenn] = '\0';
    return 0;
}

void find(char *path, const char *target)
{
    int fd = open(path, 0);
    if (fd < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type == T_FILE)
    {
        // 文件:比较最后一段名字
        char *p = path + strlen(path);
        while (p >= path && *p != '/')
            p--;
        p++; // 指向文件名
        if (strcmp(p, target) == 0)
        {
            printf("%s\n", path);
        }
        close(fd);
        return;
    }

    if (st.type != T_DIR)
    {
        close(fd);
        return; // 非普通文件/目录，忽略
    }

    // 目录:遍历
    struct dirent de;
    char buf[512];
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;
        // 跳过 . 和 ..
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        // 构造子路径
        if (buildpath(buf, sizeof(buf), path, de.name) < 0)
        {
            fprintf(2, "find: path too long %s/%s\n", path, de.name);
            continue;
        }
        struct stat st2;
        if (stat(buf, &st2) < 0)
        {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        if (st2.type == T_FILE)
        {
            // 判断文件名是否匹配 (de.name 可能包含填充的 \0 后面不需要管, xv6 的 name 已经是 DIRSIZ bytes)
            // 由于 de.name 可能有未使用字节,先确保以 0 结尾
            char name[DIRSIZ + 1];
            memmove(name, de.name, DIRSIZ);
            name[DIRSIZ] = '\0';
            // 去掉可能的尾部空格( xv6 不保证填充空格,一般是 0 )
            if (strcmp(name, target) == 0)
            {
                printf("%s\n", buf);
            }
        }
        else if (st2.type == T_DIR)
        {
            // 递归目录
            find(buf, target);
        }
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "usage: find <startpath> <filename>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
