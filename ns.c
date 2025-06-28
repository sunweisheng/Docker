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