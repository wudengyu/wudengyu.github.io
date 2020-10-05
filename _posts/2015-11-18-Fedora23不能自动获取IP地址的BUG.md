---
layout: post
title:  "Fedora 23不能自动获取IP地址的BUG"
date:   2015-11-18 22:37:53 +0800
categories: jekyll update
---
Fedora 23安装好之后，发现网络不通，设置网卡那里是有本地连接的，说明驱动是安装好了的，设置静态IP后网络也可用。说明这是DHCP Client的问题，百度没有结果，这种明显是BUG，文档上肯定不会有，只好上Fedora的官网论坛，全英文啊。还好，根据dhcp关键字，在官网论坛上还是比较容易地找到了答案，根据论坛上说的，的确解决了问题，我就不明白了，看上去是21版就已经存在的问题，为什么23版还存在。原贴地址是https://ask.fedoraproject.org/en/question/62122/no-wired-connection-after-upgrade-to-21/，正确答案内容如下：
```
Your system seems to ask for an IP address (DHCPDISCOVER) and doesn't get one. Probably it's this bug: http://fedoraproject.org/wiki/Common_F21_bugs#Network_issues (IP address discovery via DHCP does not work).

Solution: create a file /etc/dhcp/dhclient.conf with this one line:

send dhcp-client-identifier = hardware;
```
里面有一段解释，意思是说，经过仔细分析，代码是完全符合DHCP协议标准的，问题只是出现在通过某些路由器上的DHCP服务获取地址时存在，没有必要去改代码。