---
layout: post
title:  "VLAN、Bonding和Bridge"
date:   2016-01-15 01:39:06 +0800
categories: jekyll update
---
Emulex现在有一种叫端口聚合网卡，每个端口（Port）有两个VP（可能是virual port）的意思吧，这两个VP可以一个设成FCoE模式，一个设成NIC模式，这样的话，就可以实现Ethernet与SAN双网合一了。的确，这才是FCoE出现的目的，要不然，就算有FCoE，可以用Ethernet跑FC了，但从服务器出来仍然是不同的接口，走不同的线路，那跟用HBA卡和网卡分开使用有什么区别呢？很明显，这种聚合端口可以收发两种不同的Ethernet数据包，至少要有一种Ethernet数据包带有VLANID才能将FC流量分离出来，所以这种聚合端口上联的交换机端口应当是TRUNK口或者是Hybrid口。这时就需要操作系统的支持，在数据从网络接口发出时就带有VLAN ID，也就是说操作系统需要支持VLAN才行。
    
LINUX通过内核模块8021q支持vlan的802.1q标准协议，配置也不复杂。首先lsmod|grep 8021q看一看内核是否加载了这个模块，如果没有，可以用modprobe 8021q加载，然后就可以用vconfig add [interface-name] [vlan-id]命令来将网口加入到一个VLAN中，例如：vconfig add eth0 100，然后用ifconfig命令可以看到多出一个eth0.100的网络接口，同样可以用ifconfig eth0.100 x.x.x.x x.x.x.x设置其IP地址，vlan口就配置完了，这是动态配置，如果要重启系统之后仍然存在的话，和物理网卡一样，在/etc/sysconfig/network-script/下创建一个eth0.100这样的配置文件，其DEVICE前面部分是物理设备名，然后“.”，然后是VLANID，其中必须有一行VLAN=yes，其余和普通网卡配置差不多，至于8021q模块，不用特殊处理，network server启动时，如果检查到有正确的VLAN配置文件，就会自动加载8021q模块。当在一个物理网口上创建了一个VLAN子接口后，这一个物理网口就相当于交换机上的TRUNK口了。事实上，基于一个物理接口，可以创建出多个VLAN子接口。网上有人说，如果一个物理接口如果添加了VLAN子接口，就不能再配IP地址了。然而，实际上在有VLAN子接口的物理网口上是可以再配置IP地址的，只不过，直接通过物理接口发出的数据是不带VLANID的，通过它的VLAN子接口发出的数据带有VLANID，这时，这个接口就相当于交换机上的Hybrid接口了。

Linux中还可以把几个物理网口绑定起来达到负载均衡或者线路备份效果，可以在bond接口的基础上再创建VLAN子接口，但是，却不能把VLAN子接口绑定在一起。假如说把几个不同的VLAN绑在一起，那么发出的包到底应该是哪个VLAN呢？所以，虽然VLAN和BOND虽然都表现为虚拟接口的形式，但从层次上来说，Linux处理VLAN的层次要比BIND要高一些，而VLAN又可以承载IP，所以VLAN比IP层次低。Linux中还有一种网络设备叫Bridge，Bridge可以把网络接口连接起来，数据在其中转发，就像集线器一样，本来集线器就是一种网桥设备。Bridge可以桥接VLAN接口、Bond接口和物理接口，如果把不同VLAN的接口桥接，显然没有什么意义，因为VLAN的目的就是要隔离，如果想要实现VLAN之间的转发，应该是走IP层的路由才对。这么说来，Bridge的层次已经接近IP层，在Bond、VLAN、Bridge三者中是最高的了。通常只是把一个VLAN接口和物理接口桥接，桥接之后，VLAN接口收到的数据转发到其他物理接口时，会去掉VLAN标记，而物理接口收到的数据转发到VLAN接口之后会打上标记再由其下的物理接口转发出去。

Bridge配置方法如下：先brctl addbr <bridge>建一个桥，然后，brctl addif <bridge> <device>把设备一个一个添进来。静态配置的话，仍然是创建一个ifcfg-br0之类的文件，DEVICE就是桥名，TYPE=Bridge，也可以配置IP地址、掩码、网关，不过这是用来管理桥的，跟桥上的接口应该没什么关系，然后跟BOND一样，把被桥接的接口的配置文件中加一行BRIDGE=br0。不过，一定要记住，虽然前面说桥的层次较高，但始终是一个二层设备，想通过桥实现不同IP子网互通是不现实的，把不同的VLAN打通也是无意义的。