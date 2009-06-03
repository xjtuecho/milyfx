/**
 *  Copyright (C) 2008 by Echo <echo.xjtu@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/**
 * The source code encoding is GBK
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>	/*int getch()*/
#else	/*Linux*/
#include <termios.h>
#include <unistd.h>
#include <iconv.h>
#endif

#include "libfetion.h"
#include "datastruct.h"
#include "event.h"
#include "mygetopt.h"
#include "utf8.h"

#define FX_PACKAGE	"MilyFX"
#define FX_VERSION	"0.0.2"

#ifdef _MSC_VER
#define strdup _strdup
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif
#define strequ(s1,s2)		(!strcmp(s1,s2))
#define ARRAY_SIZE(a)		(sizeof(a)/sizeof(a[0]))
#define LIBFETION_VERSION	80
#define BUF_LEN	16
#define TEXT_LEN	512

enum {
	CHARSET_UTF8,
	CHARSET_GBK
}	g_charset = CHARSET_UTF8;

enum {
	STATUS_OFFLINE,
	STATUS_LOGIN,
	STATUS_RELOGIN,
	STATUS_ONLINE,
	STATUS_DEREGISTERED
}	g_status = STATUS_OFFLINE;

typedef struct {
	long id;
	const char *name;
}	index_item_t;

index_item_t *g_current=NULL;
DList *g_index=NULL;

int  g_is_invisible=0;
int  g_is_log=0;
int  g_is_debug=0;
int  g_is_login_ok=0;
char g_user[BUF_LEN]={0};
char g_passwd[BUF_LEN]={0};
char g_who[BUF_LEN]={0};
char g_text[TEXT_LEN]={0};

PROXY_ITEM g_proxy = {"211.136.85.130", "3128", "", "", PROXY_HTTP};

const char *g_cmd_array[] = {
	"quit","exit","bye","ls","!", "cd", "state","self","block",
	"help","info","send","sms","chcp","about","log","impresa","debug"
};

void init_options(int argc,char **argv);
void usage(void);
int parse_input(char *input, char **arg1, char **arg2, char **arg3);
int is_a_cmd(const char *s);
int translate(char *buf);
void print_utf8(const char *fmt,...);
void print_gbk(const char *fmt,...);
void log_message(const char *fmt,...);
void helpinfo(const char *cmd);
int main_loop(void);
int non_interactive(void);
const char *get_state_string(int state);
const char *get_usr_type_string(int usr_type);
int util_getch(void);
int util_get_password(char* pwd, int len);
void util_debug(char* info, ...);
void index_release(DList *list);
DList* index_append(DList *list,long id,const char *name);
long index2id(long id);

int  on_out_cmd(const char *input);
void on_cmd_chcp(void);

void on_cmd_ls(const char *arg);
void on_cmd_ls_online(void);
void on_cmd_ls_all(void);
void on_cmd_ls_group(void);
void on_cmd_ls_qun(void);
void on_cmd_ls_black(void);

void on_cmd_state(const char *arg2);

void on_cmd_info(const char *arg2);

void on_cmd_cd(const char *arg2);
void on_cmd_send(const char *arg2,const char *arg3);
void on_cmd_sms(const char *arg2,const char *arg3);

void on_new_message(long id);
void on_new_qun_message(long qun_id);
void on_sys_message(long id);
void show_message(long id);
void on_set_state_ok(int state);
void on_sys_err_network(int err);
void on_sys_dialog_message(int message, Fetion_MSG* fx_msg,long who);

//void cb_cmd_state(int message, WPARAM wParam, LPARAM lParam, void* args);
void cb_system_msg(int message, WPARAM wParam, LPARAM lParam, void* args);
//void cb_fx_dialog(int message, WPARAM wParam, LPARAM lParam, void* args);
void cb_fx_login(int message, WPARAM wParam, LPARAM lParam, void* args);
void cb_fx_relogin(int message, WPARAM wParam, LPARAM lParam, void* args);
void cb_cmd_self(int message, WPARAM wParam, LPARAM lParam, void* args);

int main(int argc, char **argv)
{
	int ret = 0;
	init_options(argc,argv);

	if(!fx_init())
	{
		fprintf(stderr,"fx_init() error\n");
		exit(-1);
	}

	if(g_is_invisible)
		fx_set_login_status(FX_STATUS_OFFLINE);
	else
		fx_set_login_status(FX_STATUS_ONLINE);

	if(g_text[0])
		ret = non_interactive();
	else
		ret = main_loop();

	print_utf8("logout now...\n");
	fx_loginout();
	print_utf8("release resource now...\n");
	fx_terminate();
	return ret;
}


int non_interactive(void)
{
	if(!translate(g_text))
		fprintf(stderr,"translate() error\n");

	if(!fs_login(g_user,g_passwd))
	{
		fprintf(stderr,"fs_login() error\n");
		return -1;
	}

	if(g_who[0])
	{
		fx_send_sms(atol(g_who), g_text, NULL, NULL);
		print_utf8("send SMS to %s\nSMS content:%s\n",g_who,g_text);
	}
	else
	{
		fx_send_sms_to_self(g_text, NULL, NULL);
		print_utf8("send SMS to myself\nSMS content:%s\n",g_text);
	}
	return 0;
}


