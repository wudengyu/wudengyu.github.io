---
layout: post
title:  "理解Spring Web Security实现Ajax登录"
date:   2017-10-05 20:30:18 +0800
categories: jekyll update
---
前面写了一篇《网页登录以及单点登录的一些概念》，提到了token和登录用户的相关信息，说token是保存在cookies里，而登录用户的信息是保存在Session里，应该说，如果登录、身份验证、权限验证等这些功能都自己实现的话，大多数人都是这么存放的，原因应该是Servlet提供的接口也就是这些。在说Spring Web Security之前，先看看假如按前面那种思路，继续把登录、身份验证等功能继续完善下去，会是什么样。

一般来说，肯定不是为了登录而登录，设计登录肯定是为了后面的身份验证即鉴权，也就是哪些资源允许哪些用户访问或者说允许具有哪些权限或角色的用户可以访问，这是最基本的了。按照前面的说法，登录实现后，会话里面已经有了用户的信息了，那么如何鉴权呢？在Web程序中是不是相应的页面或者说Controller中先检查一下用户的信息呢？如果页面不多，还不是很麻烦，但问题在于这种做法重复的代码太多，要是检验的逻辑发生变化，需要在很多地方修改相同的代码，不灵活，容易出错。所以，检查用户权限这种公共的、和页面本身功能不太相关的逻辑应该单独、集中处理，而最适合处理这些逻辑的地方莫过于Filter(过滤器)了，按照定义，Filter是与Servlet对应的，但是很明显，可以在应用程序的根目录或者需要保护的资源的根目录上使用一个Filter，集中在一个过滤器中根据权限实现过滤功能。只要权限设计确定了，硬编码也可以，不过，再灵活一点的话，这个权限过滤器应该是可配置的，这样的话，写一次代码，以后就可以重用。Spring Security就已经写好了这样一个过滤器，不用我们自己写了。只不过，使用别人现成的东西，就得按别人已经定好的规则来，否则用不了。当然，Spring不可能故意做什么限制让其他人没法用，恰恰相反，Spring考虑了各种使用环境，尽可能让各种使用环境的人都能用上Spring所以才有些复杂，对Spring的设计理解越深，越发现它的合理性和实用性。

首先是Filter Chain，前面已经说了，使用身份验证功能，应该使用一个过滤器，而Spring为了适应性和灵活性，过滤器不是一个，而是有一系列的过滤器形成一个过滤器链，这些过滤器统一由DelegatingFilterProxy这样一个代理来调用，实际使用的过滤器根据配置用到的功能不同而不同，也就是说，向容器注册的过滤器只有DelegatingFilterProxy这一个，其他的不用操心，Spring会根据具体的配置自行确定，当然，也可以自己实现过滤器委托给DelegatingFilterProxy管理。还可以自己实现过滤器直接向容器注册，Spring建议这样的话，最好是把DelegatingFilterProxy放在前面。只是从安全的角度来看，没有必要这样做，Spring的功能已经足够用了，即便不够也可以在它的框架内扩展，不至于需要直接向容器注册。根据Spring Security文档，过滤器有下面这一些：
1. ChannelProcessingFilter, because it might need to redirect to a different protocol
2. SecurityContextPersistenceFilter, so a SecurityContext can be set up in the SecurityContextHolder at the beginning of a web request, and any changes to theSecurityContext can be copied to theHttpSession when the web request ends (ready for use with the next web request) 
3. ConcurrentSessionFilter, because it uses theSecurityContextHolder functionality and needs to update theSessionRegistry to reflect ongoing requests from the principal
4. Authentication processing mechanisms - UsernamePasswordAuthenticationFilter,CasAuthenticationFilter,BasicAuthenticationFilter etc - so that theSecurityContextHolder can be modified to contain a validAuthentication request token
5. The SecurityContextHolderAwareRequestFilter, if you are using it to install a Spring Security awareHttpServletRequestWrapper into your servlet container
6. The JaasApiIntegrationFilter, if a JaasAuthenticationToken is in the SecurityContextHolder this will process theFilterChain as theSubject in theJaasAuthenticationToken
7. RememberMeAuthenticationFilter, so that if no earlier authentication processing mechanism updated theSecurityContextHolder, and the request presents a cookie that enables remember-me services to take place, a suitable rememberedAuthentication object will be put there
8. AnonymousAuthenticationFilter, so that if no earlier authentication processing mechanism updated theSecurityContextHolder, an anonymousAuthentication object will be put there
9. ExceptionTranslationFilter, to catch any Spring Security exceptions so that either an HTTP error response can be returned or an appropriateAuthenticationEntryPoint can be launched
10. FilterSecurityInterceptor, to protect web URIs and raise exceptions when access is denied

