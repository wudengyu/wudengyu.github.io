---
layout: post
title:  "Rsyslog+MySQL+LogAnalyzer架设日志系统"
date:    2018-03-23 11:37:00 +0800
categories: jekyll update
---
## 一、前言
架设日志系统实现收集网络及安全设备、服务器运行日志，其核心就是一台能够接收各种设备日志信息的日志服务器，再加上日志存储以及展现、查询和分析的功能就行了，其实并不复杂。但是现在一套日志系统软件动辄几万甚至几十万，无非就是支持的设备种类多一点、查询功能强一点、展现效果漂亮一点而已。当然，支持的设备多很有用，不同种类、品牌、型号的设备日志格式一般都不会完全一样，集中在一起之后，会显得很乱，不方便查询、分析。但是，几乎所有的设备都支持syslog协议，只是遵循协议标准的程度不同而已，而日志的主要内容就是一些文本，即使格式有些不一致，看上去混乱但其实影响并不大，至于界面就更不是关键的了。所以现在很多人采用Rsyslog+MySQL+LogAnalyzer的方案，采用这套方案的道理很简单，都是开源软件、搭配非常成熟配置起来并不复杂，至于功能、性能、界面等一般也没必要要求太高，再说这几个软件虽说是免费，但是只要用好了其实并不差。
## 二、安装步骤
### （一）服务器安装
当然，采用这套组合的前提是基于Linux系统，虽然MySQL有Windows版，但Rsyslog是没有Windows版的，所以首先得准备一个Linux服务器。这里采用的是Oracle Linux 6.9，可以在安装系统的时候就把Rsyslog、MySQL、Apache HTTP、PHP等一次性安装好，也可以先装基本系统，然后再用yum安装其他软件。这套系统并不需要安装X 桌面。
### （二）日志软件安装和配置
1. 安装Rsyslog。Rsyslog是不需要安装的，装好系统就有，只不过要想将日志写入数据库，需要在服务器上安装Rsyslog的Mysql模块。安装命令如下：

```
yum install rsyslog-sql
```
2. 服务器防火墙配置。syslog支持UDP和TCP协议，默认都采用514端口，所以要配置防火墙打开514端口。

vi/etc/sysconfig/iptables在其中插入下面一行，注意是插入，因为防火墙一般最后都有一条丢弃所有数据包的规则，所以如果允许的规则放在丢弃规则的后面就没作用了。
```
-A INPUT –p udp–m state –state NEW –m udp –dport 514 –j ACCEPT
```
这个是UDP协议的，TCP的类似，一般来说日志漏掉一两条也不会影响什么，不需要多么可靠的连接，就不必用TCP连接了，每发送一条日志还三次握手啥的，太浪费资源。

也可以在这时候顺便把后面要用的HTTP服务的80端口也开了，甚至21也开了，方便传文件。

3. Rsyslog服务配置。Rsyslog配置比较复杂，官网上文档也写得不是很容易懂，网上很多讲Rsyslog配置的也讲得很复杂。这里只关注要点，就是Rsyslog作为服务端来用的话其实就是加载一个网络模块让它可以接收客户端发来的日志，其他的什么消息级别、类别，什么模板、规则等等都是处理日志的，第一次配置的话先不要管，否则的话马上蒙圈，最后还是不知道怎么配。
```
vi /etc/rsyslog.conf
找到#ModLoad imudp
    #UDPServerRun 514
```
去掉行首的#号就行了，第一行表示加载UPD输入模块，第二行表示UDP服务运行在514端口。如果不想用514端口，就修改成别的，但是千万不能和别的应用冲突，另外防火墙也要对应的打开。如果要采用TCP协议，那就找下面的两行，同样去掉#号保存就行了。
```
#ModLoad imtcp
#InputTCPServerRun 514
```
4. Rsyslog客户端配置。Linux下同样是修改/etc/rsyslog.conf，这次是找
```
#*.* @@remote-host:514
```
去掉#号，并且把remote-host改成服务器主机域名，这里要保证主机域名能够被正确解决析，或许需要修改/etc/hosts文件，如果域名不能被正确解析用服务器IP也行，冒号：后面就是服务器端口号，必须和服务端配置一致，默认都是514不需要改。@一个表示采用UDP协议，两个采用TCP协议，如果采用UDP协议，就删掉一个。

Windows下没有Rsyslog软件，采用的日志格式也不是syslog格式，所以要收集Windows系统日志就需要一个软件把Windows日志格式转换成syslog格式并且发送给服务端，这种软件叫syslog代理，Rsyslog Windows Agent比较好用，可惜是收费的，有一个叫NXLog的，功能简单一点但是免费。安装配置就不多说了，一看就能明白的。

至于路由器、交换机、防火墙等网络设备，配置syslog就更没办法具体说。因为不同种类、品牌、型号配置命令都不完全一样，具体的命令只能去看手册，但大致也就是启用syslog功能、指定日志服务器IP地址、端口这两大步而已。