int main_loop(void)
{
	int args=0;
	char input_buf[TEXT_LEN] = {0};
	char *arg1=NULL,*arg2=NULL,*arg3=NULL;

	g_status = STATUS_LOGIN;
	fx_login(g_user, g_passwd, cb_fx_login, NULL);

	while(g_status != STATUS_DEREGISTERED && g_status != STATUS_OFFLINE)
	{
		if(g_current)
		{
			if(g_current->name)
				print_utf8(g_current->name);
			else
				print_utf8("%ld", g_current->id);
		}

		print_utf8(">");

		memset(input_buf, 0, TEXT_LEN);

		if(!fgets(input_buf, TEXT_LEN,stdin))
			fprintf(stderr, "fgets() error\n");
		if(!translate(input_buf))
			fprintf(stderr, "translate() error\n");

		strncpy(g_text, input_buf, TEXT_LEN);

		args = parse_input(g_text, &arg1, &arg2, &arg3);

		if(0 == args)
			continue;
		else if('!' == arg1[0]) {
			on_out_cmd(input_buf);
			continue;
		}
		else if(is_a_cmd(arg1)) {
			if(strequ("about",arg1))
				print_utf8(FX_PACKAGE" "FX_VERSION"  Author: Echo <echo.xjtu@gmail.com> http://echoxjtu.cublog.cn\n");
			else if(strequ("ls", arg1)) {
				if(1==args)
					on_cmd_ls("");
				else if(2==args)
					on_cmd_ls(arg2);
				else
					helpinfo("ls");
			}
			else if(strequ("info", arg1)) {
				if(2 == args)
					on_cmd_info(arg2);
				else
					helpinfo("info");
			}
			else if(strequ("state", arg1)) {
				if(1 == args) {
					print_gbk("当前状态: %s\n", 
						get_state_string( fx_get_user_state() ));
				} else if (2 == args)
					on_cmd_state(arg2);
				else
					helpinfo("state");
			}
			else if(strequ("log", arg1)) {
				if(1 == args) {
					g_is_log = ~g_is_log;
					if(g_is_log)
						print_utf8("keep a chatlog\n");
					else
						print_utf8("do not keep a chatlog\n");
				} else
					helpinfo("log");
			}
			else if(strequ("debug", arg1)) {
				if(1==args){
					g_is_debug = ~g_is_debug;
					if(g_is_debug)
						print_utf8("run in debug mode\n");
					else
						print_utf8("run in normal mode\n");
				}else continue;
			}
			else if(strequ("cd",arg1)) {
				if(2 == args)
					on_cmd_cd(arg2);
				else
					helpinfo("cd");
			}
			else if(strequ("send",arg1)){
				if(3 == args)
					on_cmd_send(arg2, arg3);
				else
					helpinfo("send");
			}
			else if(strequ("sms",arg1)){
				if(3==args)
					on_cmd_sms(arg2,arg3);
				else
					helpinfo("sms");
			}
			else if(strequ("chcp",arg1))	{
				on_cmd_chcp();
			}
			else if(strequ("self",arg1)) {
				if(args>1) {
					fx_send_sms_to_self(4+strstr(input_buf,"self"), cb_cmd_self,  NULL);
				} else
					helpinfo("self");
			}
			else if(strequ("help",arg1)) {
				(2==args)?helpinfo(arg2):helpinfo("help");
			}
			else if(strequ("impresa",arg1)) {
				if(2==args) {
					if(fx_set_user_impresa(arg2,NULL,NULL))
						print_utf8("set impresa successfully\n");
				}
				else if(1==args) {
					if(fx_data_get_PersonalInfo()) {
						print_utf8(fx_data_get_PersonalInfo()->impresa);
						print_utf8("\n");
					}
				}
				else
					helpinfo("impresa");
			}
			else if(strequ("quit", arg1) ||strequ("exit", arg1) || strequ("bye", arg1))
				break;
		}
		else if(!is_a_cmd(arg1) && g_current) {
			/*send dialog message to the guy who you are chatting with*/
			fx_dialog_send(g_current->id, input_buf, NULL, NULL);
		}
		else
			continue;
	}
	return 0;
}

void helpinfo(const char *cmd)
{
	int i;
	if(strequ(cmd,"ls")||strequ(cmd,"list")|| strequ(cmd,"l")) {
		print_gbk("ls a/q/g/b: 无参数列出当前在线好友列表,有参数列出好友详细信息.\n");
	} else if(strequ(cmd,"quit")||strequ(cmd,"exit")) {
		print_gbk("quit/exit/bye: 退出本程序\n");
	} else if(strequ(cmd,"block")) {
		print_gbk("block: 是否打开消息屏蔽功能\n");
	} else if(strequ(cmd,"state")) {
		print_gbk("state [1|2|3|4]: 1-在线 2-隐身 3-忙碌 4-离开,无参数察看当前状态\n");
	} else if(strequ(cmd,"send")|| strequ(cmd,"s")) {
		print_gbk("send <编号|群号> <消息>: 给好友或者群发送一个消息\n");
	} else if(strequ(cmd,"sms")) {
		print_gbk("sms [编号|飞信号] <短信内容>: 给好友发短信\n");
	} else if(strequ(cmd,"cd")) {
		print_gbk("cd <好友编号>: 开始和某人对话, 键入'cd ..'离开.\n");
	} else if(strequ(cmd,"!")) {
		print_gbk("!外部命令: 执行一个外部命令\n");
	} else if(strequ(cmd,"chcp")) {
		print_gbk("chcp: 改变当前字符集 (UTF8/GBK)\n");
	} else if(strequ(cmd,"self")) {
		print_gbk("self <短信内容>: 给自己发短信\n");
	} else if(strequ(cmd,"log")) {
		print_gbk("log: 是否保存聊天记录\n");
	} else if(strequ(cmd,"impresa")) {
		print_gbk("impresa [心情短语]: 察看/设置心情短语\n");
	} else if(strequ(cmd,"info")) {
		print_gbk("info <好友编号>: 察看好友信息\n");
	} else if(strequ(cmd,"help")) {
		for(i=0; i<ARRAY_SIZE(g_cmd_array); ++i)
		{
			print_gbk("%s\t", g_cmd_array[i]);
			if(7 == i%8)
				printf("\n");
		}
		print_gbk("\nhelp [命令]: 得到某个命令的帮助信息.\n");
	} else {
		print_gbk("未知命令\n");
	}
}


