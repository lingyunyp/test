/*********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file monitors the PRI temperature and is sent to the server at regular intervals
 *                 
 *        Version:  1.0.0(2023年01月05日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月05日 21时41分35秒"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <execinfo.h>
#include <libgen.h>

#include "main.h"
#include "sqlite3.h"
#include "get_temperature.h"
#include "get_time.h"
#include "socket_client.h"
#include "database.h"


#define SN  			"RPI1"
#define database_name    	"temper.db"
#define table_name       	"temperature"

int g_sigstop=0;


int main(int argc,char **argv)
{    
	int			sock_fd = -1;
	int			log_fd=-1;
	char                   *progname = NULL;
	int			ch;
	int			con_status = 1;
	int			rv;
	char                   *serv_ip = NULL;
  	int			serv_port = 0;
	int			daemon_run = 0;

	char                  **pIP = NULL;
	char                	str[32];
	char                   *domain = NULL;
	struct hostent         *host = NULL;

	int			interv_time=0;
	char 			datime[64];
	float 			temper;
	pack_info_t       	pack_info;
	struct sigaction    	sigact, sigign;

	static struct option long_options[] = {
            {"daemon", no_argument, NULL, 'd'},
            {"ipaddr", required_argument, NULL, 'i'},
            {"port", required_argument, NULL, 'p'},
            {"help", no_argument, NULL, 'h'},
            {"interv_time", required_argument, NULL, 't'},
            {"domain", required_argument, NULL, 'b'},
			{0, 0, 0, 0}
        };

	progname = basename(argv[0]);

	while((ch=getopt_long(argc, argv, "di:p:ht:b:", long_options, NULL)) != -1)
    	{
    		switch (ch) 
            	{
            		case 'd':
                    		daemon_run = 1;
                    		break;
                	case 'i': 
                    		serv_ip=optarg;
                    		break;
                	case 'p': 
                    		serv_port=atoi(optarg); //optarg返回的是字符串，atoi()将char转换为int型
                    		break;
                	case 'h': 
				print_usage(progname);
                    		break;
                	case 't':
                    		interv_time=atoi(optarg);
                    		break;
			case 'b':
		     		domain = optarg;
				break;
                	default:
                   		break;
            	}
            
	}

	
	if( !serv_port | !(!serv_ip ^ !domain))
	{
		print_usage(progname);
		return 0;
	}

	//没有输入IP地址，输入域名并解析
    	if(!serv_ip)
    	{
		if((host = gethostbyname(domain)) == NULL)
        	{
	    		printf("gethostbyname error: %s\n", strerror(errno));
	    		return -1;
	 	}
	    	switch(host->h_addrtype)
	    	{
	    		case AF_INET:
       	 		case AF_INET6:
		    			pIP = host->h_addr_list;
					for(; *pIP != NULL; pIP++)
						printf("IP address:%s\n", inet_ntop(host->h_addrtype, *pIP , str , sizeof(str)));
	        			serv_ip = str;
		    			break;
	       		default:
		    		printf("unknown address type\n");
		    	break;
        	}
    	}

	//判断是否打开守护进程函数
	if(daemon_run)
	{
		if(daemon(1, 1)<0)
		{
			printf("Running daemon failure:%s\n", strerror(errno));
			return 0;
		}
		printf("Running daemon successfully!\n");

		
	}
	
	//创建日志文件
	log_fd = open("temper.log",  O_CREAT|O_RDWR, 0666);
    	if (log_fd < 0)
    	{
    		printf("Open the logfile failure : %s\n", strerror(errno));

        	return 0;
    	}

    	//标准输出及标准出错重定向，重定向至日志文件
    //	dup2(log_fd, STDOUT_FILENO);
    	dup2(log_fd, STDERR_FILENO);
	
	//安装信号
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = signal_stop;

	sigemptyset(&sigign.sa_mask);
	sigign.sa_flags = 0;
	sigign.sa_handler = SIG_IGN;

	sigaction(SIGINT,  &sigact, 0); 
	sigaction(SIGPIPE, &sigign, 0); 
	sigaction(SIGTERM, &sigact, 0); 

	//检查数据库是否存在，如果存在返回0，不存在返回-1
	printf("Check database existence\n");
	if (access(database_name, W_OK) < 0)
	{
    		if(rv=create_database_table() < 0)//创建数据库和表
        	{
            		printf("Create database and table failure: %s\n", strerror(errno));
            		return -1;
        	}	
    	}
	printf("Create database and table OK\n");


	while(!g_sigstop)
	{
		//获取系统时间
		get_time(datime);
		//获取温度
        	if((get_temperature(&temper)) < 0)
	    	{
	    		printf("Get temperature failure: %s\n", strerror(errno));
		    	continue;
	    	}

		memset(&pack_info, 0, sizeof(pack_info));
		strcpy(pack_info.sn,SN );
		strcpy(pack_info.datime, datime);
		pack_info.temper = temper;

		//判断与服务端是否连接
		if(sock_fd<0)
		{
			if(sock_fd=client_init(serv_port, serv_ip)<0)
			{
				printf("connect server point  fialure: %s\n", strerror(errno));
                		con_status = 0;
				continue;
			}

		}
		else if(get_sock_status(sock_fd)==-2)//判断与服务端是否断开连接，没有连接返回-2，连接返回0
		{
			con_status=0;
		}


		//判断与服务端是否断开连接
		if(con_status)
		{
			if(send_data(sock_fd,pack_info)<0)//上传数据到服务端
        		{
            			con_status = 0;
				data_insert(pack_info,table_name);
				close(sock_fd);
            		}	

		}
		else
		{
			data_insert(pack_info, table_name);//与服务端断开连接，保存数据到数据库
			close(sock_fd);
		}

		//检查表中是否有数据
		if(check_table_data(table_name)>0)
		{
			//判断与服务端是否断开连接，没有连接返回-2，连接返回0
			if (get_sock_status(sock_fd) == -2)
			{
				con_status = 0;
			}

			if (con_status)
			{
				get_table_data(&pack_info,  table_name);//获取数据库表的第一行内容
				if (send_data(sock_fd,pack_info) < 0)
				{
					con_status = 0;
					close(sock_fd);
				}
				else
				{
					delete_table_data(table_name);//删除表第一行数
				}
			}
			else
			{
				sock_fd=client_init(serv_port, serv_ip);
				if (sock_fd< 0)
				{
					con_status = 0;
					close(sock_fd);
				}
				else
				{
					con_status = 1;
				}
			}
		}

		sleep(interv_time);
	}


	close(sock_fd);
//	close_sqlite_db();

	return 0;

}

void print_usage(char *programe)
{
	printf("Usage:%s[OPTION]....\n", programe);
	printf("-p[port]:Socket sever port address\n");
	printf("-d[daemon]:Start daemon!\n");
	printf("-b[damain]:Domain name resolution\n");
	printf("-i[ip] Socket client ip address\n");
	printf("-t[interv_time] RPI temperature interval time\n");
	printf("-h[help]:Display this help information\n");
	printf("\nExample:%s -d -p 6667 -t 30 -i 127.0.0.1 -b www.baidu.com\n", programe);
	return;
}

void signal_stop(int signum)
{
    if(SIGTERM == signum)
    {
        printf("SIGTERM signal detected\n");
        g_sigstop = 1;
    }
    if(SIGINT == signum)
    {
        printf("SIGINT signal detected\n");
        g_sigstop = 1;
    }

}
