#define _GNU_SOURCE
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];

char* const container_args[] = {
    "/bin/bash",
    NULL
  };

int container_main(void* arg){
    printf("容器 - 已进入容器内部！\n");
    
    // 默认的 mount 命名空间共享传播类型是 shared（共享传播） ，所以你在子命名空间做的 mount 操作会“传播”到父命名空间！
    // 将当前挂载点设为 private，防止传播到宿主机
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        perror("设置 mount namespace 为 private 失败");
        return 1;
    }
    // 在容器启动之前以tmpfs(内存盘)格式重新挂载/tmp目录
    mount("none", "/tmp", "tmpfs", 0, "");
    execv(container_args[0], container_args);
    perror("execv 执行失败");
    printf("发生错误，请检查路径或权限！\n");
    return 1;
}

int main(){
    printf("父进程 - 启动一个容器！\n");

    int container_pid = clone(container_main,
                               container_stack + STACK_SIZE,
                               CLONE_NEWNS | SIGCHLD,
                               NULL);

    if (container_pid == -1) {
        perror("clone 失败");
        return 1;
    }

    waitpid(container_pid, NULL, 0);
    printf("父进程 - 容器已停止！\n");

    return 0;
}