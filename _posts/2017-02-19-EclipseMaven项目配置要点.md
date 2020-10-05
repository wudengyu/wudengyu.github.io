---
layout: post
title:  "Eclipse Maven项目配置要点"
date:   2017-02-19 23:41:44 +0800
categories: jekyll update
---
maven解决项目依懒的方式很不错的，反正开发所需的JAR文件都需要下载，与其一个一个去不同的网站找，不如利用maven自动下载，下载回来的东西还统一放在本地仓库里，所有项目可以共用，好处还是很多的。至于麻烦就是要学习maven的使用，不过，使用集成开发环境的话，应该不会太麻烦吧。所以，打算今后都使用maven项目了。但是试着创建了一个maven项目，还是遇到了一些麻烦。

一个是速度慢，即便是创建一个空的web项目，maven也要下载很多文件，看上去就是下载一些和项目类型相关的POM文件和一些相关插件，都不大，但是几十K的文件进度条都要滚好久。如果中途中止，重新打开eclipse还不一定能继续，网上说是因为本地仓库里存在一些后缀为.lastupdate的文件影响了，仅仅是这样还好办，把这些文件删掉甚至删掉仓库中相关的文件夹就行了，但是这和速度慢无关，并不能解决慢的问题。当然，如果第一个项目创建好，再创建别的项目应该要少下很多东西，应该会好些。但是这还仅仅是创建空项目，还没有真正下载重要的jar包，几B/s的速度实在难以忍受。想想也是，默认的apache maven中央仓库在国外，访问它受墙的影响，二是如果所有人都用这个仓库，负载肯定非常非常大。所以，有条件的话，最好自己用Nexus建一个代理仓库，还可以把自己的库、第三方库都放上去。没条件的话，就另外找一个中央仓库，最好是国内的。搜索了一下，好像都反应阿里云的仓库不错，就把阿里云设成中央库的镜像。在用户目录下的.m2中，加一个settings.xml，内容如下：
```
<?xml version="1.0" encoding="UTF-8"?>
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 http://maven.apache.org/xsd/settings-1.0.0.xsd">
    <mirrors>
	<mirror>
	  <id>alimaven</id>
	  <name>aliyun maven</name>
	  <url>http://maven.aliyun.com/nexus/content/groups/public/</url>
	  <mirrorOf>central</mirrorOf>
	</mirror>
    </mirrors>
</settings>
```
这里需要注意，eclipse有时会出一个不能读取settings的错误，原因是在这个xml文件里不能有中文字符，空格都不行。所以，如果出现这个错误，要认真检查一下是否含有全角的字符，包括全角的空格。读取settings文件错误，显然配置是不会生效的。不出错误就行了，就为central仓库加了一个镜像，maven所有要从central下载的东西都会到镜像库去下载，速度快了很多。但是，阿里云的这个公共库也会出问题，所以再准备一个备用的库。用浏览器直接打开maven.aliyun.com可以看到，其实阿里云上不只这个库，就选择代理中央库这个作为备用吧，但是镜像库只能设置一个，另一个就只能用profile的方法了，这样还可以选择性激活profile，很方便。同样在设置里面加：
```
<profiles>		
	<profile>
 		<activation>
			<property>
				<name>ali</name>
			</property>
		</activation>
		<repositories>
			<repository>
				<name>alicentral</name>
				<url>http://maven.aliyun.com/nexus/content/repositories/central/</url>
			</repository>
		</repositories>
	</profile>
</profiles>
```
这样，当前面设置的镜像仓库不可用时，可以通过项目-属性-Maven-Active Maven Profiles设置一个ali的变量来激活配置。当然，还可以找更多的仓库用同样的办法设置备用。

