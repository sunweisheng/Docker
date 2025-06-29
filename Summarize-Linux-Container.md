# Linux容器总结

运行环境:Ubuntu 22.04.5 LTS

## 一个简单的Web程序
语言为python，保存文件名为app.py
````python
from flask import Flask
import socket
import os
app = Flask(__name__)
@app.route('/')
def hello():
    html = "<h3>Hello {name}!</h3>" \
           "<b>Hostname:</b> {hostname}<br/>"
    return html.format(name=os.getenv("NAME","world"), hostname=socket.gethostname())

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=80)
````

## 创建一个Dockerfile

创建一个依赖文件requirements.txt
````shell
cat requirements.txt

# 显示
Flask
````
创建一个Dockerfile文件
````dockerfile
# 使用官方提供python开发镜像作为基础镜像
FROM python:2.7-slim

# 将工作目录切换为/app
WORKDIR /app

# 将当前目录下的所有内容复制到/app下
ADD . /app

# 使用pip命令安装这个应用所需要的依赖，在信任pypi.python.org的前提下（不检查 SSL 证书），通过pip安装requirements.txt文件中列出的所有依赖包（这里是 Flask）
RUN pip install --trusted-host pypi.python.org -r requirements.txt

# 允许外界访问容器的80端口
EXPOSE 80

# 设置环境变量
ENV NAME=MyWorld

# 设置容器进程为python app.py,即用这个python应用的启动命令
CMD ["python", "app.py"]
````
以上大写高亮的词语是原语，并且这些原语都是按顺序执行。

````shell
# 在当前目录下准备所需要的文件
ls
# 显示
app.py  Dockerfile  requirements.txt
````

## 构建镜像

````shell
# docker build命令会自动夹在当前目录下的Dockerfile文件，然后依次执行原语，-t参数是给改镜像起一个Tag便于记忆
docker build -t helloworld .

# 显示
[+] Building 16.0s (9/9) FINISHED                                docker:default
 => [internal] load build definition from Dockerfile                       0.0s
 => => transferring dockerfile: 693B                                       0.0s
 => [internal] load metadata for docker.io/library/python:2.7-slim         0.0s
 => [internal] load .dockerignore                                          0.0s
 => => transferring context: 2B                                            0.0s
 => [1/4] FROM docker.io/library/python:2.7-slim                           0.0s
 => [internal] load build context                                          0.0s
 => => transferring context: 850B                                          0.0s
 => CACHED [2/4] WORKDIR /app                                              0.0s
 => [3/4] ADD . /app                                                       0.0s
 => [4/4] RUN pip install --trusted-host pypi.python.org -r requirements  14.8s
 => exporting to image                                                     1.1s 
 => => exporting layers                                                    1.1s 
 => => writing image sha256:491c5aafa60138122460107b4130f309b692e637089e5  0.0s 
 => => naming to docker.io/library/helloworld                              0.0s 

docker images

# 显示
REPOSITORY    TAG        IMAGE ID       CREATED         SIZE                    
helloworld    latest     491c5aafa601   3 minutes ago   158MB
````
Dockerfile中的每个原语执行之后，都会生成一个对应的镜像层，就算没有明显修改文件的操作(ENV原语)，它对应的层也会存在，只不过该层从外界看是空的。

````shell
docker inspect helloworld
````

