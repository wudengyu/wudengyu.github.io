---
layout: post
title:  "Linux下Oracle 11g数据库自动启动和关闭"
date:    2018-01-02 10:50:04 +0800
categories: jekyll update
---
Oracle 11g数据库软件安装好之后，在$ORACLE_HOME/bin目录下有dbstart、dbshut两个脚本，这两个脚本就是用来启动和关闭数据库的，用DBCA建库的时候，会在/etc/oratab文件中添加实例名、对应的目录、以及是否支持脚本启动（默认否）的记录，前面两个脚本文件运行时需要带ORACLE_HOME作为参数，运行时就是通过查找/etc/oratab文件中的支持脚本启动的项根据目录找到SID然后启动或关闭的。所以，首先修改/etc/oratab文件，将准备自动启动、关闭的实例的行的最后的N改为Y,然后就可以用dbstart /home/oracle/app/oracle/product/11.2.0/db_home1/这样的形式测试dbstart、dbshut这两个脚本是否生效，如果可以，就可以开始写自动启动的脚本：

```bash
#!/bin/sh
# chkconfig: 35 80 10
# description: Oracle auto start-stop script.
 
ORACLE_HOME=/home/oracle/app/oracle/product/11.2.0/db_home1
ORACLE_OWNER=oracle
if [ ! -f $ORACLE_HOME/bin/dbstart ]
then
    echo "Oracle startup: cannot start"
    exit
fi
case "$1" in
'start')
# Start the Oracle databases:
echo "Starting Oracle Databases ... "
echo "-------------------------------------------------" >> /var/log/oracle
date +" %T %a %D : Starting Oracle Databases as part of system up." >> /var/log/oracle
echo "-------------------------------------------------" >> /var/log/oracle
su - $ORACLE_OWNER -c "$ORACLE_HOME/bin/dbstart" >>/var/log/oracle
echo "Done"
 
# Start the Listener:
echo "Starting Oracle Listeners ... "
echo "-------------------------------------------------" >> /var/log/oracle
date +" %T %a %D : Starting Oracle Listeners as part of system up." >> /var/log/oracle
echo "-------------------------------------------------" >> /var/log/oracle
su - $ORACLE_OWNER -c "$ORACLE_HOME/bin/lsnrctl start" >>/var/log/oracle
echo "Done."
echo "-------------------------------------------------" >> /var/log/oracle
date +" %T %a %D : Finished." >> /var/log/oracle
echo "-------------------------------------------------" >> /var/log/oracle
touch /var/lock/subsys/oracle
;;
 
'stop')
# Stop the Oracle Listener:
echo "Stoping Oracle Listeners ... "
echo "-------------------------------------------------" >> /var/log/oracle
date +" %T %a %D : Stoping Oracle Listener as part of system down." >> /var/log/oracle
echo "-------------------------------------------------" >> /var/log/oracle
su - $ORACLE_OWNER -c "$ORACLE_HOME/bin/lsnrctl stop" >>/var/log/oracle
echo "Done."
rm -f /var/lock/subsys/oracle
 
# Stop the Oracle Database:
echo "Stoping Oracle Databases ... "
echo "-------------------------------------------------" >> /var/log/oracle
date +" %T %a %D : Stoping Oracle Databases as part of system down." >> /var/log/oracle
echo "-------------------------------------------------" >> /var/log/oracle
su - $ORACLE_OWNER -c "$ORACLE_HOME/bin/dbshut" >>/var/log/oracle
echo "Done."
echo ""
echo "-------------------------------------------------" >> /var/log/oracle
date +" %T %a %D : Finished." >> /var/log/oracle
echo "-------------------------------------------------" >> /var/log/oracle
;;
 
'restart')
$0 stop
$0 start
;;
esac
```
这个文件保存为/etc/init.d/oracle，然后chmod a+x，赋予所有人可执行权限，再用chkconfig --add oracle就可以将刚刚这个脚本加入到自启动程序的列表里，至于在哪种模式下启动，哪种模式下关闭是由第二行3 5这两个数字指定的。否则，就需要用chkconfig --level ? 【ON|OFF】调整。另外需要说明的是，开机自启动时候，用户配置文件里面的ORACLE_HOME这个变量是还没有的，并且操作系统也不可能知道要用哪个用户身份运行，所以，这个脚本里面ORACLE_OWNER、ORACLE_HOME要根据实际情况修改才行。

补充一下，用chkconfig管理脚本，实际上就会在/etc/rc?.d下面建立链接，允许启动的模式下链接名前面有个S，实际调用时就会传递start参数进去，禁止启动的模式下链接名前面加K，调用时传递stop，所以上面的脚本通过chkconfig -add配置之后，系统启动进入模式3或5的时候就会自动运行脚本启动数据库，而关闭或重启时（模式0、6）就会自动关闭数据库。上面的例子只有一个实例，如果有几个实例也差不多，启动脚本一样，只是/etc/oratab不一样而已。

另外，集群环境包括单实例的集群就不用这样干了，集群环境下本身有SVRCTL来控制。