到这里日志采集部分已经完成，如果有Linux客户端，可以用logger命令发送日志进行测试，Windows下Rsyslog Windows Agent也可以直接发送日志，但是NXLog就没有这个功能，NXLog就只能把系统日志发送出去。网络设备通常也没有直接发送日志的功能可以用于测试。如果没办法直接发送日志消息进行测试，那就只有等日志自己产生了，通常要不了几分钟就会有日志产生的。而对于日志服务器来说，只要能收到一个设备发的日志就应该能收到其他设备的消息，所以只需要看看日志服务器上有没有其他设备的消息就可以了。查看日志很简单，默认情况下日志是保存在服务器文件系统里，位置就是看/etc/rsyslog.conf配置文件里面的那些文件夹，一般的日志都是在*.info这一行后面指定的文件，也就是/var/log/messages里面，cat /var/log/messages看看有没有。

### （三）安装和配置MySQL数据库
```
yum install mysql mysql-libs mysql-server
```
安装这三个包就可以了。安装完之后用service mysqld start启动mysql，然后用mysqladmin –uroot –ppassword设置一下root用户口令，然后就可以用mysql –uroot –ppassword测试登录了。可以登录的话，退出来，执行下面的语句：
```
mysql –uroot –ppassword</usr/share/doc/rsyslog-mysql-5.8.10/createDB.sql
```
其中password是前面一步设置的root用户口令，而<后面的这个文件是安装rsyslog-mysql软件包的时候安装的，版本号有可能不同，可以用rpm –ql rsyslog-mysql查看一下安装的文件在哪里。

这一步完成之后，保存日志的数据库就建好了，可以用root用户登录看一看；

show databases;显示有Syslog就对了。然后就是授权：
```
GRANT all on Syslog.* to syslog identified by ‘syslog’;
flush privileges;
```
MySQL可以一条指令同时完成授权和创建用户，只是授权后要刷一下。

数据库准备好以后，就可以修改/etc/rsyslog.conf，在MODULES部分增加一行：

$Modload ommysql加载ommysql模块，在最后增加一行：
```
*.* :ommysql:localhost,Syslog,syslog,syslog
```
*.*表示所有日志，:ommysql表示使用输出到mysql的模块，后面四个“，”分隔的字段分别是：数据库主机名、数据库名、数据库用户名、密码。

这样就可以将收集到的日志保存到数据库里了，当然，文件系统里也还有一份，可以把配置文件前面对应的行注释掉，就不会占空间了。

查看一下数据库里面有没有日志。如果出现中文乱码，多半是字符集的原因，修改/etc/my.cnf文件，在[mysqld]节加character_set_server=utf8，在[client]节加default-character-set=utf8。最后用Chkconfig把mysqld设置为自动启动。

### （四）安装ApacheHTTP、PHP和LogAnaylizer
```
yum install httpd php php-mysql php-gd
```
装完之后service httpd start启动http服务，vi /var/www/html/index.php

输入以下内容：
```
<?php

phpinfo();

?>
```
打开浏览器，在地址栏输入locolhost或本机IP，如果能显示PHP信息页面，则证明ApacheHTTP、PHP安装成功。这里应该顺便检查一下能不能远程访问。

然后安装LogAnaylizer
```
tar –zxvf loganalyzer-4.1.6.tar.gz –C /var/www/html/
cd /var/www/html/loganalyzer-4.1.6
mv src/ ../loganalyzer
cp contrib./*.sh ../loganalyzer
cd ../loganalyzer
touch config.php
chmod 644 config.php
```
最后两行实际上在contrib目录里有两个脚本文件可以自动完成。但是因为很简单，执行脚本还要拷贝到loganalyzer目录，修改脚本执行权限才行，直接敲命令还快些。不过，也许版本不同脚本命令不一样，可以看看脚本的内容再决定怎么做。实际上loganalyzer运行时，也就是index.php运行时需要读取config.php内容，如果发现config.php是空的，就会先执行一次配置过程。

打开浏览器，在地址栏输入：http://服务器IP/loganalyzer，开始配置LogAnalyzer，配置很简单，其中在Step 3的时候，Enable User Database建议选择Yes，配置一个用户数据库，使它具有登录功能，这样可以以后修改一些配置，特别是中文乱码问题，登录后进Admin Center修改页面编码非常容易，修改config.php或index.php就难喽。配置完成之后，再次打开该网址，就会显示数据库里的日志信息了。最后，仍然用CHKCONFIG命令把httpd服务设成自启动，方便一些。

整个架设过程就这一些，简单了一点，但是要点都点到了。至于Rsyslog的模板、过滤器、规则集，无非就是规范日志格式，刷选日志，分类保存等等，其实如果用数据库保存日志几乎就没什么必要分类了，刷选的话倒是可以丢掉一些不重要的日志以节约空间，规范格式倒是有实际意义，方便查看、分析，但是这个最麻烦，特别是收集有多种不同的但都不规范的日志的时候更是，这个以后再慢慢学习了。