````json
[
    {
        "Id": "sha256:491c5aafa60138122460107b4130f309b692e637089e5ad8b5610b2c32ffeab0",
        "RepoTags": [
            "helloworld:latest"
        ],
        "RepoDigests": [],
        "Parent": "",
        "Comment": "buildkit.dockerfile.v0",
        "Created": "2025-06-29T22:12:18.305380716+08:00",
        "DockerVersion": "",
        "Author": "",
        "Config": {
            "ExposedPorts": {
                "80/tcp": {}
            },
            "Env": [
                "PATH=/usr/local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
                "LANG=C.UTF-8",
                "PYTHONIOENCODING=UTF-8",
                "GPG_KEY=C01E1CAD5EA2C4F0B8E3571504C367C218ADD4FF",
                "PYTHON_VERSION=2.7.18",
                "PYTHON_PIP_VERSION=20.0.2",
                "PYTHON_GET_PIP_URL=https://github.com/pypa/get-pip/raw/d59197a3c169cef378a22428a3fa99d33e080a5d/get-pip.py",
                "PYTHON_GET_PIP_SHA256=421ac1d44c0cf9730a088e337867d974b91bdce4ea2636099275071878cc189e",
                "NAME=MyWorld"
            ],
            "Cmd": [
                "python",
                "app.py"
            ],
            "WorkingDir": "/app",
            "ArgsEscaped": true
        },
        "Architecture": "amd64",
        "Os": "linux",
        "Size": 158104307,
        "GraphDriver": {
            "Data": {
                "LowerDir": "/var/lib/docker/overlay2/k0vi7p2a0uu78spdt6mygrncr/diff:/var/lib/docker/overlay2/txelit5d4miwbk498bps1zycz/diff:/var/lib/docker/overlay2/f8fe47e8d80d534508d884f61f25c13a076f8e44f4d4c4cf70625a0dbea8a9f1/diff:/var/lib/docker/overlay2/eff62084c1057500d86c71a070434c850cd0537015754501bdb78eb01701b08e/diff:/var/lib/docker/overlay2/fdcb013f1830b91618a20c180aad46799f06e4374dc9b6ce8fdd1dcb649f03af/diff:/var/lib/docker/overlay2/b9a1d0a1e8bf21761ce456855eee53baba93de0b84a63b4bf5c316879cbc8ae8/diff",
                "MergedDir": "/var/lib/docker/overlay2/tb5th6b602pgv3guofa5wnyt1/merged",
                "UpperDir": "/var/lib/docker/overlay2/tb5th6b602pgv3guofa5wnyt1/diff",
                "WorkDir": "/var/lib/docker/overlay2/tb5th6b602pgv3guofa5wnyt1/work"
            },
            "Name": "overlay2"
        },
        "RootFS": {
            "Type": "layers",
            "Layers": [
                "sha256:b60e5c3bcef2f42ec42648b3acf7baf6de1fa780ca16d9180f3b4a3f266fe7bc",
                "sha256:568944187d9378b07cf2e2432115605b71c36ef566ec77fbf04516aab0bcdf8e",
                "sha256:7ea2b60b0a086d9faf2ba0a52d4e2f940d9361ed4179642686d1d8b59460667c",
                "sha256:7a287aad297b39792ee705ad5ded9ba839ee3f804fa3fb0b81bb8eb9f9acbf88",
                "sha256:a076659d1e1d45b265f9e93dce0583b97808fb5ecaa7cdfe239f5dcca31c5a18",
                "sha256:4c8c4926906f70501c5740e2b9790b15ecb8b51cb56a2d5967c66c6b122d6b04",
                "sha256:5def38db0020de808aabcfdbc2dda30d343d6b57a97cd403d7ee0a003f668341"
            ]
        },
        "Metadata": {
            "LastTagTime": "2025-06-29T22:38:20.992008831+08:00"
        }
    }
]
````
LowerDir(虽然不知道这些层如何与原语一一对应，但从层数和内容上看基本上是对应了原语的操作步骤):
````shell
sudo ls -l /var/lib/docker/overlay2/k0vi7p2a0uu78spdt6mygrncr/diff
drwxr-xr-x 2 root root 4096  6月 29 22:12 app

sudo ls -l /var/lib/docker/overlay2/k0vi7p2a0uu78spdt6mygrncr/diff/app
-rw-rw-r-- 1 root root   335  6月 29 21:42 app.py
-rw-rw-r-- 1 root root   654  6月 29 22:11 Dockerfile
-rwxrwxr-x 1 root root 16320  6月 29 01:03 ns
-rwxrwxr-x 1 root root 16360  6月 29 01:41 ns-1
-rw-rw-r-- 1 root root  1511  6月 29 01:41 ns-1.c
-rw-rw-r-- 1 root root  1030  6月 29 01:01 ns.c
-rw-rw-r-- 1 root root     6  6月 29 21:46 requirements.txt

sudo ls -l /var/lib/docker/overlay2/txelit5d4miwbk498bps1zycz/diff
drwxr-xr-x 2 root root 4096  6月 29 22:08 app

sudo ls -l /var/lib/docker/overlay2/txelit5d4miwbk498bps1zycz/diff/app
总计 0

sudo ls -l /var/lib/docker/overlay2/f8fe47e8d80d534508d884f61f25c13a076f8e44f4d4c4cf70625a0dbea8a9f1/diff
drwxr-xr-x 2 root root 4096  4月 21  2020 etc
drwx------ 2 root root 4096  4月 21  2020 root
drwxrwxrwt 2 root root 4096  4月 21  2020 tmp
drwxr-xr-x 6 root root 4096  4月 14  2020 usr
drwxr-xr-x 5 root root 4096  4月 14  2020 var

sudo ls -l /var/lib/docker/overlay2/eff62084c1057500d86c71a070434c850cd0537015754501bdb78eb01701b08e/diff
drwxr-xr-x 2 root root 4096  4月 21  2020 bin
drwxr-xr-x 6 root root 4096  4月 21  2020 etc
drwxr-xr-x 3 root root 4096  4月 21  2020 lib
drwx------ 2 root root 4096  4月 21  2020 root
drwxrwxrwt 2 root root 4096  4月 21  2020 tmp
drwxr-xr-x 9 root root 4096  4月 14  2020 usr
drwxr-xr-x 5 root root 4096  4月 14  2020 var

