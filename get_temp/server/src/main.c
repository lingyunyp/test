/*********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2023年01月10日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月10日 19时51分51秒"
 *                 
 ********************************************************************************/


#include <stdio.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include <syslog.h>
#include <libgen.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>

#include "main.h"
#include "database.h"
#include "socket_server.h"
#include "sqlite3.h"

#define database_name		"temperature.db"
#define table_name			"temperature"
#define MAX_EVENTS			512

int     					g_sigstop = 0;


int main (int argc, char **argv)
{
	int						daemon_run=0;
	int						opt;
	char					*progname = NULL;
	int						log_fd=1;
	struct sigaction		sigact,sigign;

	int						listenfd,connfd;
	int						serv_port;
	int						serv_ip;

	int						epollfd;
	struct epoll_event		event;
	struct epoll_event		event_array[MAX_EVENTS];
	int						events;

	char					buf[512];
	pack_info_t				pack_info;
	char					*ptr=NULL;

	struct option long_options[] =
 	{
 		{"daemon", no_argument, NULL, 'd'},
 		{"port", required_argument, NULL, 'p'},
 		{"help", no_argument, NULL, 'h'},
 		{NULL, 0, NULL, 0}
 	};

	progname = basename(argv[0]);

 	/* Parser the command line parameters */
 	while ((opt = getopt_long(argc, argv, "dp:h", long_options, NULL)) != -1)
 	{
		switch (opt)
 		{
 			case 'd':
 				daemon_run=1;
 				break;
 			case 'p':
 				serv_port = atoi(optarg);
				break;
 			case 'h': /* Get help information */
 				print_usage(progname);
 				return EXIT_SUCCESS;
 			default:
 				break;
 		}
 	}
 
	if( !serv_port )
 	{
 		print_usage(progname);
 		
		return -1;
 	}


	
	//建立日志系统
	log_fd=open("temperature.log", O_CREAT|O_RDWR, 0666);
    if(log_fd < 0)
    {
    	printf("Open the logfile failure: %s\n", strerror(errno));
        return 0;
    }
        
	//标准输出、标准出错重定向
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);

	//安装信号
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = signal_stop;

    sigemptyset(&sigign.sa_mask);
    sigign.sa_flags = 0;
    sigign.sa_handler = SIG_IGN;

    sigaction(SIGINT, &sigign, 0);
    sigaction(SIGPIPE, &sigign, 0);
    sigaction(SIGTERM, &sigact, 0);

	//检查数据库是否存在，如果存在返回0，不存在返回-1
    printf("Check database existence\n");
    if (access(database_name, W_OK) < 0)
    {
    	if(create_database_table() < 0)//创建数据库和表
        {
            printf("Create database and table failure: %s\n", strerror(errno));
            return -1;
        }
    }
    printf("Create database and table successfully\n");

	set_socket_rlimit(); /* set max open socket count */

	listenfd=socket_server_init(NULL,serv_port);
	if(listenfd<0)
	{
		printf("ERROR: %s server listen on port %d failure\n", argv[0],serv_port);
		return -2;
	}
	printf("%s server start to listen on port %d\n", argv[0],serv_port);

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

	//创建epoll实例
	epollfd=epoll_create(MAX_EVENTS);
	if(epollfd<0)
	{
		printf("epoll_create() failure: %s\n", strerror(errno));
		return -3;
	}

	event.events=EPOLLIN;
	event.data.fd=listenfd;

	//把listenfd加入epoll兴趣列表
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&event)<0)
	{
		printf("epoll add listen socket failure: %s\n", strerror(errno));
		return -4;
	}

	while(!g_sigstop)
	{
		//程序在这阻塞
		events=epoll_wait(epollfd,event_array,MAX_EVENTS,-1);
		if(events<0)
		{
			printf("epoll failure :%s\n",strerror(errno));
			break;
		}
		else if(events==0)
		{
			printf("epoll get timeout\n");
			continue;
		}
		
		//处理发生的事件
		for(int i=0;i<events;i++)
		{
			if ( (event_array[i].events & EPOLLERR) || (event_array[i].events & EPOLLHUP) )
			{
				printf("epoll_wait get error on fd[%d]: %s\n", event_array[i].data.fd, strerror(errno));
				epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
				close(event_array[i].data.fd);
			}

			
			if(event_array[i].data.fd==listenfd)
			{
				if((connfd=accept(listenfd,NULL,NULL))<0)/*不保存客户端信息（结构体指针为NULL)*/
				{
					printf("accept new client failure: %s\n", strerror(errno));
					continue;
				}

				event.data.fd=connfd;
				event.events=EPOLLIN;
				
				if( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
				{
					printf("epoll add client socket failure: %s\n", strerror(errno));
					close(event_array[i].data.fd);
					continue;
				}
				printf("epoll add new client socket[%d] ok.\n", connfd);
			}
			else
			{
				memset(buf,0,sizeof(buf));
				if(read(event_array[i].data.fd,buf,sizeof(buf))<=0)
				{
					printf("socket[%d] read data failure or disconnect and will be remove.\n", event_array[i].data.fd);
                	epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
                	close(event_array[i].data.fd);
                	continue;
				}

				
				//分割字符串
				ptr=strtok(buf,"/");
				while(NULL!=ptr)
				{
					strncpy(pack_info.sn, ptr, sizeof(pack_info.sn));
				    ptr=strtok(NULL, "/");
					strncpy(pack_info.datime, ptr, sizeof(pack_info.datime));
				    ptr=strtok(NULL, "/");
				    pack_info.temper=atof(ptr);
				    ptr=strtok(NULL, "/");
				}

				//保存到表中
				data_insert(pack_info, table_name);


			}


		}

	}


	close(listenfd);
    close_sqlite_db();


	return 0;
} 


void print_usage(char *progname)
{
	printf("Usage: %s [OPTION]...\n", progname);
    printf(" %s is a socket server program, which used to verify client and echo back string from it\n",progname);
    printf("\nMandatory arguments to long options are mandatory for short options too:\n");
    printf(" -d[daemon ] set program running on background\n");
    printf(" -p[port ] Socket server port address\n");
    printf(" -h[help ] Display this help information\n");
    printf("\nExample: %s -b -p 8900\n", progname);
    return ;
}


void signal_stop(int signum)
{
    if(SIGTERM == signum)
    {
        printf("SIGTERM signal detected\n");
        g_sigstop = 1;
    }
}


/* Set open file description count to max */
void set_socket_rlimit(void)
{
	struct rlimit limit = {0};
 	getrlimit(RLIMIT_NOFILE, &limit );
 	limit.rlim_cur = limit.rlim_max;
 	setrlimit(RLIMIT_NOFILE, &limit );
 	printf("set socket open fd max count to %ld\n", limit.rlim_max);
}
