# Docker基础命令

## 镜像
### 查找镜像
````shell
docker search ubuntu

NAME                             DESCRIPTION                                      STARS     OFFICIAL
ubuntu                           Ubuntu is a Debian-based Linux operating sys…   17590     [OK]
ubuntu/squid                     Squid is a caching proxy for the Web. Long-t…   114       
ubuntu/nginx                     Nginx, a high-performance reverse proxy & we…   129       
ubuntu/cortex                    Cortex provides storage for Prometheus. Long…   4         
ubuntu/kafka                     Apache Kafka, a distributed event streaming …   53        
ubuntu/apache2                   Apache, a secure & extensible open-source HT…   95        
ubuntu/prometheus                Prometheus is a systems and service monitori…   72        
ubuntu/bind9                     BIND 9 is a very flexible, full-featured DNS…   107       
.....
````
- DESCRIPTION：镜像介绍
- STARS：用户对镜像评价指数
- OFFICIAL：是否是官方发布

如果你想在不拉取镜像的情况下查看一个镜像的所有 tags，可以访问 Docker Hub 并搜索对应的镜像，然后点击进入详情页，在 "Tags" 标签页中查看所有可用的 tags。

### 下载镜像
````shell
# 这个试验中latest是指24.04版本
docker pull ubuntu:latest

# 显示
latest: Pulling from library/ubuntu
2f074dc76c5d: Pull complete 
Digest: sha256:6015f66923d7afbc53558d7ccffd325d43b4e249f41a6e93eef074c9505d2233
Status: Downloaded newer image for ubuntu:latest
docker.io/library/ubuntu:latest
````

### 列出本地镜像
````shell
docker images

# 显示
REPOSITORY   TAG       IMAGE ID       CREATED       SIZE
ubuntu       latest    6015f66923d7   3 weeks ago   139MB
````

### 删除镜像
````shell
docker rmi ubuntu

# 显示
Untagged: ubuntu:latest
Deleted: sha256:6015f66923d7afbc53558d7ccffd325d43b4e249f41a6e93eef074c9505d2233
````

### 查看镜像详细信息
````shell
docker inspect ubuntu

# 显示
[
    {
        "Id": "sha256:6015f66923d7afbc53558d7ccffd325d43b4e249f41a6e93eef074c9505d2233",
        "RepoTags": [
            "ubuntu:latest"
        ],
        "RepoDigests": [
            "ubuntu@sha256:6015f66923d7afbc53558d7ccffd325d43b4e249f41a6e93eef074c9505d2233"
        ],
        "Parent": "",
        "Comment": "",
        "Created": "2025-04-28T09:45:01.678790458Z",
        "DockerVersion": "",
        "Author": "",
        "Config": {
            "Hostname": "",
            "Domainname": "",
            "User": "",
            "AttachStdin": false,
            "AttachStdout": false,
            "AttachStderr": false,
            "Tty": false,
            "OpenStdin": false,
            "StdinOnce": false,
            "Env": [
                "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
            ],
            "Cmd": [
                "/bin/bash"
            ],
            "Image": "",
            "Volumes": null,
            "WorkingDir": "",
            "Entrypoint": null,
            "OnBuild": null,
            "Labels": {
                "org.opencontainers.image.ref.name": "ubuntu",
                "org.opencontainers.image.version": "24.04"
            }
        },
        "Architecture": "arm64",
        "Variant": "v8",
        "Os": "linux",
        "Size": 28856302,
        "GraphDriver": {
            "Data": null,
            "Name": "overlayfs"
        },
        "RootFS": {
            "Type": "layers",
            "Layers": [
                "sha256:dcb77598f9351d02d5cb14fb3b3d4ed3d206d83b9f9b594bf5470f8c3fa25890"
            ]
        },
        "Metadata": {
            "LastTagTime": "2025-05-26T04:52:14.824242253Z"
        },
        "Descriptor": {
            "mediaType": "application/vnd.oci.image.index.v1+json",
            "digest": "sha256:6015f66923d7afbc53558d7ccffd325d43b4e249f41a6e93eef074c9505d2233",
            "size": 6688
        }
    }
]
````

