	MilyFX 0.0.1 
		by echo <echo.xjtu@gmail.com>

1. Introduction
---------------
    ��ٵ�ʱ���ڼ������ģ�������MilyQQ��C���Դ��룬��������Ǵ���������С
MilyQQ����ģ������ö�̬���ӽ�C++��MilyQQ���������С��44K��Ȼ�������ûɶ
���ˡ���ʱ�պ��ڿ�DDD��libfetion�⣬���Ǿ�ƴ������ô��������ȡ��MilyFX. ��
��������ƴ�յ�ǧ���д���Ҳ��֪�Դ������У���ԣ������У���ֽ��������ܵ�
½���ţ������ָ����ˣ�ֱ��ǰ���첻֪���ƶ����ǽ��������ķ��֣���ͨ��ѧ����
���ᵽ���ŷ����������ӣ���ϲ֮���æ���ԣ����Ǿ����� 0.0.1 �汾���⻹ֻ��
��һ�����õİ汾���϶��кܶ��ȱ�ݺͲ��㣬ϣ����Ҷ��ָ���������뷢�����
����blog���ԡ�
    ����Լ��Ļ�����������㹻�ã��Ƽ�ʹ�ùٷ����Ż���DDD��LibFx���������
û��X������Զ��SSH��UNIX������ʹ�ã������ڽű��е��ã�MilyFX������һ������
��ѡ��

2. Features overview
----------------------
    MilyFX�ṩ��������ģʽ������ģʽ�ͷǽ���ģʽ���ڷǽ���ģʽ�У�MilyFX
����������ѡ���½�������ѻ����Լ�����һ������Ȼ���˳����Ƚ��ʺϴӽű���
���ã�����ģʽ��½����һ������gdb�Ľ������������������죬����Ϊ����ģʽ
���õ������(����ģʽ�¿���ʹ�� help <������> �쿴�����÷�)
(1)ls: �쿴���ߺ���
(2)ls a: �쿴���к���
(3)ls g: �쿴����
(4)ls q: �쿴Ⱥ
(5)ls b: �쿴������
(6)state: �쿴/�ı�����״̬
(7)chcp: �л����GBK/UTF8����
(8)impresa: �쿴/�޸��������
(9)info: �쿴������ϸ��Ϣ
(10)cd: ���������
(11)send: �����ѷ�����Ϣ
(12)sms: �����ѷ��Ͷ���
(13)!: ִ���ⲿ����
(13)about: �쿴�汾��Ϣ

3. Build & Run
----------------------
��Ҫlibcurl, gcc >= 4.1.3 (��Ҫ��libfetion��Ҫ...)
����Ŀ¼ִ�� make , Ȼ�� ./MilyFX �ͻῴ�������в���ѡ��
make static ���Ծ�̬����libcurl�⣬������ο�Makefile

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
-u ѡ�������Ը��ֻ��Ż��߷��ź�
-p ѡ����������֮��û�пո񣬽���ֻ��-p��Ȼ���ڿ���̨����������
-i �����½
-l �����¼(todo)
-g ָ���������ΪGBK��Linux��Ĭ��ΪUTF8
-w ѡ������Ҫ���Ͷ��ŵķ��ź�(�������ֻ���)�������ָ�� -w ѡ����Ĭ�ϸ��Լ����Ͷ���
-t �����Ҫ���͵Ķ������ݣ�����������пո����ڶ��������ϼ�����
e.g.
��½���뽻��ģʽ��
./MilyFX -u ����ֻ��Ż��߷��ź� -p��ķ�������

��½���Լ�����һ������Ȼ���˳���
./MilyFX -u ����ֻ��Ż��߷��ź� -p��ķ������� -t "��������"

��½�����ѷ���һ������Ȼ���˳�
./MilyFX -u ����ֻ��Ż��߷��ź� -p��ķ������� -w ���ѵķ��ź� -t "��������"

4. Command reference
----------------------
��ʹ�� help ����쿴���߰����������Ķ�Դ����

5. Todo
(1)�����¼
(2)Ⱥ
(3)�ڸ����Linux���а����
(4)Windows�汾����Ҫlibfetion��Windows��̬��

6. Support & Bug report
(1)echo.xjtu@gmail.com
(2)http://echoxjtu.cublog.cn
(3)Libfetion: http://www.libfetion.cn

8. Thanks
   �˶��� dengdd <dedodong@163.com> http://www.libfetion.cn
   ��л���ṩ����ô�õĿ⣬MilyFX������������Ҳ�����Һܴ������ͬʱҲ�ܵ��Ҳ����ż�ɧ��.

				echo <echo.xjtu@gmail.com>  2008-10-28



