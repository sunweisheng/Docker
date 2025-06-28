# 容器技术-overlay2

运行环境:Ubuntu 22.04.5 LTS

## UnionFS
UnionFS (Union File System) 是一种联合文件系统，它允许将多个目录（称为分支）透明地叠加在一起，形成一个统一的视图。这种文件系统在容器技术（如Docker）、Live CD系统和嵌入式设备中广泛应用。

基本概念：
UnionFS 的核心思想是将多个目录层次结构"叠加"在一起，形成一个单一的、统一的文件系统视图：
- 下层分支 (Lower Branch): 通常是只读的基础层
- 上层分支 (Upper Branch): 可写层，用于存储修改
- 合并视图 (Merged View): 呈现给用户的统一文件系统视图

主要特性：
- 写时复制 (Copy-on-Write, CoW):当需要修改下层文件时，UnionFS会先将文件复制到上层再进行修改，这保证了原始数据不被改变，同时允许用户看到修改后的内容。
- 文件优先级:上层分支的文件会覆盖下层分支的同名文件 ，通常遵循"从上到下"的搜索顺序。
- 白名单机制:可以通过特殊的白名单文件排除某些文件或目录。

常见实现：
- AUFS (Advanced Multi-Layered Unification Filesystem):Docker早期使用的实现，功能丰富但未被合并到Linux主线内核。
- OverlayFS:已合并到Linux内核主线(从3.18版本开始)，Docker目前默认使用的联合文件系统，性能优于AUFS。
- Union Mount:早期的简单实现。

工作示例
假设有两个目录：
````text
lower/
  file1
  file2
upper/
  file2
  file3