sudo ls -l /var/lib/docker/overlay2/fdcb013f1830b91618a20c180aad46799f06e4374dc9b6ce8fdd1dcb649f03af/diff
drwxr-xr-x 4 root root 4096  4月 17  2020 etc
drwxrwxrwt 2 root root 4096  4月 17  2020 tmp
drwxr-xr-x 7 root root 4096  4月 14  2020 usr
drwxr-xr-x 5 root root 4096  4月 14  2020 var

sudo ls -l /var/lib/docker/overlay2/b9a1d0a1e8bf21761ce456855eee53baba93de0b84a63b4bf5c316879cbc8ae8/diff
drwxr-xr-x  2 root root 4096  4月 14  2020 bin
drwxr-xr-x  2 root root 4096  2月  2  2020 boot
drwxr-xr-x  2 root root 4096  4月 14  2020 dev
drwxr-xr-x 28 root root 4096  4月 14  2020 etc
drwxr-xr-x  2 root root 4096  2月  2  2020 home
drwxr-xr-x  7 root root 4096  4月 14  2020 lib
drwxr-xr-x  2 root root 4096  4月 14  2020 lib64
drwxr-xr-x  2 root root 4096  4月 14  2020 media
drwxr-xr-x  2 root root 4096  4月 14  2020 mnt
drwxr-xr-x  2 root root 4096  4月 14  2020 opt
drwxr-xr-x  2 root root 4096  2月  2  2020 proc
drwx------  2 root root 4096  4月 14  2020 root
drwxr-xr-x  3 root root 4096  4月 14  2020 run
drwxr-xr-x  2 root root 4096  4月 14  2020 sbin
drwxr-xr-x  2 root root 4096  4月 14  2020 srv
drwxr-xr-x  2 root root 4096  2月  2  2020 sys
drwxrwxrwt  2 root root 4096  4月 14  2020 tmp
drwxr-xr-x 10 root root 4096  4月 14  2020 usr
drwxr-xr-x 11 root root 4096  4月 14  2020 var
````

UpperDir:
````shell
sudo ls -l /var/lib/docker/overlay2/tb5th6b602pgv3guofa5wnyt1/diff
drwxr-xr-x 2 root root 4096  6月 29 22:12 etc
drwx------ 3 root root 4096  6月 29 22:12 root
drwxrwxrwt 2 root root 4096  6月 29 22:12 tmp
drwxr-xr-x 3 root root 4096  4月 14  2020 usr
````

## 启动容器

````shell
docker run -p 4000:80 helloworld

# 显示
* Serving Flask app "app" (lazy loading)
 * Environment: production
   WARNING: This is a development server. Do not use it in a production deployment.
   Use a production WSGI server instead.
 * Debug mode: off
 * Running on http://0.0.0.0:80/ (Press CTRL+C to quit)
 
 docker ps
 
 # 显示
CONTAINER ID   IMAGE        COMMAND           CREATED         STATUS         PORTS                                     NAMES
6c8ef380835e   helloworld   "python app.py"   2 minutes ago   Up 2 minutes   0.0.0.0:4000->80/tcp, [::]:4000->80/tcp   bold_turing

curl http://localhost:4000
# 显示
<h3>Hello MyWorld!</h3><b>Hostname:</b> 6c8ef380835e<br/>sunweisheng@agent-home:~$ 
````

## Docker Exec

````shell
docker exec -it 6c8ef380835e /bin/sh
ls
# 显示
Dockerfile  app.py   requirements.txt

touch text.txt
ls
# 显示
Dockerfile  app.py  requirements.txt  text.txt

exit
````
docker exec 意味着一个进程可以选择加入已有的某个Namespace当中，从而“进入”该进程所在容器。

````shell
# docker inspect提供的一个用于查看容器、镜像、卷等详细信息的命令，.State.Pid 表示要提取容器状态中的 主进程 PID
docker inspect --format '{{ .State.Pid}}' 6c8ef380835e

# 显示 运行容器的PID是5565
5565

# 查看宿主机的proc文件，查看5565进程所有Namespace对应的文件
sudo ls -l /proc/5565/ns

