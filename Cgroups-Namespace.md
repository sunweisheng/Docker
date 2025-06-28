# 容器技术-Namespace And Cgroups

运行环境:Ubuntu 22.04.5 LTS

## Namespace机制

程序一旦被执行，它就从磁盘上的二进制文件变成了由计算机内存中的数据、寄存器里的值、堆栈中的指令、被打开的文件，以及各种设备的状态信息组成的一个集合。一个程序运行起来之后的计算机执行环境的总和就是-进程。
计算机里的数据和状态总和，就是程序的动态表现。

````shell
# 运行一个容器
docker run -it busybox /bin/sh

# 进入容器之后执行PS
/ # ps
PID   USER     TIME  COMMAND
    1 root      0:00 /bin/sh
    7 root      0:00 ps
/ # 
````

在容器里的/bin/sh进程编号是1，这个容器只有两个进程在运行，容器里和容器外是互相隔离的环境，而实际情况呢？

````shell
# 在容器之外显示所有计算机进程
ps -ef | grep docker

# 实际容器的进程编号是3388
root         962       1  0 23:26 ?        00:00:04 /usr/bin/dockerd -H fd:// --containerd=/run/containerd/containerd.sock
sunweis+    3388    3308  0 23:50 pts/0    00:00:00 docker run -it busybox /bin/sh
sunweis+    3531    3517  0 23:53 pts/1    00:00:00 grep --color=auto docker
````

这种在运行进程中对进程空间动手脚的机制就是Namespace机制
````c++
// 创建一个进程时在参数中指定CLONE_NEWPID参数，就能使这个新进程看到一个全新的进程空间
int pid = clone(main_function, stack_size, CLONE_NEWPID | SIGCHLD, NULL);
````
除了对PID做手脚之外，Linux还提供了对Mount、UTS、IPC、Network和User这些内容的Namespace机制，所以容器只是一个特殊的进程而已。尽管在容器里利用Mount Namespace单独挂载其他版本的操作系统文件，比如CentOS或者Ubuntu，但这并不能改变共享宿主机内核的事实，所以Windows宿主机上运行Linux容器就行不通。

## Linux Cgroups(Linux control groups)

是Linux内核中用来为进程设置资源限制的功能，包括CPU、内存、磁盘、网络带宽等。容器可以理解为用Cgroups限制过的进程组。

````shell
# Cgroups向用户暴露出来的操作接口是文件系统
ls /sys/fs/cgroup

# 显示
cgroup.controllers      cpu.pressure           init.scope        memory.reclaim                 sys-kernel-debug.mount
cgroup.max.depth        cpuset.cpus.effective  io.cost.model     memory.stat                    sys-kernel-tracing.mount
cgroup.max.descendants  cpuset.cpus.isolated   io.cost.qos       memory.zswap.writeback         system.slice
cgroup.pressure         cpuset.mems.effective  io.pressure       misc.capacity                  user.slice
cgroup.procs            cpu.stat               io.prio.class     misc.current
cgroup.stat             cpu.stat.local         io.stat           proc-sys-fs-binfmt_misc.mount
cgroup.subtree_control  dev-hugepages.mount    memory.numa_stat  sys-fs-fuse-connections.mount
cgroup.threads          dev-mqueue.mount       memory.pressure   sys-kernel-config.mount

# 创建一个子组用于控制测试
sudo mkdir /sys/fs/cgroup/testCgroupV2

# 自动生成该子组下对应的资源限制文件
ls /sys/fs/cgroup/testCgroupV2

