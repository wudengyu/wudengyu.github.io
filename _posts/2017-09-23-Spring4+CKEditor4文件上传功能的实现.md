---
layout: post
title:  "Spring4+CKEditor4文件上传功能的实现"
date:   2017-09-23 23:46:02 +0800
categories: jekyll update
---
CKEditor的安装配置看文档很容易搞定，这里要纠正网上一些错误的说法，就是要启动CKEditor的上传功能，要修改CKEditor代码里的一个hide标志。从官网下载到的CKEditor代码是经过压缩过的，很难看，更别说修改了。而那个hide标志，之所以默认为true，只是因为这个功能需要一些前置条件，这很容易理解，上传嘛，不仅仅是客户端的事，上传得要有服务器接收啊，同理，浏览服务器文件也一样，肯定得要服务器返回什么才行。事实上，只要像下面那样设置了filebrowserBrowseUrl、filebrowserUploadUrl这两个参数，浏览和上传功能就自动打开了，并不需要去修改源文件。而这些，文档中都有。
```json
CKEDITOR.replace( 'editor2', {
         filebrowserBrowseUrl: '/browser/browse.php?type=Files'，
         filebrowserUploadUrl: '/uploader/upload.php?type=Files'
         };
```
不过，CKEditor官方文档只有英文，不大看得懂，在文件上传这一块一直在绕着弯子推销CKFinder（收费软件），所以在实现文件浏览与上传功能的时候，费了一番功夫。文件浏览基本上没什么问题，写一个页面，把服务器上的图形文件一个个枚举出来生成<img>标签，然后写一个onclick函数把链接传回父窗口就行了，这一部分文档也还写得详细，照着做很快完成。但是在实现上传的时候陷入了一个误区，以为form是用来向服务器提交编辑内容的，还要向服务器传文件的话，是用XMLHTTP请求来提交的，但是在文档中翻了很久都没有找到这个东西，费了不少时间，还试着在Spring中直接响应文本，坑爹的CKEditor在上传文件那里展现服务器响应内容的框几乎没有高度，服务器端无论返回什么都看不到，又不知道哪里出错了，怎么改都不起作用。不得已上网搜索别人的实现，几乎都是用strust2的，看不懂，不过琢磨了很久之后发现，别人的实现当中并没有常见的JSON出现，也就是说并不像是针对XMLHTTP的响应，最后，还是在文档里面找到了一段PHP的实现，虽然还是看不懂，但怎么着也只是一个普通的网页，又将Spring服务端改成用普通视图，结果终于在上传服务器之后看到了一点变化，原来上传对话框有一个iframe显示服务器响应的，有滚动条，这才看到显示的是一个错误页，意思是说服务器不允许该页在框架中显示，坑爹啊，别人的代码，虽然不能完全全懂，但都没有包含这层意思啊，上网查了一下，这里Spring EnableWebSecurity后的一个默认设置，要取消需要这样
```java
@Configuration
@EnableWebSecurity
public class WebSecurityConfig
    extends WebSecurityConfigurerAdapter {
    @Override
    protected void configure(HttpSecurity http) throws Exception {
        http
           ...
           
           .headers()
                .frameOptions().sameOrigin()
```
或者设置为disable()，这样设置以后，再照着文档将php的页面改写成thymeleaf模板页面。结果，实验成功，服务器传回的链接自动填到了url文本框里。这也解释了为什么CKEditor的那个iframe怎么那么小了，因为，如果正常，根本就不需要在里面显示什么。搞清楚CKEditor上传以及回显机制之后，就好办了。控制器代码如下：
```java
@PostMapping
 public String handleFormUpload(@RequestParam("upload") Part file, @RequestParam("CKEditorFuncNum") String funcNum,
   Model model)   {
  model.addAttribute("funcnum", funcNum);
  String contenttype = file.getHeader("content-type");
  String filename = "";
  String ext = "";
  if (contenttype.equals("image/pjpeg") || contenttype.equals("image/jpeg"))
   ext = ".jpg";
  if (contenttype.equals("image/png") || contenttype.equals("image/x-png"))
   ext = ".png";
  if (contenttype.equals("image/gif"))
   ext = ".gif";
  if (contenttype.equals("image/bmp"))
   ext = ".bmp";
  if (contenttype.equals("image/x-icon"))
   ext = ".ico";
  if (contenttype.equals("image/pict"))
   ext = ".pic";
  if (contenttype.equals("image/tiff"))
   ext = ".tif";
  if (!ext.isEmpty()) {
   SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");
   filename = sdf.format(new Date());
  }
  String path = context.getServletContext().getRealPath("/upload");
  try {
   InputStream inputStream = file.getInputStream();
   File save = new File(path, filename + ext);
   OutputStream outputStream = new FileOutputStream(save);
   byte[] buffer = new byte[1024];
   int length = 0;
   while ((length = inputStream.read(buffer)) > 0) {
    outputStream.write(buffer, 0, length);
   }
   inputStream.close();
   outputStream.close();
  } catch (IOException e) {
   model.addAttribute("data", "Upload Error!");
  }
  model.addAttribute("fileUrl", "\upload" + filename + ext);
  return "uploadresult";
 }
```
视图如下
```html
<!DOCTYPE html>
<html xmlns:th="http://www.thymeleaf.org">
<head>
<meta charset="UTF-8">
<title>File Upload Success</title>
</head>
<body>
 <script th:if="${data}==null" th:inline="javascript">
  window.parent.CKEDITOR.tools.callFunction([(${funcnum})], [[@{/}+${fileUrl}]]);
 </script>
 <script th:if="${data}!=null" th:inline="javascript">
  window.parent.CKEDITOR.tools.callFunction([(${funcnum})], [[@{/}+${fileUrl}]],[[${data}]]);
 </script>
</body>
</html>
```
这个视图基本没有什么，只是利用thymeleaf的脚本模板功能，把control传入Model的数据插入脚本中。CKEditor接收到这个页面，并且在iframe中显示这个页面，实际上就是执行脚本，把服务器传回的文件链接填入url框里，然后，这个链接就可以插入正在编辑的文本中。

