---
layout: post
title:  "修改Nvidia WebDriver驱动程序，支持当前macOS版本"
date:    2019-01-24 18:02:37 +0800
categories: jekyll update
---
每个Nvidia WebDriver驱动，只支持唯一一个macOS版本，检查操作系统版本时精确到小版本号，也就是说，打一个系统补丁，就有可能使得驱动不能使用。这样在系统提示有更新时，都还要看看WebDrive驱动放出来了没有，很别扭。还好，国外有人写了个叫NVIDIA WebDriver Updater的程序，用于下载最新的WebDriver驱动，同时，如果下载安装的WebDriver与当前系统版本不兼容，还提供修改和补丁的功能。这个程序可以从https://mac.softpedia.com/get/System-Utilities/NVIDIA-WebDriver-Updater.shtml下载。

只不过，用这个程序的话就只知其然而不知其所以然，网上搜索相关问题，下载工具或修改好的驱动的多，且多数需要注册会员才能下载，而详细解说如何手工修改的不多，因此，把一篇好不容易找到的文章搬运过来。修改的大致过程如下：
### 一、修改安装程序
如果原来已经安装过WebDriver驱动，只是因为升级系统而不能驱动的话，可以不用重新下载和安装驱动，自然也就不需要修改安装程序。修改安装程序实际上就是让安装程序跳过检查操作系统版本的步骤。
```
sudo -s #進入Single User模式
pkgutil --expand ~/downloads/NVIDIA_WEBDRIVER.pkg ~/desktop/DriverExtract/ #解开安装包
```
打开Distribution 文件
找到这样一段代码：
```c
function InstallationCheck()
{
if (!validateSoftware()) return false;
return true;
}
```
删除if (!validateSoftware()) return false;这一行
找到这样一段代码：
```c
return false;
}
return true;
}
```
改为
```c
return true;
}
return true;
}
```
pkgutil --flatten ~/Desktop/Web-Driver-Expanded ~/Desktop/Web-Driver-Patched.pkg #重新打包
用新的安装包安装，并不要重新启动，进行下一步修改。
### 二、修改驱动程序
在/Library/Extensions/找到NVDAStartupWeb.kext

打开info.plist

找到NVDARequiredOS 17B1003把这一串字符改成当前系统版本

然后，输入以下命令(这一步是修复权限)：
```bsh
sudo chown -R 0:0 NVDAStartupWeb.kext
sudo chmod -R 755 NVDAStartupWeb.kext
sudo kextcache -i /
```
### 三、在Clover中打补丁
这一步用NVIDIA WebDriver Updater程序的话是生成一个Patch文件。

在Clover Configurator 的Kext To Patch中输入
```
Name：NVDAStartupWeb
Find：4e56444152657175697265644f5300
Replace：000000000000000000000000000000
Comment：Disable NVDARequiredOS
```