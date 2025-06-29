#define _GNU_SOURCE     // 启用 GNU 扩展特性（为了使用 setns）
#include <fcntl.h>      // 文件控制选项，用于 open()
#include <sched.h>      // sched 头文件，包含 setns() 的声明
#include <unistd.h>     // execvp(), fork(), 等 POSIX API
#include <stdlib.h>     // exit(), EXIT_FAILURE
#include <stdio.h>      // perror()

// 定义错误处理宏：打印错误信息并退出
// 例如调用 errExit("open") 将输出类似 "open: No such file or directory"
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
    int fd;

    // 检查参数个数是否正确：
    // 第一个参数是命名空间的路径（如 /proc/PID/ns/net）
    // 第二个参数是要执行的命令（比如 bash）
    if (argc < 3) {
        fprintf(stderr, "Usage: %s /proc/PID/ns/TYPE command [arguments...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 打开指定的命名空间文件（只读方式）
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        errExit("open");
    }

    // 将当前进程加入到打开的命名空间中
    // 第二个参数 0 表示类型由文件描述符本身决定
    if (setns(fd, 0) == -1) {
        errExit("setns");   // 如果失败，打印错误并退出
    }

    // 关闭文件描述符，因为我们已经加入了命名空间
    close(fd);

    // 在新的命名空间中执行指定的命令
    // execvp() 会替换当前进程的镜像为新程序
    execvp(argv[2], &argv[2]);

    // 如果 execvp 返回说明出错（比如命令不存在），则打印错误
    errExit("execvp");
}