### 构建镜像（commit）
````shell
# -i：保持容器的标准输入（STDIN）打开，允许你与容器交
# -t: 分配一个伪终端（pseudo-TTY），相当于提供一个终端界面。通常和-i一起使用，进入交互模式。
# ubuntu：镜像名称
# /bin/bash：要执行的命令
# 这个命令执行之后会自动用root为当前容器的用户
# -d：在后台运行容器，并让容器“脱离”当前终端会话。
# -itd：是上面-i -t -d的组合，意思是启动一个容器，分配一个交互式终端（可以进入 shell），但让它在后台运行，需要时可用exec命令进入该容器shell。
docker run -i -t ubuntu /bin/bash
````

进入容器
````shell
# 显示
root@6597ca71f17e:/# 

root@6597ca71f17e:/# apt update
root@6597ca71f17e:/# apt upgrade
root@6597ca71f17e:/# apt install htop wget  telnet net-tools iproute2 iputils-ping apache2 -y

# 退出容器
root@6597ca71f17e:/# exit
````

````shell
# 查看容器 -a：无论容器什么状态都显示出来
docker ps -a

CONTAINER ID   IMAGE     COMMAND       CREATED         STATUS                      PORTS     NAMES
6597ca71f17e   ubuntu    "/bin/bash"   6 minutes ago   Exited (0) 13 seconds ago             vibrant_mclaren
````

将使用这个容器创建新的镜像
````shell
docker commit 6597ca71f17e basic-ubuntu

# 显示
sha256:9f6c790e38ca1b7eeb3d4059001583fd4ebd81bb6903eab585112604793589d0

docker images

# 显示
REPOSITORY     TAG       IMAGE ID       CREATED          SIZE
basic-ubuntu   latest    9f6c790e38ca   35 seconds ago   394MB
ubuntu         latest    6015f66923d7   4 weeks ago      139MB
````

### 修改镜像标签

````shell
docker tag basic-ubuntu:latest my-ubuntu:v1.0

docker images

# 显示 内容不改只是标签改变的话，两个镜像会共用一个存储空间
REPOSITORY     TAG       IMAGE ID       CREATED         SIZE
basic-ubuntu   latest    9f6c790e38ca   3 minutes ago   394MB
my-ubuntu      v1.0      9f6c790e38ca   3 minutes ago   394MB
ubuntu         latest    6015f66923d7   4 weeks ago     139MB
````

## 容器
### 创建容器

````shell
# docker create [参数] image
# --add-host=[]：指定主机域名到IP地址的映射关系，格式是域名:ip
# --dns=[]：为容器指定域名服务器
# -h：为容器指定主机名
# -i：打开容器的标准输入
# --name：指定容器名
# -u,--user=：创建用户

docker create basic-ubuntu

# 显示
7cf3652c51979b6b6b36fc8e997b0313b727613feb97fbf10ab10d0f87b139eb

docker ps -a

# 显示
CONTAINER ID   IMAGE          COMMAND       CREATED          STATUS                      PORTS     NAMES
7cf3652c5197   basic-ubuntu   "/bin/bash"   21 seconds ago   Created                               quirky_cray
6597ca71f17e   ubuntu         "/bin/bash"   25 minutes ago   Exited (0) 19 minutes ago             vibrant_mclaren

# 用run命令创建容器
docker run -it --name demo-ubuntu my-ubuntu:v1.0 /bin/bash

# 显示
root@a88660783ab0:/# exit

# 查看容器
docker ps -a
CONTAINER ID   IMAGE            COMMAND       CREATED              STATUS                      PORTS     NAMES
a88660783ab0   my-ubuntu:v1.0   "/bin/bash"   About a minute ago   Exited (0) 51 seconds ago             demo-ubuntu
7cf3652c5197   basic-ubuntu     "/bin/bash"   5 minutes ago        Created                               quirky_cray
6597ca71f17e   ubuntu           "/bin/bash"   30 minutes ago       Exited (0) 24 minutes ago             vibrant_mclaren

# 启动停止容器
# docker start/restart/stop/kill 容器名称或容器ID，kill会丢失数据
docker start  quirky_cray

docker ps -a 

# 显示
CONTAINER ID   IMAGE            COMMAND       CREATED          STATUS                      PORTS     NAMES
a88660783ab0   my-ubuntu:v1.0   "/bin/bash"   4 minutes ago    Exited (0) 3 minutes ago              demo-ubuntu
7cf3652c5197   basic-ubuntu     "/bin/bash"   8 minutes ago    Exited (0) 8 seconds ago              quirky_cray
6597ca71f17e   ubuntu           "/bin/bash"   32 minutes ago   Exited (0) 26 minutes ago             vibrant_mclaren