第二个麻烦是每次update项目，项目依懒的JRE版本都会变成J2SE-1.5，现在哪里还有什么J2SE-1.5呀，官网上没有了，记得有一个环境非得用JRE1.6都找了好久。这个问题查了一下，是maven的一个特性，除非显示地在POM中指定版本，而在eclipse中的配置不管怎么改，只要update项目，eclipse就会根据POM的配置更新项目的配置，所以关键还是maven的设置。这个网上有说改POM的，有说改setting的，但是很显然，在根本不可能用J2SE-1.5的情况下，当然是改setting好喽，一劳永逸。方法也简单，同样改用户目录下的m2\setting.xml，在里面加上下面这样一段：
```
	<profiles>
		<profile>
			<id>jdk-1.8</id>
			<activation>
				<activeByDefault>true</activeByDefault>
				<jdk>1.8</jdk>
			</activation>
			<properties>
				<maven.compiler.source>1.8</maven.compiler.source>
				<maven.compiler.target>1.8</maven.compiler.target>
				<maven.compiler.compilerVersion>1.8</maven.compiler.compilerVersion>
			</properties>
		</profile>
	</profiles>
```
然后再maven->update project，就不会再有这个问题了，最多在eclipse里面java bulid path、Project Facets里面手工设置一遍。

第三个问题是如果用maven-archetype-webapp这个archetype创建maven web项目的话，在eclipse里面，Dynamic Web Module的版本就是2.3，并且改不了，这个版本之间的区别不是很清楚，好像是对应servlet版本的，如果是这样，那么版本低了有些功能就使用不了。要么在web.xml里配置不了，要么配置了校验会出错，要么配置了、校验也过了但实际上不起作用。但不管怎样，2.3版本也太低了，应该改高一些。这个版本实际上是由web.xml文件决定的，由eclipse创建的web项目，这个版本号从低改高可以，从高改低如果低于web.xml文件中的版本号也同样不行，因为改这个版本号并没有修改到web.xml文件。要修改的话就得手工修改web.xml文件，修改为3.0版就修改为
```
<?xml version="1.0" encoding="UTF-8"?>
<web-app xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns="http://java.sun.com/xml/ns/javaee"
	xsi:schemaLocation="http://java.sun.com/xml/ns/javaee http://java.sun.com/xml/ns/javaee/web-app_3_0.xsd"
	id="WebApp_ID" version="3.0">
```
要修改为3.1版本就修改为
```
<?xml version="1.0" encoding="UTF-8"?>
<web-app xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns="http://xmlns.jcp.org/xml/ns/javaee"
	xsi:schemaLocation="http://xmlns.jcp.org/xml/ns/javaee http://xmlns.jcp.org/xml/ns/javaee/web-app_3_1.xsd"
	id="WebApp_ID" version="3.1">
```
4.0版是这样的
```
<?xml version="1.0" encoding="UTF-8"?>
<web-app xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xmlns="http://xmlns.jcp.org/xml/ns/javaee"
         xsi:schemaLocation="http://xmlns.jcp.org/xml/ns/javaee http://xmlns.jcp.org/xml/ns/javaee/web-app_4_0.xsd"
         id="WebApp_ID" version="4.0">
```
这里命名空间不完全一样，所以不能通过简单地修改version来实现。仅仅改这个文件头的话，maven update的时候也会报一个不能修改Dynamic Web Module的版本的错，这就需要修改项目下.settings文件夹下的 org.eclipse.wst.common.project.facet.core.xml中的<installed facet="jst.web" version="3.1"/>这一项，否则总是有一个错误。本来这个版本号还和java版本有一定的关系， 需要同时修改<installed facet="java" version="1.8"/>这项，还有前面第二个问题中的maven依懒的java版本也需要对应的修改。不过，如果前面第二个问题已经那样解决了的话，这个问题就只需要修改这两处就够了，其他地方已经自动改好，不需要再改。

第三个问题实际上就是因为maven-archetype-webapp这个archetype（相当于是一个模板）中包含的web.xml文件版本太低所至，当然，如果maven默认提供的这个archetype版本太高，想要降低也存在同样的问题，所以，最好的办法是自己根据需要生成archetype，甚至各种不同的archetype，就灵活了，就不用每次手工修改了。