# 显示
lrwxrwxrwx 1 root root 0  6月 30 00:28 cgroup -> 'cgroup:[4026532508]'
lrwxrwxrwx 1 root root 0  6月 30 00:28 ipc -> 'ipc:[4026532506]'
lrwxrwxrwx 1 root root 0  6月 30 00:20 mnt -> 'mnt:[4026532503]'
lrwxrwxrwx 1 root root 0  6月 30 00:20 net -> 'net:[4026532509]'
lrwxrwxrwx 1 root root 0  6月 30 00:28 pid -> 'pid:[4026532507]'
lrwxrwxrwx 1 root root 0  6月 30 00:36 pid_for_children -> 'pid:[4026532507]'
lrwxrwxrwx 1 root root 0  6月 30 00:36 time -> 'time:[4026531834]'
lrwxrwxrwx 1 root root 0  6月 30 00:36 time_for_children -> 'time:[4026531834]'
lrwxrwxrwx 1 root root 0  6月 30 00:36 user -> 'user:[4026531837]'
lrwxrwxrwx 1 root root 0  6月 30 00:28 uts -> 'uts:[4026532505]'
````
一个进程的每种Linux Namespace都在它对应的/proc/进程号/ns 下有一个对应的虚拟文件，并且链接到一个真实的Namespace文件上，我们就可以对Namespace做一些有意义的事情了，比如加入一个已经存在的Namespace当中。

这段 C 语言代码的作用是 进入一个已经存在的 Linux 命名空间（namespace） ，然后在那个命名空间中执行一个新的命令。它使用了 setns() 系统调用来实现这个功能。
````c++
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
````

编译并执行：
````shell
nano set_ns.c

gcc -o set_ns set_ns.c

sudo ./set_ns /proc/5565/ns/net /bin/bash

# 进入容器Namespace
ifconfig

# 显示 这不是宿主机的网络配置，这是6c8ef380835e容器的网络配置
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.17.0.2  netmask 255.255.0.0  broadcast 172.17.255.255
        ether 1e:a2:4c:eb:3e:67  txqueuelen 0  (以太网)
        RX packets 49  bytes 4976 (4.9 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 16  bytes 1155 (1.1 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (本地环回)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
````

在宿主机上执行：
````shell
ps aux | grep /bin/bash

# 显示
root        6135  0.0  0.0  17680  6224 pts/1    S+   00:56   0:00 sudo ./set_ns /proc/5565/ns/net /bin/bash
root        6136  0.0  0.0  17680  2532 pts/2    Ss   00:56   0:00 sudo ./set_ns /proc/5565/ns/net /bin/bash
root        6137  0.1  0.0  13460  4224 pts/2    S    00:56   0:00 /bin/bash
root        6225  0.0  0.0  12192  2432 pts/2    S+   01:00   0:00 grep --color=auto /bin/bash

sudo ls -l /proc/5565/ns/net
lrwxrwxrwx 1 root root 0  6月 30 00:20 /proc/5565/ns/net -> 'net:[4026532509]'

sudo ls -l /proc/6137/ns/net
lrwxrwxrwx 1 root root 0  6月 30 01:02 /proc/6137/ns/net -> 'net:[4026532509]'
````
两个进程共享了名为net:[4026532509]的Network Namespace。Docker命令中专门提供了一个参数-net，可以让启动的容器“加入”另一个容器的Network Namespace，也是这个道理（-net=host 意味着共享宿主机的网络栈）。

## Volume(数据卷)

两种Volume声明方式，可以把宿主机目录挂载进容器的/test目录：
````shell
docker run -v /test .....
docker run -v /home:/test .....
````
- 第一种情况下，由于没有显式声明宿主机目录，因此Docker默认在宿主机上创建一个临时目录/var/lib/volumes/[VOLUME_ID]/_data,然后把它挂载到容器的/test目录上。
- 第二种情况下，Docker直接把宿主机的/home目录挂载到了容器的/test目录上。

注意：因为Mount Namespace的隔离作用，导致宿主机并不知道这个挂载的存在（只有容器知道），所以Docker Commit命令发生在宿主机空间，会认为该挂载目录内容始终为空（但会创建对用的层），也就是docker commit命令不会提交volume数据卷的内容。

````shell
docker run -d -v /test helloworld

# 显示
9c7f587bcfc6a13916e934a138bcb3be8be63bb8d558b8bae39f0f7d591aa0da

docker volume ls

# 显示
DRIVER    VOLUME NAME
local     f81a7d64a6648f2107d075dfd481c72c7f6233f6049646c08a40e524928f6b51

# 查看volumes工作目录
sudo ls /var/lib/docker/volumes/f81a7d64a6648f2107d075dfd481c72c7f6233f6049646c08a40e524928f6b51/_data
# 显示为空

# 进入容器
docker exec -it 9c7f587bcfc6 /bin/sh

# 执行命令创建文件
cd /test
touch test.txt
exit

# 回到宿主机查看数据卷目录
sudo ls /var/lib/docker/volumes/f81a7d64a6648f2107d075dfd481c72c7f6233f6049646c08a40e524928f6b51/_data

# 显示
test.txt
````