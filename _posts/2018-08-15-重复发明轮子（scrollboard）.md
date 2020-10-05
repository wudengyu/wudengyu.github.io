---
layout: post
title:  "重复发明轮子（scrollboard）"
date:    2018-08-15 22:13:11 +0800
categories: jekyll update
---
最近修改网站时，发现这个古老的网站图片轮播居然是用flash做的，图片地址作为参数传到flash做的控件里去，然后这个控件就负责播放，但是这个控件并没有width和height参数，也就是说，大小是固定的，这就比较麻烦了，想改一改尺寸都不行，用flash做一个轮播的动画应该不难，但是要做成控件，还要能接收参数就得好好研究flash及其脚本编程不可，flash用处似乎并不大，已经很久很久没用过了，不想再去深入学习了。现在jquery、css功能这么强大，何不用这些技术来实现呢？

首先，肯定不想重复发明轮子，网上找下来很多类似的插件以及js源码，不过大多数说明文档都不详细，照抄例子可以，但是想要自定义一下就无从下手，还是得研究源码，源码都还不短，真还不太好办。然后还是找一找功能比较全、文档也详细、活跃的开源代码吧，最典型的是slick和jcarousel，但是简单试用下来slick显示不正常，不知道是哪里设置不对，而jcarousel使用比较复杂，控制播放之类的功能都做成插件形式，这倒是符合现在的组件化的潮流，按需分发，小巧且灵活，不过这么简单一个东西，也不必要这样吗？值得怀疑。总之，感觉这两个功能都是强大的，就是要用好还是得仔细琢磨文档，都是E文，也很麻烦。麻烦并不可怕，只不过总是觉得轮播几个图片不值得这么麻烦，所以还是考虑自己发明轮子吧。(这个真的是轮子会scroll的)

然后，找思路，最直接的想法就是在页面上开一个窗口，然后把图片拼成一长条放在下面一层，图片在窗口下面移动，就跟放电影或者幻灯片一样。挺简单的。窗口overflow:hidden之后就可以遮住之外的部分，图片拼接也不是真的拼成一张大图，全部float起来就排好了，移动第一个其他的都跟着动了。这种简单直接的思路存在一个问题，块元素overflow:hidden之后，必须要指定大小，否则会跟内容一样大小，放一张大图在里面就会被撑大，达不到遮挡多余部分的效果。这么一来跟现在流行的自适应设计模式不符。如果要自适应就不能这么干，或许要换其他实现方法，或许要在写脚本的时候复杂一点，但不管怎样，先实现了这个简单的再说，复杂的放到以后吧。
主要代码如下：
1. html部分