void on_cmd_chcp(void)
{
	if(CHARSET_UTF8==g_charset)
	{
		g_charset=CHARSET_GBK;
		print_utf8("active charset: GBK\n");
	}
	else
	{
		g_charset=CHARSET_UTF8;
		print_utf8("active charset: UTF8\n");
	}
}

void index_release(DList *list)
{
	DList *tmp=NULL;
	while(list)
	{
		free(list->data);
		tmp=list;
		list=list->next;
		free(tmp);
	}
}

DList* index_append(DList *list, long id, const char *name)
{
	index_item_t *item = NULL;
	DList *ret = NULL;

	if(!(item=(index_item_t *)malloc(sizeof(index_item_t))))
	{
		fprintf(stderr,"malloc() error\n");
		return NULL;
	}
	item->id = id;
	item->name = name;

	ret = d_list_append(list, (index_item_t*)item);

	return ret;
}

void on_cmd_ls(const char *arg)
{
	if(strequ(arg,""))
		on_cmd_ls_online();
	else if(strequ(arg,"a")||strequ(arg,"all"))
		on_cmd_ls_all();
	else if(strequ(arg,"g")||strequ(arg,"group"))
		on_cmd_ls_group();
	else if(strequ(arg,"q")||strequ(arg,"qun"))
		on_cmd_ls_qun();
	else if(strequ(arg,"b")||strequ(arg,"black"))
		on_cmd_ls_black();
	else
		helpinfo("ls");
}


void on_cmd_ls_online(void)
{
	int i=0;
	const Fetion_Account *account=NULL;
	char *name=NULL;

	index_release(g_index);
	g_index = NULL;
	g_current = NULL;

  	account = fx_get_first_account();

	print_gbk("+--------------------在线好友------------------------+\n");
  	while(account)
	{
		if(fx_is_on_line_by_account(account))
		{
			//to skip ourself.
			if(FTION_UTYPE_UNSET == account->usr_type)
				break;
			name = account->local_name;
			if(!(name) && account->personal)
				name = account->personal->nickname;

			print_utf8(" %d\t%d\t%s", ++i, account->id,	name);
			print_gbk( "\t\t%s\t%s\n", 
				get_state_string(account->online_status),
				get_usr_type_string(account->usr_type)
			);
			g_index = index_append(g_index,account->id,account->local_name);
			if(!(i%20))
				util_getch();

		}
  		account = fx_get_next_account(account);
  	}
	print_utf8("+----------------------------------------------------+\n");
	//util_debug("the length of the list is %d \n", d_list_length(g_index));
}


void on_cmd_ls_all(void)
{
	int i = 0;
	const Fetion_Account *account = NULL;
	char *name = NULL;
	int name_len = 0;

	index_release(g_index);
	g_index = NULL;
	g_current = NULL;

  	account = fx_get_first_account();

	print_gbk("+-------------------所有好友-----------------------+\n");
  	while(account)
	{
		/**
		 * 最后一个account是自己，跳过，否则会有错误.
		 * 这个bug在新版本的LibFetion里面已经解决了
		 */
		//if(FTION_UTYPE_UNSET == account->usr_type)
		//	break;

		name = account->local_name;

		// 如果用户没有设置显示名字,则显示用户的昵称
		// 但是现在判断的时候还有点问题...
		if(!name && account->personal)
			name = account->personal->nickname;

		print_utf8(" %d\t%d\t%s", ++i, account->id,	name);
		print_gbk( "\t\t%s\t%s\n", 
			get_state_string(account->online_status),
			get_usr_type_string(account->usr_type)
		);

		g_index = index_append(g_index, account->id, name);

		if(!(i%20))
			util_getch();
  		account = fx_get_next_account(account);
  	}
	print_utf8("+--------------------------------------------------+\n");
	//util_debug("the length of the list is %d \n", d_list_length(g_index));
}



void on_cmd_ls_group(void)
{
	Fetion_Group *group = NULL;
  	DList *iter=NULL;

	iter = (DList*)fx_get_group();

	print_gbk("+----------------好友分组----------------+\n");
  	while(iter)
	{
  		if(group=(Fetion_Group *)iter->data) {
			print_gbk(" 分组:");
			print_utf8("%d\t%s\n",group->id,group->name);
		}
  		iter = iter->next;
  	}
	print_utf8("+----------------------------------------+\n");
}


void on_cmd_ls_qun(void)
{
	int i=0;
	Fetion_Qun *qun=NULL;
  	DList *iter=NULL;

	index_release(g_index);
	g_index = NULL;
	g_current = NULL;

	iter = (DList*)fx_get_qun();

	print_gbk("+-----------------飞信群-----------------+\n");
  	while(iter)
	{
  		if(qun=(Fetion_Qun *)iter->data)
		{
			print_gbk(" %d\t群:",++i);
			print_utf8("%d\t%s\n",qun->id,qun->quninfo?qun->quninfo->name:"");
			g_index = index_append(g_index, qun->id, qun->quninfo?qun->quninfo->name:"");
  		}
  		iter = iter->next;
  	}
	print_utf8("+----------------------------------------+\n");

}

void on_cmd_ls_black(void)
{
	Fetion_Black *black = NULL;
  	DList *iter=NULL;

	iter = (DList*)fx_get_blacklist();

	print_gbk("+----------------黑名单------------------+\n");
  	while(iter)
	{
  		if(black=(Fetion_Black*)iter->data)
		{
			print_utf8("ID:%-11d\tName:%s\n",black->uid, black->local_name);
  		}
  		iter=iter->next;
  	}
	print_utf8("+----------------------------------------+\n");

}

