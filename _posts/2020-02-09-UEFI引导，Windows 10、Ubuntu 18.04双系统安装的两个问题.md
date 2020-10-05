---
layout: post
title:  "UEFI引导，Windows 10、Ubuntu 18.04双系统安装的两个问题"
date:    2020-02-09 12:45:14 +0800
categories: jekyll update
---
Ubuntu的安装器能够识别出已经存在的Windows系统，可以选择共存的方式安装，但是不知道是如何修改分区的，如果选择自定义分区，那么所有的分区、挂载点都要自己定义，没有一种先自动分配再自定义修改的方式，相比之下不是很友好。另外，自定义分区时没办法创建逻辑卷（VG、LV），也不友好。不过，现在Linux桌面版基本上都是先进Live再运行安装程序，分区问题可以事先准备好。Ubuntu 18.0.4 Live CD自带Gparted真的只是分区工具，不能用来管理LV，要用LVM只能命令行。这里顺便解释一下BOOT分区的问题，网上有的说要有专门的引导分区，有的说UEFI模式下只需要EFI分区，无须BOOT分区。UEFI模式下，EFI分区肯定是必须的。至于BOOT分区，单独分出来实际上是解决早期引导器只能从硬盘前1024柱面读取数据的问题，在硬盘前面部分分一个BOOT分区存放系统内核，就可以解决此问题。但现在应该不存在这个问题了，所以已经没必要单独分一个BOOT分区了。但是如果采用LVM逻辑卷管理并且把根挂载到LV上，由于引导器（GRUB2）不支持从LV挂载文件系统写，所以不能使用引导管理器的写入功能，如引导上一次引导的系统功能就需要保存上一次成功的引导项到硬盘上，正常引导倒是可以的。单独分一个BOOT分区需要注意分区大小，200M非常紧张，但是太大了又没必要。

Ubuntu安装器的分区工具还有一个问题是能够识别出EFI分区，可以选择将引导器安装到这个分区上，但是可能是BUG，安装到最后总是出一个不能将GRUB安装到/Target的错误，直接将EFI挂载到/boot/efi也不行，会提示没有指定EFI分区。所以这个问题没办法，只能安装结束后再安装GRUB。但是事实上系统已经是安装好了的，可以通过CMOS直接选择EFI文件引导的。重装GRUB步骤如下：
```bash
sudo mount /dev/sdXXX /mnt
sudo mount /dev/sdXY /mnt/boot
sudo mount /dev/sdXX /mnt/boot/efi
for i in /dev /dev/pts /proc /sys /run; do sudo mount -B $i /mnt$i; done
sudo chroot /mnt
grub-install /dev/sdX
update-grub
```
其中：sdX = disk | sdXX = efi partition | sdXY = boot partition | sdXXX = system partition。grub-install后面的参数可以省略。

装好系统第一件事就是修改源为国内源，这个网上一找一大堆教程，不过都只是给了链接，这里需要注意，sources.list里面的格式，其中有一个是表示版本的，需要用lsb_release -a查看codename，替换后再添加。

完了之后还有一个BUG，是在apt升级之后，重装了几次都有同样的问题，从提示来看也跟GRUB有关，但解决方案却是dpkg的问题：
```
$ sudo apt update
	dpkg: 处理软件包 shim-signed (--configure)时出错：
    依赖关系问题 - 仍未被配置
	因为错误消息指示这是由于上一个问题导致的错误，没有写入 apport 报告。
	 在处理时有错误发生：
	 grub-efi-amd64-signed
	 shim-signed
```
解决办法如下：
```
#1.重命名dpkg目录下的info目录
$ sudo mv /var/lib/dpkg/info /var/lib/dpkg/info.bak
#2.创建一个新的info文件夹
$ sudo mkdir /var/lib/dpkg/info
#3.执行更新操作
$ sudo apt-get update && sudo apt-get -f install
#4.将更新操作产生的文件，全部复制到重命名的info.bak文件夹下
$ sudo mv /var/lib/dpkg/info/* /var/lib/dpkg/info.bak
#5.删除创建的info文件夹
$ sudo rm -rf /var/lib/dpkg/info
#6.将重命名的info.bak文件夹重新改回info
$ sudo mv /var/lib/dpkg/info.bak /var/lib/dpkg/info
#7.再次执行更新操作，问题解决
$  sudo apt-get update && sudo apt-get upgrade
```