很多过滤器，实际上Spring是把所有Web安全相关的功能都通过这一系列过滤器来实现了，其中第4个是认证处理实体，包括常用的用户名密码认证、单点登录认证、基本认证等等，最后一个才是拦截器，根据配置的规则进行拦截，到了这一步如果还是被拦截的话，通常就是返回一个禁止访问的错误页面。而前面的步骤基本上就是处理用户身份问题的，比如在《网页登录以及单点登录的一些概念》中提到的用户已经有一个token的话，它就会取得用户的相关信息并且放入SecurityContextHolder，呃，在这一系列过程中，用户信息都是放在SecurityContextHolder中的，这个对象每次请求都会产生一个（在第2个过滤器也就是SecurityContextPersistenceFilter实现），后面会同步到Session中，在Spring Web环境下，随时可以用SecurityContextHolder.getContext()取得当前的SecurityContext对象也就是用户的信息。前面说最后一个是拦截器，实际上第1个也是，如果配置了安全通道(比如Https）在第1个过滤器处就会重定向，所以这一系列过滤器不能一概而论，各有各的功能，次序却是固定的，并且比较合理。里面很多是可以定制的，只不过先得了解它的流程以及调用的接口，下面通过实现Ajax登录支持的过程，介绍一下如何定制。

很明显，要实现一套安全规则，用户身份验证是必不可少的，像用户名、密码这些请求参数的名称都不确定的话，没办法验证的。所以，一般来说，实现了身份验证的程序都会提供一个默认的登录页，直接指定了用户名、密码这些请求参数的变量名称，比如tomcat提供的基本容器的身份认证就是这样。Spring的身份认证是通过UsernamePasswordAuthenticationFilter这个过滤器实现，而这个类可以指定使用的用户名、密码的参数名称，比较灵活。但是从这个类可用的方法来看，除此之外就是attemptAuthentication这个方法了，如果直接用这个过滤器去实现Ajax登录，无论成功还是失败，返回的是两个页面，XMLHTTPRequest对象不好处理返回的结果。一开始也想重写类似的过滤器，但是太复杂了，特别是具体的token生成、保存这些问题，理论上是那么一回事，但是在这里实现的话就得与Spring的实现一致，否则这一系列的过滤器就都用不上，那就等于是重新实现一套认证方案，没利用Spring了。所以只好继续研究文档和API，发现attemptAuthentication这个方法继承自AbstractAuthenticationProcessingFilter，而在AbstractAuthenticationProcessingFilter的API文档里就提到认证成功的话就会调用AuthenticationSuccessHandler失败就会调用AuthenticationFailureHandler这个接口，既然如此，编写实现这两个接口的类，在其中根据是否Ajax请求作出不同的响应就行了。实现如下：
1. AjaxAuthenticationSuccessHandler.java

```
import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
 
import org.springframework.security.core.Authentication;
import org.springframework.security.web.authentication.AuthenticationSuccessHandler;
import org.springframework.security.web.savedrequest.HttpSessionRequestCache;
import org.springframework.security.web.savedrequest.RequestCache;
import org.springframework.security.web.savedrequest.SavedRequest;
 
public class AjaxAuthenticationSuccessHandler implements AuthenticationSuccessHandler {
	
	@Override
	public void onAuthenticationSuccess(HttpServletRequest request, HttpServletResponse response,
			Authentication authentication) throws IOException, ServletException {
		if ("XMLHttpRequest".equals(request.getHeader("X-Requested-With"))) {
			String returnStr = "{\"message\":\"Success\"}";
			response.setStatus(200);
			PrintWriter writer = response.getWriter();
			writer.write(returnStr);
			writer.flush();
			writer.close();
		}else {
			RequestCache requestCache = new HttpSessionRequestCache();
			SavedRequest savedRequest = requestCache.getRequest(request,response);
			if(savedRequest!=null) {
				response.sendRedirect(savedRequest.getRedirectUrl());
			}else
				response.sendRedirect("");
		}
	}
}
```
其中SavedRequest就是Spring用来保存跳转到登录页前请求的地址的对象，《网页登录以及单点登录的一些概念》提到过服务器应该有这个功能的，而Spring是在ExceptionTranslationFilter这个过滤器里面把request保存在SavedRequest里面的。用法就像代码里面一样，先取得HttpSessionRequestCache，然后再利用getRequest方法，找到跟当前request匹配的存储的request对像，再取得其中的redirecturl，因为如果不是访问受限而跳转到登录页而是用户直接访问登录页的话，这个存储的request是没有的，所以要判断是否为空。这里实现简单粗暴了一些，实际Spring本身默认的实现是会根据配置里面是否设置了默认的跳转页面，是否总是跳转到默认的页面等再决定如何跳转的，考虑得很全面。
2. AjaxAuthenticationFailureHandler.java

代码差不多的，略。
3. WebSecurityConfig片段

```
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.annotation.authentication.builders.AuthenticationManagerBuilder;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.annotation.web.configuration.WebSecurityConfigurerAdapter;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.web.util.matcher.AntPathRequestMatcher;
 
import web.security.AjaxAuthenticationFailureHandler;
import web.security.AjaxAuthenticationSuccessHandler;
 
@Configuration
@EnableWebSecurity
public class WebSecurityConfig extends WebSecurityConfigurerAdapter{
	@Override
	protected void configure(HttpSecurity http) throws Exception {
		http
			.formLogin()
				.loginPage("/login").permitAll()
				.successHandler(ajaxAuthenticationSuccessHandler())
				.failureHandler(ajaxAuthenticationFailureHandler())
				.and()
			.logout()
				.logoutRequestMatcher(new AntPathRequestMatcher("/logout"))
 
	}
	@Bean
	public AjaxAuthenticationSuccessHandler ajaxAuthenticationSuccessHandler() {
		AjaxAuthenticationSuccessHandler ajaxAuthenticationSuccessHandler=new AjaxAuthenticationSuccessHandler();
		return ajaxAuthenticationSuccessHandler;
	}
	@Bean
	public AjaxAuthenticationFailureHandler ajaxAuthenticationFailureHandler() {
		AjaxAuthenticationFailureHandler ajaxAuthenticationFailureHandler=new AjaxAuthenticationFailureHandler();
		return ajaxAuthenticationFailureHandler;
	}
}
```
UserDetailsService这个就不说了，除非就照Spring的规定用同样的表结构存储用户信息、权限信息，否则，这个接口必须自己实现，不过这个比较容易（资料比较容易找到，网上介绍也清楚）另外，如果登录需要其他信息，比如所在部门，Spring建议是拼接在username或passwprd里面提交，反正UserDetailsService总是要定制的嘛，然后Spring也是根据这个来判断用户是否是合法用户、并取得用户信息的，这样的确是可行的办法，关键是不用做大的改动。但是如果更复杂些，比如，加图片验证功能，好像还是得自已实现UsernamePasswordAuthenticationFilter，这个以后再说了。暂时先研究到这里。