void on_cmd_state(const char *arg2)
{
	if(strequ(arg2,"1"))
		fx_set_user_state(FX_STATUS_ONLINE, NULL, NULL, NULL);
	else if(strequ(arg2,"2"))
		fx_set_user_state(FX_STATUS_OFFLINE, NULL, NULL, NULL);
	else if(strequ(arg2,"3"))
		fx_set_user_state(FX_STATUS_BUSY, NULL, NULL, NULL);
	else if(strequ(arg2,"4"))
		fx_set_user_state(FX_STATUS_AWAY, NULL, NULL, NULL);
	else
		NULL;
}
void  on_cmd_info(const char *arg2)
{
	long id = atol(arg2);
	Fetion_Account *account = NULL;
	char *name = NULL;
	char *mobile_no = NULL;
	char *nickname = NULL;
	int gender = 0;
	char *impresa = NULL;

	id = index2id(id);
	if(!(account = (Fetion_Account*)fx_get_account_by_id(id)))
	{
		util_debug("fx_get_account_by_id() return NULL\n");
		return;
	}

	if(account->personal)
	{
		mobile_no = account->personal->mobile_no;
		nickname = account->personal->nickname;
		gender = account->personal->gender;
		impresa = account->personal->impresa;
	}
	// 目前的libfetion似乎Fetion_Personal里面的很多条目都没有实现,
	// 比如常用的手机号，性别.已经确认实现的有屏显名称,飞信号, 昵称,心情短语
	print_gbk("飞信号:%d\t昵称:", account->id);
	print_utf8("%s\t", nickname);
	print_gbk("显示名:");
	print_utf8("%s\n", account->local_name);
	print_gbk("心情短语:");
	print_utf8("%s\n", impresa	);
}
void on_cmd_cd(const char *arg2)
{
	int n=0;
	if(strequ("..", arg2) && g_current)
	{
		fx_end_dialog(g_current->id);
		g_current = NULL;
		return;
	}
	n = atoi(arg2);
	g_current = (index_item_t*)d_list_nth_data(g_index, n-1);
	if(g_current)
		fx_begin_dialog(g_current->id, NULL, NULL);
}

void on_cmd_send(const char *arg2, const char *arg3)
{
	long id = atol(arg2);
	const char *msg = arg3;
	if(id > 10000)
		return;
	g_current = (index_item_t*)d_list_nth_data(g_index, id-1);
	if(!g_current)
		return;
	fx_begin_dialog(g_current->id, NULL, NULL);
	fx_dialog_send(g_current->id, msg, NULL, NULL);
}

void on_cmd_sms(const char *arg2, const char *arg3)
{
	long id = atol(arg2);
	const char *msg = arg3;

	id = index2id(id);
	if(id > 100000)
		fx_send_sms(id, msg, NULL, NULL);

	//if((id > 100000) && fs_send_sms(id, msg))
	//	print_utf8("搞定!\n");
	//log_message(arg2);
	//log_message(arg3);
}

long index2id(long id)
{
	index_item_t *item = NULL;
	long index_len = (long) d_list_length(g_index);
	//id is an index
	if(id >= 1 && id <= index_len )
	{
		item = (index_item_t*)d_list_nth_data(g_index, id-1);
		if(item)
			id = item->id;
	}
	return id;
}

const char *get_state_string(int state)
{
	switch(state)
	{
		case FX_STATUS_UNSET:
			return "未设置";
		case FX_STATUS_ONLINE:
			return "在线";
		case FX_STATUS_BUSY:
			return "忙碌";
		case FX_STATUS_OFFLINE:
			return "离线";
		case FX_STATUS_AWAY:
			return "马上回来";
		case FX_STATUS_MEETING:
			return "会议中";
		case FX_STATUS_PHONE:
			return "电话中";
		case FX_STATUS_DINNER:
			return "外出用餐";
		case FX_STATUS_EXTENDED_AWAY:
			return "离开";
		case FX_STATUS_NUM_PRIMITIVES:
			return "自定义";
		default:
			return "";
	}
}
const char *get_usr_type_string(int usr_type)
{
	switch(usr_type)
	{
		case FTION_UTYPE_UNSET:
			return "UNSET";
		case FTION_UTYPE_PC:
			return "PC";
		case FTION_UTYPE_MOBLIE:
			return "MOBILE";
		default:
			return "";
	}
}

int translate(char *buf)
{
	char *u=NULL;
	if(CHARSET_UTF8 == g_charset)
		return ~0;
	//util_debug("start to encode input\n");
	if(!utf8_encode(buf,&u)) {
		snprintf(buf, TEXT_LEN, u);
		free(u);
	//	util_debug("encode input sucessfully\n");
		return ~0;
	} else {
		free(u);
	//	util_debug("encode input unsucessfully\n");
		return 0;
	}
}


/**
 * Print a UTF8 string in the screen
 */
void print_utf8(const char *fmt,...)
{
	va_list ap;
	char *gb=NULL;
	char buf[1024];

	va_start(ap, fmt);
	vsnprintf(buf,1024,fmt,ap);
	va_end(ap);
	if(CHARSET_UTF8 == g_charset)
	{
		printf(buf);
		return;
	}
	if(utf8_decode(buf,&gb))
		fprintf(stderr,"print_utf8() error\n");
	printf(gb);
	free(gb);
}

/**
 * Print a GBK string in the screen
 */
void print_gbk(const char *fmt,...)
{
	va_list ap;
	char *utf=NULL;
	char buf[1024];

	va_start(ap, fmt);
	vsnprintf(buf,1024,fmt,ap);
	va_end(ap);
	if(CHARSET_GBK == g_charset)
	{
		printf(buf);
		return;
	}
	if(utf8_encode(buf,&utf))
		fprintf(stderr,"print_utf8() error\n");
	printf(utf);
	free(utf);
}



void log_message(const char *fmt,...)
{
	va_list ap;
	FILE *fp=NULL;
	char log_name[BUF_LEN+5];

	if(!g_is_log) return;
	snprintf(log_name,BUF_LEN,g_user);
	strcat(log_name,".txt");
	if(!(fp=fopen(log_name,"a")))
		fprintf(stderr,"log_message() error\n");
	va_start(ap, fmt);
	vfprintf(fp,fmt,ap);
	va_end(ap);
	fclose(fp);
}