# 删除容器
docker rm vibrant_mclaren

docker ps -a

# 显示
CONTAINER ID   IMAGE            COMMAND       CREATED          STATUS                     PORTS     NAMES
a88660783ab0   my-ubuntu:v1.0   "/bin/bash"   6 minutes ago    Exited (0) 6 minutes ago             demo-ubuntu
7cf3652c5197   basic-ubuntu     "/bin/bash"   10 minutes ago   Exited (0) 2 minutes ago             quirky_cray
````

## 网络模式

- host模式：使用Docker Desktop的Network（不支持端口映射 -p），Docker Desktop使用NAT连接宿主机网络（例如容器 <---> Docker虚拟机（Linux） <--NAT--> macOS <--WiFi/Ethernet--> 外网）
- container：新创建的容器和已经存在的一个容器共享一个Network
- none：容器拥有自己的Network但IP、网卡、DNS需要手动设置
- bridge：这是默认模式，容器有自己的Network，并连接到一个虚拟网桥上，由网桥连接网络（例如：容器 → Docker 桥接网络（NAT）→ Linux VM → macOS → 外网）
- 注意：IP地址是随机分配的，每次启动可能会发生变化可用--ip来固定IP，但需要none模式
- 创建容器是使用--network参数指定网络类型
- 在Mac+Docker Desktop环境下，host模式和bridge模式看起来相似，是因为它们都在一个Linux 虚拟机中运行；但本质上，host模式共享网络栈，而bridge模式是隔离的。
````shell
# 使用bridge模式
docker run -itd --name=T1 basic-ubuntu
docker exec T1 ip a show

# 关键显示 说明IP地址是172.17.0.2
11: eth0@if19: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 65535 qdisc noqueue state UP group default 
    link/ether 76:34:fe:88:8a:64 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.2/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever

docker run -itd --name=T2 basic-ubuntu
docker exec T2 ip a show

# 关键显示 说明IP地址是172.17.0.3
11: eth0@if20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 65535 qdisc noqueue state UP group default 
    link/ether 46:a9:be:26:4d:22 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 172.17.0.3/16 brd 172.17.255.255 scope global eth0
       valid_lft forever preferred_lft forever
       
# 两个虚拟机互联
docker exec T1 ping 172.17.0.3

PING 172.17.0.3 (172.17.0.3) 56(84) bytes of data.
64 bytes from 172.17.0.3: icmp_seq=1 ttl=64 time=0.145 ms
64 bytes from 172.17.0.3: icmp_seq=2 ttl=64 time=0.090 ms
64 bytes from 172.17.0.3: icmp_seq=3 ttl=64 time=0.136 ms

docker exec T2 ping 172.17.0.2

PING 172.17.0.2 (172.17.0.2) 56(84) bytes of data.
64 bytes from 172.17.0.2: icmp_seq=1 ttl=64 time=0.053 ms
64 bytes from 172.17.0.2: icmp_seq=2 ttl=64 time=0.133 ms
64 bytes from 172.17.0.2: icmp_seq=3 ttl=64 time=0.141 ms

# 容器与外部网络通过虚拟网桥可以互联
docker exec T2 ping www.baidu.com

PING www.baidu.com (110.242.69.21) 56(84) bytes of data.
64 bytes from 110.242.69.21: icmp_seq=1 ttl=63 time=12.9 ms
64 bytes from 110.242.69.21: icmp_seq=2 ttl=63 time=17.0 ms
64 bytes from 110.242.69.21: icmp_seq=3 ttl=63 time=25.8 ms

# 创建一个Apache HTTP Server并将主机的80端口映射到容器的80端
docker run -itd -p 80:80 httpd 

# 显示
Unable to find image 'httpd:latest' locally
latest: Pulling from library/httpd
d119a2961c84: Pull complete 
b16f1b166780: Pull complete 
4f4fb700ef54: Pull complete 
ba8579c915c0: Pull complete 
6ac56f068528: Pull complete 
00eb3240bbd3: Pull complete 
Digest: sha256:09cb4b94edaaa796522c545328b62e9a0db60315c7be9f2b4e02204919926405
Status: Downloaded newer image for httpd:latest
dfeac3ee7d16cd529122f7956660595771d5f28a9cf0c82065acf687f454322f

# 访问本机80端口
curl http://192.168.0.10

# 正常打开网页
<html><body><h1>It works!</h1></body></html>
````