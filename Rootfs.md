# 容器技术-Rootfs

运行环境:Ubuntu 22.04.5 LTS

## Mount Namespace

编写一个实验程序：
````c++
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
````
你这段代码是使用 clone() 创建一个容器（container）并进入新的挂载命名空间（CLONE_NEWNS），然后在容器中执行 /bin/bash。
````shell
gcc -o ns ns.c

sudo ./ns
# 显示
父进程 - 启动一个容器！
容器 - 已进入容器内部！

ls
# 显示了宿主机的目录，原因是如果没有“挂载”这个操作，新创建的进程会直接继承宿主机的各个挂载点。
公共的  视频  文档  音乐  ns    snap
模板    图片  下载  桌面  ns.c  thinclient_drives
````

修改一下container_main函数：
````c++
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
````
````shell
gcc -o ns-1 ns-1.c

sudo ./ns-1

ls /tmp
# 显示啥都没有

mount -l | grep tmpfs

# 显示
udev on /dev type devtmpfs (rw,nosuid,relatime,size=3760128k,nr_inodes=940032,mode=755,inode64)
tmpfs on /dev/shm type tmpfs (rw,nosuid,nodev,inode64)
tmpfs on /run type tmpfs (rw,nosuid,nodev,noexec,relatime,size=760316k,mode=755,inode64)
tmpfs on /run/lock type tmpfs (rw,nosuid,nodev,noexec,relatime,size=5120k,inode64)
tmpfs on /run/user/128 type tmpfs (rw,nosuid,nodev,relatime,size=760312k,nr_inodes=190078,mode=700,uid=128,gid=134,inode64)
tmpfs on /run/snapd/ns type tmpfs (rw,nosuid,nodev,noexec,relatime,size=760316k,mode=755,inode64)
tmpfs on /run/user/1000 type tmpfs (rw,nosuid,nodev,relatime,size=760312k,nr_inodes=190078,mode=700,uid=1000,gid=1000,inode64)
none on /tmp type tmpfs (rw,relatime,inode64)

# 推出容器
exit

# 显示
exit
父进程 - 容器已停止

# 回到宿主机
ls /tmp

# 显示
snap-private-tmp
systemd-private-a310eba3a231437fad4df94c9c8dfc20-bluetooth.service-FhUXZW
systemd-private-a310eba3a231437fad4df94c9c8dfc20-colord.service-iiNbsy
systemd-private-a310eba3a231437fad4df94c9c8dfc20-ModemManager.service-UaCTVz
systemd-private-a310eba3a231437fad4df94c9c8dfc20-ntp.service-In3Bfn
systemd-private-a310eba3a231437fad4df94c9c8dfc20-power-profiles-daemon.service-TB0z7u
systemd-private-a310eba3a231437fad4df94c9c8dfc20-switcheroo-control.service-G0xtzE
systemd-private-a310eba3a231437fad4df94c9c8dfc20-systemd-logind.service-AtllHk
systemd-private-a310eba3a231437fad4df94c9c8dfc20-systemd-oomd.service-wL0OGk
systemd-private-a310eba3a231437fad4df94c9c8dfc20-systemd-resolved.service-AUD0C3
systemd-private-a310eba3a231437fad4df94c9c8dfc20-upower.service-PsnsxS

mount -l | grep tmpfs
# 显示
udev on /dev type devtmpfs (rw,nosuid,relatime,size=3760128k,nr_inodes=940032,mode=755,inode64)
tmpfs on /run type tmpfs (rw,nosuid,nodev,noexec,relatime,size=760316k,mode=755,inode64)
tmpfs on /dev/shm type tmpfs (rw,nosuid,nodev,inode64)
tmpfs on /run/lock type tmpfs (rw,nosuid,nodev,noexec,relatime,size=5120k,inode64)
tmpfs on /run/user/128 type tmpfs (rw,nosuid,nodev,relatime,size=760312k,nr_inodes=190078,mode=700,uid=128,gid=134,inode64)
tmpfs on /run/snapd/ns type tmpfs (rw,nosuid,nodev,noexec,relatime,size=760316k,mode=755,inode64)
tmpfs on /run/user/1000 type tmpfs (rw,nosuid,nodev,relatime,size=760312k,nr_inodes=190078,mode=700,uid=1000,gid=1000,inode64)
````
以上实验演示通过Mount Namespace达成了隔离宿主机和容器挂载的效果。Mount Namespace与其他Namespace不同，它对容器进程视图的修改一定要伴随着挂载操作才能生效。

## chroot

chroot是一个Linux/Unix系统命令，它的作用是：临时改变当前进程及其子进程所看到的根目录（即 /），限制它们只能访问指定的新根目录下的文件。 这就像给程序“伪造”了一个新的操作系统环境，它以为自己在系统根目录下运行，其实它只能访问你指定的那个目录树。

实际上，Mount Namespace正是基于对chroot的不断改良才被发明出来的，Mount Namespace也是Linux操作系统里第一个Namespace。

一般会在容器的根目录下挂载一个完整操作系统的文件系统，这个挂载在容器根目录上用来为容器提供隔离后执行环境的文件系统，就是所谓的“容器镜像”，它还有一个专业的名字：rootfs(根文件系统)。

Docker最核心的原理就是为待创建的用户进程：
- 启动Linux Namespace配置;
- 设置指定的Cgroups参数;
- 切换进程的根目录(change root)

这样，一个完整的容器就诞生了。