int is_a_cmd(const char *s)
{
	int i;
	for(i=0; i < ARRAY_SIZE(g_cmd_array); i++)
		if(strequ(g_cmd_array[i],s))
			return ~0;
	return 0;
}


int on_out_cmd(const char *input)
{
	while(isspace((unsigned int)*input))
		input++;
	if('!'==input[0])
	{
		++input;
		while(isspace((unsigned int)*input))
			input++;
		system(input);
		return ~0;
	} else
		return 0;
}


/*the function will destroy input string*/
int parse_input(char *input, char **arg1, char **arg2, char **arg3)
{
	int i;
	char *p=NULL,*brk1=NULL,*brk2=NULL;
	while(isspace((unsigned int)*input))
		input++;
	if(!input[0]) 
		return 0;
	*arg1=input;
	for(i=1;input[i];i++)
	{
		if(isspace((unsigned int)input[i]) && !isspace((unsigned int)input[i-1]))
		{
			if(!brk1&&!brk2)
				brk1=&input[i];
			else if(brk1&&!brk2)
				brk2=&input[i];
			else
				break;
		}
	}

	if(brk1)
	{
		*brk1='\0';
		p=brk1+1;
		while(isspace((unsigned int)*p))p++;
		if(!p[0]) return 1;
		*arg2=p;
	}
	else
		return 1;

	if(brk2)
	{
		*brk2='\0';
		p=brk2+1;
		while(isspace((unsigned int )*p))
			p++;
		if(!p[0])
			return 2;
		*arg3=p;
		return 3;
	}
	else
		return 2;
}

void usage(void)
{
	fprintf(stderr,"Usage:\n"FX_PACKAGE" -u phone -p[password] [-i] [-l] [-w who] [-t \"SMS text\"]\n"
		"       -u phone/fetion  login with phone number or fetion number\n"
		"       -p[password]     no space before [password]\n"
		"       -i               login invisible\n"
		"       -l               keep a chatlog\n"
		"       -w who           the Fetion number of the one you want to sent a SMS\n"
		"       -t \"SMS text\"    the SMS content\n"
	);
}


void init_options(int argc,char **argv)
{
	int c;
	const char *g_opt_str ="u:p::ilgw:t:d";

	if(argc<4)
	{
		usage();
		exit(1);
	}

#ifdef _WIN32
	g_charset = CHARSET_GBK;
#else
	g_charset = CHARSET_UTF8;
#endif

	while(1)
	{
		if(EOF==(c=mfx_getopt(argc,argv,g_opt_str)))
			break;
		switch(c) {
		case 'u':
			snprintf(g_user,BUF_LEN,mfx_optarg);
			print_utf8("user:%s\n",g_user);
			break;
		case 'p':
			if(mfx_optarg)
				snprintf(g_passwd,BUF_LEN,mfx_optarg);
			else  {
				print_utf8("password:");
				util_get_password(g_passwd,BUF_LEN);
				print_utf8("\n");
			}
			break;
		case 'i':
			g_is_invisible = ~0;
			print_utf8("login invisible\n");
			break;
		case 'l':
			g_is_log = ~0;
			print_utf8("keep a log\n");
			break;
//		case 'g':
//			g_charset = CHARSET_GBK;
//			print_utf8("charset:GBK\n");
//			break;
		case 'w':
			snprintf(g_who, BUF_LEN, mfx_optarg);
			print_utf8("to:%s\n",g_who);
			break;
		case 't':
			//如何完整显示 -t 后面的带空格信息呢?
			snprintf(g_text, TEXT_LEN, mfx_optarg);
			print_utf8("text:%s\n", g_text);
			break;
		case 'd':
			g_is_debug = ~0;
			print_utf8("run in debug mode\n");
			break;
		default:
			break;
		}
	}
	if(!g_user[0]||!g_passwd[0]){
		usage();
		exit(2);
	}
}

int is_online(int state)
{
	if (state != FX_STATUS_OFFLINE &&
		state != 0 &&
		state != FX_STATUS_WAITING_AUTH &&
		state != FX_STATUS_REFUSE &&
		state != FX_STATUS_BLACK &&
		state != FX_STATUS_MOBILE )
		return ~0;
	else
		return 0;
}

void on_new_message(long id)
{
	char *msg = NULL;
	Fetion_MSG *fxMsg = NULL;
	Fetion_Account *account = NULL;

	if(fx_is_qun_by_id(id))
	{
		on_new_qun_message(id);
		return;
	}
	if(!(account = (Fetion_Account*)fx_get_account_by_id(id)))
	{
		util_debug("on_new_message()->fx_get_account_by_id()\n");
		return;
	}
	if(!(fxMsg = (Fetion_MSG*)fx_get_msg(id)))
	{
		util_debug("on_new_message()->fx_get_msg()\n");
		return;
	}
	msg = fx_msg_no_format(fxMsg->message);
	print_gbk("\n来自 ");
	print_utf8("%s(%ld)", account->local_name, account->id);
	print_gbk(" 的新消息:\n");
	print_utf8(msg);
	print_utf8("\n");
	log_message(msg);
	if(msg)
		free(msg);
	fx_destroy_msg(fxMsg);
}

void show_message(long id)
{
	char *msg = NULL;
	Fetion_MSG *fxMsg = NULL;

	if(!(fxMsg = (Fetion_MSG*)fx_get_msg(id)))
		return;
	msg = fx_msg_no_format(fxMsg->message);
	//print_utf8(fxMsg->message);
	print_utf8(msg);
	log_message(msg);
	if(msg)
		free(msg);
	fx_destroy_msg(fxMsg);
}

