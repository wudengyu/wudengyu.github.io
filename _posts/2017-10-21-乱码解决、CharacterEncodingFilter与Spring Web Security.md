---
layout: post
title:  "乱码解决、CharacterEncodingFilter与Spring Web Security"
date:   2017-10-21 16:39:39 +0800
categories: jekyll update
---
 编程过程中，时不时就会遇到乱码问题，也就是汉字显示不正常。最根本的原因当然是因为现在的计算机不是咱中国人发明的，那些歪果仁当初在设计字符编码的时候就只考虑了26个英文字母和一些英文符号，处理不了汉字。其实不仅仅是汉字，就连同样是拼音文字的欧洲各国语言也处理不了，反正只要不是英语的就都处理不了。然后各个国家、各种语言都把自己的语言进行编码，甚至同一个国家同一种语言都会有几种不同的编码方案，因为一开始的时候没有什么标准要求，怎么编都行。于是，各种各样的字符集产生了，其实编码多并没有太大的问题，大家各用各的，互不相关，互不干涉，只是要想交换数据就不太可能，要想同时处理几种编码就更不可能。这时就有人跳出来统一各种编码，把世界上所有语言的文字、符号统统收编，其实这也是好事，各种文字、各种符号，都有不同的编码，想用什么文字就用什么文字，只不过是不同的编码而已。但是，乱码问题恰恰是这个时候涌现出来的，以前，不管有多少种编码方案，但就一个程序来说，输入的是这种编，输出的也是这种编码，不会有什么乱码问题，而自从用了Unicode之后，要考虑转换问题，转换错了，乱码就出现了。

为什么要转换呢？没有Unicode的时候，各种编码互相转换很麻烦，也就不转了，即便要转也是专门程序、专业团队的事，一般人不去管，倒也相安无事，有了Unicode的时候，各种编码转成Unicode很容易，而且Unicode的目标就是要统一，所以，当核心软件或者说操作系统采用Unicode之后，没有采用Unicode的程序就只能依靠转换的办法了，因为到最后调用操作系统功能进行输出的时候只能用Unicode。不是所有的程序都采用了Unicode编码的，这其中，很关键的是输入法，事实上，一个字符或者一个字在计算机内如何表示，最根本是取决于输入法的，当然，广义来说，还要包括语音输入、文字识别之类的输入，这些是字符编码的根本，要是所有的输入都采用了Unicode编码，Unicode就能真正实现统一编码，转换应该就不需要了。

事实上，决大多数程序都不考虑编码转换的问题，比方说计算一个简单的加法，或者是求解一个方程，或者格式化一个磁盘，再或者通过网络发送一串数据，这些功能和编码没什么关系，是这些程序根本不需要去考虑的。这些程序即便要要输出字符，也只是简单的调用操作系统或其他程序的功能，接受输入也一样，不管输入的是什么编码，就是简单的当作一串二进制数处理就行了。本来我们自己写的Web程序也是可以不用管什么编码的，但这取决于接受的输入也就是来自浏览器请求是不是已经转换成Unicode编码了，显然，浏览器对于输入并没有处理编码的转换，而IIS、Tomcat这些后台程序也没有，所以，把本地编码转换成Unicode的过程就只能放在Web程序中。另一方面，浏览器可以处理各种编码的输出，但必须通过<meta charset="${encoding}">这个语句告诉它才行，同时还要保证提供的确实是指定的编码，否则，浏览器就会转换错误，出现乱码。也就是说，如果是一个静态的网页，这个html文件本身的编码就要和meta语句标明的编码一致。同样，像Thymeleaf这些经过程序处理的模板其文件本身采用的编码必需正确的告诉这些处理程序或者说设置正确，否则模板引擎在处理的时候就会转换错误出现乱码，处理模板的过程跟meta没有关系，不过，一般来说，输入的是什么编码输出也是什么编码，然后输出的到了浏览器又必须要跟meta一致，所以实际上就要三处一致。这些是静态编码，处理起来比较简单，就是要设置和实际一致。像表单输入的编码转换，一般的处理办法就是加一个filter，Spring里面就有一个现成的CharacterEncodingFilter过滤器，采用JavaConfig的情况下，加过滤器的位置通常是在 AbstractAnnotationConfigDispatcherServletInitializer子类里重载getServletFilters()，代码如下：

```
import javax.servlet.Filter;
import javax.servlet.MultipartConfigElement;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.ServletRegistration.Dynamic;
 
import org.springframework.web.filter.CharacterEncodingFilter;
import org.springframework.web.servlet.support.AbstractAnnotationConfigDispatcherServletInitializer;
 
public class WebAppInitializer extends AbstractAnnotationConfigDispatcherServletInitializer {
 
	@Override
	protected Class<?>[] getRootConfigClasses() {
		return new Class[] {RootConfig.class};
	}
 
	@Override
	protected Class<?>[] getServletConfigClasses() {
		return new Class[] {WebConfig.class};
	}
 
	@Override
	protected String[] getServletMappings() {
		return new String[] {"/"};
	}
	
    @Override
	protected Filter[] getServletFilters() {
    	CharacterEncodingFilter characterEncodingFilter=new CharacterEncodingFilter();
    	characterEncodingFilter.setEncoding("utf-8");
    	characterEncodingFilter.setForceEncoding(true);
    	return new Filter[] {characterEncodingFilter};
	}
}
```
这段代码在没有加入Spring Security的时候是可以解决乱码问题的，加入Spring Security后就失效了，原因是Spring Security把它的Filter Chain自动放到了最前面，然后其中的CsrfFilter过滤器在验证了csrf token之后，重写了请求中的数据（应该删除了csrf这一部分），改变了编码，造成了CharacterEncodingFilter转换失败。虽然想不通为什么删除或者重写会改变编码，但总之问题就是这个，所以解决的办法就是把CharacterEncodingFilter放在CsrfFilter之前，代码可以这样：
```
@Configuration
@EnableWebMvcSecurity
public class SecurityConfig extends WebSecurityConfigurerAdapter {
    @Override
    protected void configure(HttpSecurity http) throws Exception {
        CharacterEncodingFilter filter = new CharacterEncodingFilter();
        filter.setEncoding("UTF-8");
        filter.setForceEncoding(true);
        http.addFilterBefore(filter,CsrfFilter.class);
        ......
        ......
 
       }
}
```
或者，仍然是在AbstractAnnotationConfigDispatcherServletInitializer子类里重载，不过，重载OnStartup。

```
import javax.servlet.Filter;
import javax.servlet.MultipartConfigElement;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.ServletRegistration.Dynamic;
 
import org.springframework.web.filter.CharacterEncodingFilter;
import org.springframework.web.servlet.support.AbstractAnnotationConfigDispatcherServletInitializer;
 
public class WebAppInitializer extends AbstractAnnotationConfigDispatcherServletInitializer {
        ......
        ......
	@Override
	public void onStartup(ServletContext servletContext) throws ServletException {
		servletContext.addFilter("characterEncodingFilter", new CharacterEncodingFilter("utf-8",true))
					.addMappingForUrlPatterns(null,false,"/*");
		super.onStartup(servletContext);
	}
        ......
        ......
}
```
这里要注意addMappingForUrlPatterns函数里面第二个参数，API里面这个参数名称是isMatchAfter，所以为true也达不到放在Spring Security的过滤链之前的目的。