````
UnionFS合并后的视图将显示：
```text
merged/
  file1 (来自lower)
  file2 (来自upper，覆盖了lower中的版本)
  file3 (来自upper)
````

## overlay2

````shell
# Docker Version:28.2.2
docker info | grep "Storage Driver"

# 显示
 Storage Driver: overlay2
````

查看一个镜像信息：
````shell
docker inspect  ubuntu
````

````json
[
    {
        "Id": "sha256:bf16bdcff9c96b76a6d417bd8f0a3abe0e55c0ed9bdb3549e906834e2592fd5f",
        "RepoTags": [
            "ubuntu:latest"
        ],
        "RepoDigests": [
            "ubuntu@sha256:b59d21599a2b151e23eea5f6602f4af4d7d31c4e236d22bf0b62b86d2e386b8f"
        ],
        "Parent": "",
        "Comment": "",
        "Created": "2025-05-29T04:21:01.971275965Z",
        "DockerVersion": "24.0.7",
        "Author": "",
        "Config": {
            "Env": [
                "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
            ],
            "Cmd": [
                "/bin/bash"
            ],
            "Labels": {
                "org.opencontainers.image.ref.name": "ubuntu",
                "org.opencontainers.image.version": "24.04"
            }
        },
        "Architecture": "amd64",
        "Os": "linux",
        "Size": 78112490,
        "GraphDriver": {
            "Data": {
                "MergedDir": "/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/merged",
                "UpperDir": "/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/diff",
                "WorkDir": "/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/work"
            },
            "Name": "overlay2"
        },
        "RootFS": {
            "Type": "layers",
            "Layers": [
                "sha256:a8346d259389bc6221b4f3c61bad4e48087c5b82308e8f53ce703cfc8333c7b3"
            ]
        },
        "Metadata": {
            "LastTagTime": "0001-01-01T00:00:00Z"
        }
    }
]
````

这段信息来自Docker容器的GraphDriver信息，具体是使用overlay2存储驱动时的结构:

UpperDir（上层目录）:/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/diff
- 这是容器的“可写层”。
- 所有对容器文件系统的修改（比如新建、删除、修改文件）都保存在这里。
- 是容器运行时的私有数据。
- 类比：你可以把它看作是容器自己的“本地改动区”。

MergedDir（合并视图）:/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/merged
- 这是一个虚拟目录，是你在容器内看到的整个文件系统。
- 它是由以下部分组合而成：
- 镜像的只读层（lower layers）
- 容器的可写层（upper layer） 
- 类比：这是你在容器中执行 ls / 时看到的真实目录结构，是“所有层合并后的视图”。

WorkDir（工作目录）:/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/work
- 这是 OverlayFS 内部使用的临时工作目录。
- 主要用于内部操作，比如复制文件（copy-up）等操作。
- 通常你不需要关心这个目录的具体内容。
- 类比：可以理解为 OverlayFS 的“缓存或中间操作目录”，类似 .git/index 或 /tmp 的作用。

OverlayFS 工作原理简述
- LowerDir（底层） ：镜像的只读层（可能有多个）
- UpperDir（上层） ：容器的可写层
- MergedDir（合并层） ：最终呈现给用户的统一视图

RootFS.Layers 列出的就是容器所依赖的 镜像只读层（LowerDir） ，它们是构成容器文件系统的基础部分。

当容器运行时：
- 读取文件 ：如果文件在 UpperDir 中存在，优先读 Upper；否则从 Lower 层读取
- 修改文件 ：触发 copy-up：将 Lower 层文件拷贝到 Upper 层再修改
- 删除文件 ：在 Upper 层创建 whiteout 文件标记删除

演示真实目录情况：
````shell
sudo ls /var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/diff

# 显示
bin  boot  dev	etc  home  lib	lib64  media  mnt  opt	proc  root  run  sbin  srv  sys  tmp  usr  var
````

Docker使用了一个中间映射文件：/var/lib/docker/overlay2/l/

这是 lower 的缩写，里面存放的是指向实际 layer 的符号链接（symbolic links），每个链接的名字是一个 短哈希（如：AHEI3L7TQZ...) ，对应你看到的 SHA256 哈希。
````shell
sudo ls -l /var/lib/docker/overlay2/l/

# 显示
lrwxrwxrwx 1 root root 72  6月 24 23:35 CWU7DHX3O4ULSIQD6LHE3HVBTI -> ../b9a37de7da70c4e9cf0db0e95c0861399d5233b9fb40f17102988ef7690763fc/diff
lrwxrwxrwx 1 root root 72  6月 25 01:05 ELRKWMMP7SQVCLZLQFVC3CDYT7 -> ../6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/diff
lrwxrwxrwx 1 root root 77  6月 29 02:38 MPFWW6XFDSYDM7UWPXQP2AYLSC -> ../ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6-init/diff
lrwxrwxrwx 1 root root 72  6月 29 02:38 PTKZ2VLNJ33LFN6WXTS44BZW44 -> ../ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6/diff
lrwxrwxrwx 1 root root 72  6月 22 11:51 XH27NKLRN2QCLCB5PD2T6A53AX -> ../419f1ad280fd8613486c079aa15e58d5ff3a3d49d33953900bd5180236922ea4/diff

sudo ls /var/lib/docker/overlay2/l/ELRKWMMP7SQVCLZLQFVC3CDYT7

# 显示
bin  boot  dev	etc  home  lib	lib64  media  mnt  opt	proc  root  run  sbin  srv  sys  tmp  usr  var
````
容器运行时：
````shell
docker ps -a

# 显示
CONTAINER ID   IMAGE     COMMAND       CREATED             STATUS          PORTS     NAMES
6aadf56dae72   ubuntu    "/bin/bash"   About an hour ago   Up 55 minutes             exciting_babbage

docker start 6aadf56dae72

# 显示
6aadf56dae72

docker exec -it 6aadf56dae72 /bin/bash

root@6aadf56dae72:/# mount

# 显示
overlay on / type overlay (rw,relatime,lowerdir=/var/lib/docker/overlay2/l/MPFWW6XFDSYDM7UWPXQP2AYLSC:/var/lib/docker/overlay2/l/ELRKWMMP7SQVCLZLQFVC3CDYT7,upperdir=/var/lib/docker/overlay2/ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6/diff,workdir=/var/lib/docker/overlay2/ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6/work,nouserxattr)
proc on /proc type proc (rw,nosuid,nodev,noexec,relatime)
tmpfs on /dev type tmpfs (rw,nosuid,size=65536k,mode=755,inode64)
devpts on /dev/pts type devpts (rw,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=666)
sysfs on /sys type sysfs (ro,nosuid,nodev,noexec,relatime)
cgroup on /sys/fs/cgroup type cgroup2 (ro,nosuid,nodev,noexec,relatime,nsdelegate,memory_recursiveprot)
mqueue on /dev/mqueue type mqueue (rw,nosuid,nodev,noexec,relatime)
shm on /dev/shm type tmpfs (rw,nosuid,nodev,noexec,relatime,size=65536k,inode64)
/dev/sda5 on /etc/resolv.conf type ext4 (rw,relatime,errors=remount-ro)
/dev/sda5 on /etc/hostname type ext4 (rw,relatime,errors=remount-ro)
/dev/sda5 on /etc/hosts type ext4 (rw,relatime,errors=remount-ro)
devpts on /dev/console type devpts (rw,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=666)
proc on /proc/bus type proc (ro,nosuid,nodev,noexec,relatime)
proc on /proc/fs type proc (ro,nosuid,nodev,noexec,relatime)
proc on /proc/irq type proc (ro,nosuid,nodev,noexec,relatime)
proc on /proc/sys type proc (ro,nosuid,nodev,noexec,relatime)
proc on /proc/sysrq-trigger type proc (ro,nosuid,nodev,noexec,relatime)
tmpfs on /proc/asound type tmpfs (ro,relatime,inode64)
tmpfs on /proc/acpi type tmpfs (ro,relatime,inode64)
tmpfs on /proc/interrupts type tmpfs (rw,nosuid,size=65536k,mode=755,inode64)
tmpfs on /proc/kcore type tmpfs (rw,nosuid,size=65536k,mode=755,inode64)
tmpfs on /proc/keys type tmpfs (rw,nosuid,size=65536k,mode=755,inode64)
tmpfs on /proc/latency_stats type tmpfs (rw,nosuid,size=65536k,mode=755,inode64)
tmpfs on /proc/timer_list type tmpfs (rw,nosuid,size=65536k,mode=755,inode64)
tmpfs on /proc/scsi type tmpfs (ro,relatime,inode64)
tmpfs on /sys/firmware type tmpfs (ro,relatime,inode64)
tmpfs on /sys/devices/virtual/powercap type tmpfs (ro,relatime,inode64)
tmpfs on /sys/devices/system/cpu/cpu0/thermal_throttle type tmpfs (ro,relatime,inode64)
tmpfs on /sys/devices/system/cpu/cpu1/thermal_throttle type tmpfs (ro,relatime,inode64)
tmpfs on /sys/devices/system/cpu/cpu2/thermal_throttle type tmpfs (ro,relatime,inode64)
tmpfs on /sys/devices/system/cpu/cpu3/thermal_throttle type tmpfs (ro,relatime,inode64)
````

现在看到的是一个运行中的 Docker 容器（6aadf56dae72）的挂载视图,它使用的是 overlay2 存储驱动 ，由多个 layer 组成，其中：
- lowerdir: 镜像的只读层（基础镜像）:lowerdir=/var/lib/docker/overlay2/l/MPFWW6XFDSYDM7UWPXQP2AYLSC:/var/lib/docker/overlay2/l/ELRKWMMP7SQVCLZLQFVC3CDYT7,（实际是/var/lib/docker/overlay2/6cfee4ce01531e34ec51772af69527541a64b7b21f8a8c991e1f8b29aa9f5fe9/diff）
- upperdir: 容器的可写层:upperdir=/var/lib/docker/overlay2/ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6/diff,
- workdir: overlayfs 内部工作目录:workdir=/var/lib/docker/overlay2/ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6/work)

````shell
sudo ls /var/lib/docker/overlay2/ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6

# 显示
diff  link  lower  merged  work
````

注意：/var/lib/docker/overlay2/l/下有个目录是：

lrwxrwxrwx 1 root root 77  6月 29 02:38 MPFWW6XFDSYDM7UWPXQP2AYLSC -> ../ec961545df56db9743cf6927a1d21c72157639ae652ea80d1f7e2bf1ff9fe7b6-init/diff

这个目录多了-init是因为这是Init层：

它是夹在只读层和可读可写层之间的层。Init是Docker的一个内部层，专门用来存放/etc/hosts、/etc/resolv.conf等信息。

以上这些层被联合挂载表现为一个完整的Ubuntu操作系统容器使用.