void on_new_qun_message(long qun_id)
{
	long sender=0;
	char *msg=NULL;
	char *qun_name=NULL;
	char *sender_name=NULL;
	Fetion_MSG *fxMsg=NULL;
	Fetion_Qun *fx_qun=NULL;
	Fetion_QunInfo *qun_info=NULL;
	Fetion_QunMember *qun_member=NULL;
	DList *dl=NULL;

	if(!(fxMsg = (Fetion_MSG*)fx_get_msg(qun_id))) 
		return;
	sender = fxMsg->ext_id;
	msg = fx_msg_no_format(fxMsg->message);
	if(!(fx_qun = (Fetion_Qun*)fx_get_qun_by_id(qun_id)))
	{
		if(msg)
			free(msg);
		fx_destroy_msg(fxMsg);
		return;
	}
	qun_info = fx_qun->quninfo;
	qun_name = qun_info->name;
	dl = qun_info->QunMember;
	while(dl)
	{
		qun_member=(Fetion_QunMember*)dl->data;
		if(qun_member && qun_member->id==sender)
		{
			sender_name=(qun_member->iicnickname?qun_member->iicnickname:qun_member->nickname?qun_member->nickname:"");
			break;
		}
		dl=d_list_next(dl);
	}
	print_utf8(sender_name);
	print_utf8("(%ld)",sender);
	print_gbk("@群:");
	print_utf8(qun_name);
	print_utf8("(%ld)\n",qun_id);
	print_utf8(msg);
	print_utf8("\n");

	log_message(sender_name);
	log_message("(%ld)@群:",sender);
	log_message(qun_name);
	log_message("(%ld)\n",qun_id);
	log_message(msg);
	log_message("\n");

	if(msg) 
		free(msg);
	fx_destroy_msg(fxMsg);
}

void on_sys_message(long id)
{
	char *msg=NULL;
	Fetion_MSG *fxMsg=NULL;

	if(!(fxMsg = (Fetion_MSG*)fx_get_msg(id)))
		return;
	msg = fx_msg_no_format(fxMsg->message);
	print_gbk("系统消息:\n");
	print_utf8(msg);
	print_utf8("\n");
	if(msg)
		free(msg);
	fx_destroy_msg(fxMsg);
}



void on_set_state_ok(int state)
{
	print_gbk("设置状态成功,当前状态: ");
	print_gbk(get_state_string(state));
	print_utf8("\n");
}

void on_sys_err_network(int err)
{
	g_status=STATUS_RELOGIN;
	fx_relogin(cb_fx_relogin, NULL);
}

void on_sys_deregistered(void)
{
	g_status=STATUS_DEREGISTERED;
	print_utf8("you have login in other pc, i will quit\n");
}

void on_sys_dialog_message(int message, Fetion_MSG *fx_msg, long who)
{
	switch(message)
	{
		case FX_SMS_OK:
		case FX_DIA_SEND_OK:
		case FX_QUN_SEND_OK:
		case FX_QUN_SMS_OK:
			util_debug("send msg to %ld successfully\n", who);
			if(!fx_msg)
				return;
			fx_destroy_msg((Fetion_MSG *)fx_msg);
			break;
		case FX_SMS_FAIL:
		case FX_DIA_SEND_FAIL:
		case FX_QUN_SEND_FAIL:
		case FX_QUN_SMS_FAIL:
		case FX_SMS_FAIL_LIMIT:
		case FX_QUN_SMS_FAIL_LIMIT:
			print_utf8("fail to send msg to %ld\n", who);
			if(!fx_msg)
				return;
			fx_destroy_msg((Fetion_MSG *)fx_msg);
			break;
		case FX_SMS_TIMEOUT:
		case FX_DIA_SEND_TIMEOUT:
		case FX_QUN_SEND_TIMEOUT:
		case FX_QUN_SMS_TIMEOUT:
			print_utf8("send msg to %ld timeout,i will resend it for you...\n", who);
			//time out should not to destroy msg, beacuse the system will resend by itself..
			//the libfetion will resend the message???
			//if(!fx_msg)	return;
			//fx_destroy_msg((Fetion_MSG *)fx_msg);
			break;
	}
}

//void cb_cmd_state(int message, WPARAM wParam, LPARAM lParam, void* args)
//{
//	print_utf8("当前状态: %s\n", get_state_string( fx_get_user_state() ));
//}

void cb_cmd_self(int message, WPARAM wParam, LPARAM lParam, void* args)
{
	print_gbk("给自己发短信成功!\n");
}


//void cb_fx_dialog(int message, WPARAM wParam, LPARAM lParam, void* args)
//{
//	on_new_message((long)lParam);
//}

