	MilyFX 0.0.1 
		by echo <echo.xjtu@gmail.com>

1. Introduction
---------------
    暑假的时候在家里无聊，翻出了MilyQQ的C语言代码，这个本来是打算用来减小
MilyQQ体积的，后来用动态链接将C++的MilyQQ代码体积缩小到44K，然后这个就没啥
用了。当时刚好在看DDD的libfetion库，于是就拼凑了这么个东西，取名MilyFX. 家
里无网，拼凑的千余行代码也不知对错，打算回校测试，结果回校发现教育网不能登
陆飞信，于是又搁下了，直到前几天不知道移动还是教育网良心发现，打通了学生区
宿舍到飞信服务器的连接，欣喜之余赶忙测试，于是就有了 0.0.1 版本，这还只是
第一个能用的版本，肯定有很多的缺陷和不足，希望大家多多指正，建议请发邮箱或
者在blog留言。
    如果自己的机器如果配置足够好，推荐使用官方飞信或者DDD的LibFx，如果机器
没有X，比如远程SSH到UNIX主机上使用，或者在脚本中调用，MilyFX或许是一个不错
的选择。

2. Features overview
----------------------
    MilyFX提供两种运行模式：交互模式和非交互模式。在非交互模式中，MilyFX
根据命令行选项登陆并给好友或者自己发送一条短信然后退出，比较适合从脚本中
调用；交互模式登陆进入一个类似gdb的交互界面可以与好友聊天，以下为交互模式
中用到的命令，(交互模式下可以使用 help <命令名> 察看命令用法)
(1)ls: 察看在线好友
(2)ls a: 察看所有好友
(3)ls g: 察看分组
(4)ls q: 察看群
(5)ls b: 察看黑名单
(6)state: 察看/改变自身状态
(7)chcp: 切换输出GBK/UTF8编码
(8)impresa: 察看/修改心情短语
(9)info: 察看好友详细信息
(10)cd: 与好友聊天
(11)send: 给好友发送消息
(12)sms: 给好友发送短信
(13)!: 执行外部命令
(13)about: 察看版本信息

3. Build & Run
----------------------
需要libcurl, gcc >= 4.1.3 (主要是libfetion需要...)
进入目录执行 make , 然后 ./MilyFX 就会看到命令行参数选项
make static 可以静态链接libcurl库，详情请参考Makefile

echo@ubuntu-echo:~/MilyFX$ ./MilyFX
Usage:
MilyFX -u phone -p[password] [-i] [-l] [-g] [-w who] [-t "SMS text"]
       -u phone/fetion  login with phone number or fetion number
       -p[password]     no space before [password]
       -i               login invisible
       -l               keep a chatlog
       -g               activate GBK charset
       -w who           the Fetion number of the one you want to sent a SMS
       -t "SMS text"    the SMS content
-u 选项后面可以跟手机号或者飞信号
-p 选项后面和密码之间没有空格，建议只用-p，然后在控制台上输入密码
-i 隐身登陆
-l 聊天纪录(todo)
-g 指定输出编码为GBK，Linux下默认为UTF8
-w 选项后面跟要发送短信的飞信号(不能是手机号)，如果不指定 -w 选项则默认给自己发送短信
-t 后面跟要发送的短信内容，如果内容中有空格，请在短信内容上加引号
e.g.
登陆进入交互模式：
./MilyFX -u 你的手机号或者飞信号 -p你的飞信密码

登陆给自己发送一条短信然后退出：
./MilyFX -u 你的手机号或者飞信号 -p你的飞信密码 -t "短信内容"

登陆给好友发送一条短信然后退出
./MilyFX -u 你的手机号或者飞信号 -p你的飞信密码 -w 好友的飞信号 -t "短信内容"

4. Command reference
----------------------
请使用 help 命令察看在线帮助，或者阅读源代码

5. Todo
(1)聊天纪录
(2)群
(3)在更多的Linux发行版测试
(4)Windows版本，需要libfetion的Windows静态库

6. Support & Bug report
(1)echo.xjtu@gmail.com
(2)http://echoxjtu.cublog.cn
(3)Libfetion: http://www.libfetion.cn

8. Thanks
   邓东东 dengdd <dedodong@163.com> http://www.libfetion.cn
   感谢他提供了这么好的库，MilyFX开发过程中他也给了我很大帮助，同时也受到我不少信件骚扰.

				echo <echo.xjtu@gmail.com>  2008-10-28