```html
<section class="scrollboard">
        <ul class="scroll-content">
          <li class="scroll-item">
            <img src="img/20_20180710150722_Vz88H.jpg" alt="2018年7月5日，XXXX仪式现场" />
            <div class="item-title">
                2018年7月5日，XXXX仪式现场
            </div>
          </li>
          <li class="scroll-item">
            <img src="img/20_20180709170721_xA0ER.jpg" alt="2018年7月5日上午，XXXX现场" />
            <div class="item-title">
                2018年7月5日上午，XXXX现场
            </div>
          </li>
          <li class="scroll-item">
            <img src="img/20_20180710150749_PcWOO.jpg" alt="2018年7月5日，参加活动人员现场合影留念" />
            <div class="item-title">
                2018年7月5日，参加活动人员现场合影留念
            </div>
          </li>
          <li class="scroll-item">
            <img src="img/20_20180712160754_oVV8v.jpg" alt="7月12日上午，XXXX召开XXXX会" />
            <div class="item-title">
                7月12日上午，XXXX召开XXXX会
            </div>
          </li>
          <li class="scroll-item">
            <img src="img/20_20180711150757_g29ql.jpg" alt="2018年7月11日上午，XXXX召开XXXX培训会，图为会议现场" />
            <div class="item-title">
                2018年7月11日上午，XXXX召开XXXX培训会，图为会议现场
            </div>
          </li>
        </ul>
      </section>
```
使用了几个css类，用于控制样式和jQuery定位，元素名不重要，重要的是层次结构。比上面简单的描述多了一个显示说明文字的div。本来可以在js里从img的alt取值生成的，但是似乎没有这个必要。
2. css部分
```
.scrollboard{
position:relative;
overflow:hidden;
width:512px;
height:288px;
border:1px solid transparent;
}
.scroll-content{
position:relative;
padding-left:0;
list-style:none;
}
.scroll-content .scroll-item{
float:left;
position:relative;
}
.scroll-content .scroll-item .item-title{
position:absolute;
bottom:0;
width:100%;
height:30px;
line-height:30px;
text-align:center;
filter:alpha(opacity=50);
background:rgba(0, 0, 0, 0.5) none repeat scroll 0 0 !important;
color:#dfecf7;
}
.scroll-pageination{
position:absolute;
bottom:30px;
right:3px;
}
.scroll-pageination span{
padding:0 5px;
color:#fff;
background:#777;
}
.scroll-pageination .curr{
background:#cc0000;
}
```
其实css部分才是最主要部分，实现上面的思路主要靠css,完成之后页面效果就出来了，除了不会动之外，该显示的图片，文字说明都有并且都在设计的位置。另外，定义了.ageination类，是用来控制页码样式的，本来也是写在里的，不过就是123456几个数字而已，感觉每次写这样的html也是太傻，后来改成自动生成了，不过这不重要。
3. js部分                              
```
(function ($){
    var interval = 5000;
    var speed = 500;
    var timeout;
    var total;
    var current = 0;
    var target = 1;
    var $content;
    var $pagination;
    $.fn.extend({
        "scrollboard": function () {
            total=this.children(":first").children().length;
            $content = $(".scroll-content",this);
            /*修改容器宽度，以使其容纳得下所有图片；*/
            $content.width(this.width()*total);
            /*使用容器的尺寸替换每个显示项目的尺寸*/
            var width=this.css("width");
            var height=this.css("height");
            for(var i=0;i<total;i++){
                $content.find(".scroll-item img").eq(i).css("width",width);
                $content.find(".scroll-item img").eq(i).css("height",height);
            } 
            /*构造页码(pagination)*/
            console.log("total:"+total);
            $pagination=$("<div class='scroll-pageination'></div>");
            for (var i=0;i<total;i++){
                if(i==0){
                    $pagination.append($("<span class='curr'>"+(i+1)+"</span>"));
                }else{
                    $pagination.append($("<span>"+(i+1)+"</span>"));
                }
            }
            this.append($pagination);
            /*定义鼠标移到内容上的处理，两个回调函数，一个是移入时，一个是移出时。
            setInterval()、clearInterval()是DOM函数*/
            $content.hover(
                function(){
                    clearInterval(timeout);
                },
                function(){
                    timeout = setInterval(autoswitch,interval);
                }
            );
            /*鼠标移到页码上时：停止自动播放并切换到指定页*/
            $pagination.children().each(function(){
                $(this).mouseover(function(){
                    clearInterval(timeout);
                    $content.stop(true);//停止动画
                    switchto($(this).index());
                });
            });
            //timeout = setInterval(autoswitch,interval);
        }
    });
    function switchto(target) {
        console.log("switchTo:"+target);
        if(target>=total){
            target = 0;
        }   
        if(target<0) {
            target = total-1;
        }   
        $content.animate({left:"-"+$content.children(":first").width()*target+"px"},speed);
        $pagination.children().eq(current).removeClass("curr");
        $pagination.children().eq(target).addClass("curr");                
        current = target;
    }
    function autoswitch() { 
        target = current + 1;
        switchto(target);
    }
})(jQuery);
```                                                         
代码是仿扩展jQuery的方式写的，但是并没有自定义参数，自己用需要修改的话直接改代码就行了，要正式发布的话再完善。主要部分一是初始化，再就是switchto函数，自动播放的话定义一个计时器，再调用switchto播放下一张图。然后就是鼠标悬停时到图片上时停止倒计时，悬停到页码上直接切换而几个事件而已 。                                                                     
   
最后就是调用，在html这一部分代码之后，用jQuery的方式调用，即：$(“.scrollboard”).scrollboard(); 