void cb_system_msg (int message, WPARAM wParam, LPARAM lParam, void* args)
{
	switch(message)
	{
		case FX_NEW_MESSAGE:
			on_new_message((long)lParam);
			break;
		case FX_NEW_QUN_MESSAGE:
			on_new_qun_message((long)lParam);
			break;
		case FX_SYS_MESSAGE:
			on_sys_message((long)lParam);
			break;
		case FX_SET_STATE_OK:
			on_set_state_ok((int)wParam);
			break;
		case FX_SYS_ERR_NETWORK:
			on_sys_err_network(wParam);
			break;
		case FX_SYS_DEREGISTERED:
			on_sys_deregistered();
			break;

		case FX_ADDACCOUNT_APP:
			//emit signal_AddAccountApp((char*)(lParam), (char*)wParam);
			break;
		case FX_STATUS_OFFLINE:
		case FX_STATUS_ONLINE :
		case FX_STATUS_BUSY:
		case FX_STATUS_AWAY:
		case FX_STATUS_MEETING:
		case FX_STATUS_PHONE:
		case FX_STATUS_DINNER:
		case FX_STATUS_EXTENDED_AWAY:
		case FX_STATUS_NUM_PRIMITIVES:
		case FX_ACCOUNT_UPDATA_OK:
			//emit signal_UpdateAcInfo(qlonglong(lParam));
			break;

		case FX_STATUS_SMSEXTENED:
			//printf("have receive the FX_STATUS_SMSEXTENED message of %d %ld\n", wParam, lParam);
			//emit signal_UpdateAcInfo(qlonglong(lParam));
			break;

		case FX_SET_REFUSE_SMS_DAY_OK:
			util_debug("FX_SET_REFUSE_SMS_DAY_OK:0x%04X\n",FX_SET_REFUSE_SMS_DAY_OK);
			break;
		case FX_SET_REFUSE_SMS_DAY_FAIL:
			util_debug("FX_SET_REFUSE_SMS_DAY_FAIL:0x%04X\n",FX_SET_REFUSE_SMS_DAY_FAIL);
			break;
		case FX_DIA_SEND_OK:
		case FX_DIA_SEND_FAIL:
		case FX_DIA_SEND_TIMEOUT:
		case FX_SMS_OK:
		case FX_SMS_FAIL:
		case FX_SMS_FAIL_LIMIT:
		case FX_SMS_TIMEOUT:
		case FX_QUN_SEND_OK:
		case FX_QUN_SEND_FAIL:
		case FX_QUN_SEND_TIMEOUT:
		case FX_QUN_SMS_OK:
		case FX_QUN_SMS_FAIL:
		case FX_QUN_SMS_FAIL_LIMIT:
		case FX_QUN_SMS_TIMEOUT:
			on_sys_dialog_message(message, (Fetion_MSG*)wParam, (long)lParam);
			break;
		case FX_REMOVE_BLACKLIST_OK:
			util_debug("FX_REMOVE_BLACKLIST_OK:0x%04X\n",FX_REMOVE_BLACKLIST_OK);
			break;
		case FX_ADD_BLACKLIST_OK:
			util_debug("FX_ADD_BLACKLIST_OK:0x%04X\n",FX_ADD_BLACKLIST_OK);
			break;
		case FX_CURRENT_VERSION:
			if(LIBFETION_VERSION < (int)wParam)
				print_utf8("LibFetion have new version, access http://www.libfetion.cn for more infomation.\n");
			break;
		case FX_ADD_BUDDY_OK:
			util_debug("FX_ADD_BUDDY_OK:0x%04X\n",FX_ADD_BUDDY_OK);
			break;
		case FX_MOVE_GROUP_OK:
			util_debug("FX_MOVE_GROUP_OK:0x%04X\n",FX_MOVE_GROUP_OK);
			break;
		case FX_MOVE_GROUP_FAIL:
			util_debug("FX_MOVE_GROUP_FAIL:0x%04X\n",FX_MOVE_GROUP_FAIL);
			break;
		case FX_RENAME_GROUP_OK:
			util_debug("FX_RENAME_GROUP_OK:0x%04X\n",FX_RENAME_GROUP_OK);
			break;
		case FX_SET_BUDDY_INFO_OK:
			util_debug("FX_SET_BUDDY_INFO_OK:0x%04X\n",FX_SET_BUDDY_INFO_OK);
			break;
		case FX_ADD_GROUP_OK:
			util_debug("FX_ADD_GROUP_OK:0x%04X\n",FX_ADD_GROUP_OK);
			break;
		case FX_DEL_GROUP_OK:
			util_debug("FX_DEL_GROUP_OK:0x%04X\n",FX_DEL_GROUP_OK);
			break;
		case FX_DEL_BUDDY_OK:
			util_debug("FX_DEL_BUDDY_OK:0x%04X\n",FX_DEL_BUDDY_OK);
			break;
		case FX_ADD_GROUP_FAIL:
			util_debug("FX_ADD_GROUP_FAIL:0x%04X\n",FX_ADD_GROUP_FAIL);
			if(wParam)
				free((char*)(int)wParam);
			break;
		case FX_DEL_GROUP_FAIL:
			util_debug("FX_DEL_GROUP_FAIL:0x%04X\n",FX_DEL_GROUP_FAIL);
			if(wParam)
				free((char*)(int)wParam);
			break;
		case FX_SET_BUDDY_INFO_FAIL:
			util_debug("FX_SET_BUDDY_INFO_FAIL:0x%04X\n",FX_SET_BUDDY_INFO_FAIL);
			if(wParam)
				free((char*)(int)wParam);
			break;
		case FX_RENAME_GROUP_FAIL:
			util_debug("FX_RENAME_GROUP_FAIL:0x%04X\n",FX_RENAME_GROUP_FAIL);
			if(wParam)
				free((char*)(int)wParam);
			break;
		case FX_REMOVE_BLACKLIST_FAIL:
			util_debug("FX_REMOVE_BLACKLIST_FAIL:0x%04X\n",FX_REMOVE_BLACKLIST_FAIL);
			break;
		case FX_ADD_BLACKLIST_FAIL:
			util_debug("FX_ADD_BLACKLIST_FAIL:0x%04X\n",FX_ADD_BLACKLIST_FAIL);
			break;
		case FX_REMOVE_BLACKLIST_TIMEOUT:
			util_debug("FX_REMOVE_BLACKLIST_TIMEOUT:0x%04X\n",FX_REMOVE_BLACKLIST_TIMEOUT);
			break;
		case FX_ADD_BLACKLIST_TIMEOUT:
			util_debug("FX_ADD_BLACKLIST_TIMEOUT:0x%04X\n",FX_ADD_BLACKLIST_TIMEOUT);
			break;
		case FX_RENAME_GROUP_TIMEOUT:
			util_debug("FX_RENAME_GROUP_TIMEOUT:0x%04X\n",FX_RENAME_GROUP_TIMEOUT);
			break;
		case FX_SET_BUDDY_INFO_TIMEOUT:
			util_debug("FX_SET_BUDDY_INFO_TIMEOUT:0x%04X\n",FX_SET_BUDDY_INFO_TIMEOUT);
			break;
		default:
			break;
	}
}