最后，还有一个问题，servlet上传文件，都有一个大小限制，这在配置服务器上传功能时必须配置的。如果用Apache的Commons FileUpload组件，就需要注册一个CommonsMultipartResolver Bean，如果使用servlet 3.0自带的MultipartResolver，那么在DispatcherServlet中就已经包含了，不需定义Bean，但是在DispatcherServlet向容器注册时，必须要进行配置，特别是要设置临时目录才能使用。同时，上传大小限制也就被设置了。
```java
@Override
 protected void customizeRegistration(Dynamic registration) {
  registration.setMultipartConfig(new MultipartConfigElement("C:\\temp", 10485760, 10485760, 0));
 }
```
虽然这里可以把限制设置得很大，但是还是要有所限制才好，那么问题来了，如果上传的文件大小超出限制了会怎么样，抛出异常，但是这个异常很特别，是由servlet抛出的，如果直接用servlet编程当然不会有什么问题，捕获异常并进行处理就行了，但是现在是用Spring，Spring对底层的servlet进行了包装，然后把request发送给controller，现在还没包装好就引发异常了，所以这个异常不能在controller里面捕获和处理，也就是说在controller里用@ExceptionHandler(MultipartException.class)是捕获不到异常的。如果用Apache的Commons FileUpload组件可以设置一个resolveLazily参数值，就可以让异常推迟到controller里抛出，但是用Selvlet 3.0自带的MultipartResolver就没有这个参数可用，就只有前面代码中那几个值可以设。这样的话，就只有自定义异常处理器了。同样，这个异常处理器需要在WebMvcConfigurerAdapter里面注册，否则没地方放啊，即不是M又不是V，也不是C。WebMvcConfigurerAdapter这个类里面有一个extendHandlerExceptionResolvers方法，可以用来扩展异常处理，自定义的异常处理器就在这里加进去。实际上，对于这种异常，也没什么可以处理的，就是响应一个错误页面，提示出错罢了，所以，也就不必专门实现什么异常处理类，直接生成一个SimpleMappingExceptionResolver的实例，然后把这个异常与所对应的错误页关连起来就行了。代码如下：
```java
@Override
 public void extendHandlerExceptionResolvers(List<HandlerExceptionResolver> exceptionResolvers) {
  SimpleMappingExceptionResolver exceptionResolver=new SimpleMappingExceptionResolver();
  Properties mappings=new Properties();
  mappings.setProperty("MultipartException", "error");
  exceptionResolver.setExceptionMappings(mappings);
  exceptionResolver.setOrder(Ordered.LOWEST_PRECEDENCE);
  exceptionResolvers.add(exceptionResolver);
 }
```
实现这么一个看似简单的功能，涉及到这么多知识点：Spring上传组件的配置、X-Frame-Options及配置、Spring异常处理、CKEditor及javascript脚本还有服务端保存文件。这里是用时间生成服务端文件名，因为CKEditor提交服务器就只有一个Multpart，本来也可以从请求头中取客户端文件名的，一来取文件名麻烦，二是按客户端的名称来保存的话，客户端的名称可能各种各样乱七八糟的不规范，三就是容易重名，异常处理也麻烦，所以目前采用这种办法来生成服务端文件名，如有必要，再完善吧。