# cgroup.controllers:当前层级中可用的控制器（如 cpu、memory 等）
# cgroup.subtree_control:控制哪些控制器被允许在子层级中使用
# cgroup.procs:文件中保存的是属于该cgroup的所有进程的 PID（Process ID），你可以通过向这个文件写入PID，将某个进程加入到该cgroup中
# cpu.max:设置最大CPU时间配额或配额和周期，比如50000 100000，每 100000 微秒（即 100 毫秒）为一个调度周期；在这个周期内，该cgroup最多运行50000微秒（即 50 毫秒）；相当于限制CPU使用率为50% 
cgroup.controllers      cpu.pressure                     hugetlb.2MB.events        memory.low            memory.zswap.max
cgroup.events           cpuset.cpus                      hugetlb.2MB.events.local  memory.max            memory.zswap.writeback
cgroup.freeze           cpuset.cpus.effective            hugetlb.2MB.max           memory.min            misc.current
cgroup.kill             cpuset.cpus.exclusive            hugetlb.2MB.numa_stat     memory.numa_stat      misc.events
cgroup.max.depth        cpuset.cpus.exclusive.effective  hugetlb.2MB.rsvd.current  memory.oom.group      misc.max
cgroup.max.descendants  cpuset.cpus.partition            hugetlb.2MB.rsvd.max      memory.peak           pids.current
cgroup.pressure         cpuset.mems                      io.max                    memory.pressure       pids.events
cgroup.procs            cpuset.mems.effective            io.pressure               memory.reclaim        pids.max
cgroup.stat             cpu.stat                         io.prio.class             memory.stat           pids.peak
cgroup.subtree_control  cpu.stat.local                   io.stat                   memory.swap.current   rdma.current
cgroup.threads          cpu.uclamp.max                   io.weight                 memory.swap.events    rdma.max
cgroup.type             cpu.uclamp.min                   memory.current            memory.swap.high
cpu.idle                cpu.weight                       memory.events             memory.swap.max
cpu.max                 cpu.weight.nice                  memory.events.local       memory.swap.peak
cpu.max.burst           hugetlb.2MB.current              memory.high               memory.zswap.current

# 启用cpu控制器
echo "+cpu" | sudo tee /sys/fs/cgroup/testCgroupV2/cgroup.subtree_control

# 将当前 shell 加入这个组（仅用于测试）
echo $$ | sudo tee /sys/fs/cgroup/testCgroupV2/cgroup.procs

# 设置CPU配额
echo 50000 100000 | sudo tee /sys/fs/cgroup/testCgroupV2/cpu.max
````

Docker会创建一个子目录也就是子控制组，然后将容器的PID写入到该子控制组的cgroup.procs文件中即可，其他资源控制文件内容，可根据docker run的参数指定比如：
````shell
# 创建一个容器
docker run -it --cpu-period=100000 --cpu-quota=20000 ubuntu /bin/bash

# 这个容器的ID是750081271d43
Unable to find image 'ubuntu:latest' locally
latest: Pulling from library/ubuntu
d9d352c11bbd: Pull complete 
Digest: sha256:b59d21599a2b151e23eea5f6602f4af4d7d31c4e236d22bf0b62b86d2e386b8f
Status: Downloaded newer image for ubuntu:latest
root@750081271d43:/# 

# 找到容器完整的ID
docker inspect 750081271d43 | grep Id
"Id": "750081271d43916927516df4f2a4404a0c2a94e96a2aa6d9d8f9d9e7db4c7751",

# 查看Docker为该容器创建的Cgroups控制组
ls /sys/fs/cgroup/system.slice/docker-750081271d43916927516df4f2a4404a0c2a94e96a2aa6d9d8f9d9e7db4c7751.scope

cgroup.controllers      cpu.pressure                     hugetlb.2MB.events        memory.low            memory.zswap.max
cgroup.events           cpuset.cpus                      hugetlb.2MB.events.local  memory.max            memory.zswap.writeback
cgroup.freeze           cpuset.cpus.effective            hugetlb.2MB.max           memory.min            misc.current
cgroup.kill             cpuset.cpus.exclusive            hugetlb.2MB.numa_stat     memory.numa_stat      misc.events
cgroup.max.depth        cpuset.cpus.exclusive.effective  hugetlb.2MB.rsvd.current  memory.oom.group      misc.max
cgroup.max.descendants  cpuset.cpus.partition            hugetlb.2MB.rsvd.max      memory.peak           pids.current
cgroup.pressure         cpuset.mems                      io.max                    memory.pressure       pids.events
cgroup.procs            cpuset.mems.effective            io.pressure               memory.reclaim        pids.max
cgroup.stat             cpu.stat                         io.prio.class             memory.stat           pids.peak
cgroup.subtree_control  cpu.stat.local                   io.stat                   memory.swap.current   rdma.current
cgroup.threads          cpu.uclamp.max                   io.weight                 memory.swap.events    rdma.max
cgroup.type             cpu.uclamp.min                   memory.current            memory.swap.high
cpu.idle                cpu.weight                       memory.events             memory.swap.max
cpu.max                 cpu.weight.nice                  memory.events.local       memory.swap.peak
cpu.max.burst           hugetlb.2MB.current              memory.high               memory.zswap.current

# 查看run参数是否写入到了该控制组的cpu.max内
cat /sys/fs/cgroup/system.slice/docker-750081271d43916927516df4f2a4404a0c2a94e96a2aa6d9d8f9d9e7db4c7751.scope/cpu.max

# 显示
20000 100000
````