void cb_fx_login(int message, WPARAM wParam, LPARAM lParam, void* args)
{
	switch(message)
	{
	case FX_LOGIN_URI_ERROR:
		util_debug("FX_LOGIN_URI_ERROR:0x%04X\n",FX_LOGIN_URI_ERROR);
		print_utf8("login fail because of wrong phone No. or fetion No. \n");
		exit(FX_LOGIN_URI_ERROR);
		break;
	case FX_LOGIN_CONNECTING:
		util_debug("FX_LOGIN_CONNECTING:0x%04X\n",FX_LOGIN_CONNECTING);
		print_utf8("connecting the server... \n");
		break;
	case FX_LOGIN_WAIT_AUTH:
		util_debug("FX_LOGIN_WAIT_AUTH:0x%04X\n",FX_LOGIN_WAIT_AUTH);
		print_utf8("validating account and password... \n");
		break;
	case FX_LOGIN_AUTH_OK:
		util_debug("FX_LOGIN_AUTH_OK:0x%04X\n",FX_LOGIN_AUTH_OK);
		print_utf8("validate account and password successfully \n");
		break;
	case FX_LOGIN_FAIL:
		util_debug("FX_LOGIN_FAIL:0x%04X\n",FX_LOGIN_FAIL);
		print_utf8("login fail\n");
		exit(FX_LOGIN_FAIL);
		break;
	case FX_LOGIN_NETWORK_ERROR:
		util_debug("FX_LOGIN_NETWORK_ERROR:0x%04X\n",FX_LOGIN_NETWORK_ERROR);
		print_utf8("network error, check your network connection please \n");
		exit(FX_LOGIN_NETWORK_ERROR);
		break;
	case FX_LOGIN_UNKOWN_ERROR :
		util_debug("FX_LOGIN_UNKOWN_ERROR:0x%04X\n",FX_LOGIN_UNKOWN_ERROR);
		print_utf8("unknown error occurs\n");
		exit(FX_LOGIN_UNKOWN_ERROR);
		break;
	case FX_LOGIN_UNKOWN_USR:
		util_debug("FX_LOGIN_UNKOWN_USR:0x%04X\n",FX_LOGIN_UNKOWN_USR);
		print_utf8("wrong user account\n");
		exit(FX_LOGIN_UNKOWN_USR);
		break;
	case FX_LOGIN_GCL_GETTING:
		util_debug("FX_LOGIN_GCL_GETTING:0x%04X\n",FX_LOGIN_GCL_GETTING);
		print_utf8("getting the connection list...\n");
		break;
	case FX_LOGIN_GCL_OK:
		util_debug("FX_LOGIN_GCL_OK:0x%04X\n",FX_LOGIN_GCL_OK);
		print_utf8("get the connection list ok\n");
		break;
	case FX_LOGIN_GCL_FAIL:
		util_debug("FX_LOGIN_GCL_FAIL:0x%04X\n",FX_LOGIN_GCL_FAIL);
		print_utf8("fail to get the connection list, type 'exit' to quit the program\n");
		break;
	case FX_LOGIN_GP_GETTING:
		util_debug("FX_LOGIN_GP_GETTING:0x%04X\n",FX_LOGIN_GP_GETTING);
		print_utf8("getting the presence info...\n");
		break;
	case FX_LOGIN_GP_OK:
		util_debug("FX_LOGIN_GP_OK:0x%04X\n",FX_LOGIN_GP_OK);
		print_utf8("get the presence info ok\n");
		break;
	case FX_LOGIN_GP_FAIL:
		util_debug("FX_LOGIN_GP_FAIL:0x%04X\n",FX_LOGIN_GP_FAIL);
		print_utf8("fail to get the presence info,type 'exit' to quit the program\n");
		break;
	case FX_LOGIN_OK:
		util_debug("FX_LOGIN_OK:0x%04X\n",FX_LOGIN_OK);
		print_utf8("login successfully!\nServer address:%s\ntype 'help' to get help information...\n",
			fx_get_serve_address());
		g_status = STATUS_ONLINE;
		fx_set_system_msg_cb(cb_system_msg, NULL);
		break;
	}
}
void cb_fx_relogin(int message, WPARAM wParam, LPARAM lParam, void* args)
{
	static int relogin_try;
	if(relogin_try>10)
	{
		print_utf8("you have tried to many times, maybe you should check you network connection\n");
		g_status = STATUS_OFFLINE;
		return;
	}
	switch(message)
	{
		case FX_LOGIN_URI_ERROR:
		case FX_LOGIN_FAIL:
		case FX_LOGIN_NETWORK_ERROR:
		case FX_LOGIN_UNKOWN_ERROR :
		case FX_LOGIN_UNKOWN_USR:
		case FX_LOGIN_GP_FAIL:
			fx_relogin(cb_fx_relogin, NULL);
			g_status=STATUS_RELOGIN;
			relogin_try++;
			break;
		case FX_LOGIN_OK :
			print_utf8("relogin successfully\n");
			g_status = STATUS_ONLINE;
			relogin_try=0;
			break;
		default:
			break;
	}

}

int util_getch(void)
{
#ifdef _WIN32
	return _getch();
#else
	struct termios oldt,newt;
	int ch;

	tcgetattr(STDIN_FILENO,&oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr(STDIN_FILENO,TCSANOW,&newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
	return ch;
#endif
}

int util_get_password(char* pwd, int len)
{
	int i=0;
	char ch;

	while(i < len)
	{
		ch = util_getch();
		if(ch == '\r' || ch == '\n')
		{
			pwd[i] = '\0';
			break;
		}
		else if(ch == '\b')
		{
			printf("\b");
			--i;
		}
		else
		{
			pwd[i++] = ch;
			printf("*");
		}
	}
	pwd[len-1] = '\0';
	return i;
}

void util_debug(char* info, ...)
{
	va_list ap;
	if(!g_is_debug)
		return;
	//fprintf(stderr,"\n+----------Debug Information----------+\n");
	va_start(ap, info);
	vfprintf(stderr,info,ap);
	va_end(ap);
	//fprintf(stderr,"\n+-------------------------------------+\n");
}
