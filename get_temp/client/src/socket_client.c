/*********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  client_init.c
 *    Description:  This file client initial function
 *                 
 *        Version:  1.0.0(2023年01月05日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月05日 19时34分07秒"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "main.h"
#include "socket_client.h"


int client_init(int port, char *serv_ip)
{
    int                 con_fd = -1;
    int 		        rv = -1;
    struct sockaddr_in  serv_addr;

    con_fd=socket(AF_INET, SOCK_STREAM, 0);

    if(con_fd < 0)
    { 
        printf("Create socket failure: %s\n", strerror(errno));
        return -1;
    }
    printf("Create socket[%d] sucessfully!\n", con_fd); 
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(port);
    inet_aton(serv_ip, &serv_addr.sin_addr);
    
    if(con_fd >= 0)
    {
        rv=connect(con_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        if(rv < 0)
        {
            printf("Connect to server[%s:%d] failure: %s\n", serv_ip, port, strerror(errno));
            return -2;
        }
        printf("Connect to server[%s:%d] sucessfully!\n", serv_ip, port);
    }

    return con_fd;
}


int get_sock_status(int sock_fd)
{
	
    struct tcp_info   info;
    int 	          len = sizeof(info);

	if(sock_fd < 0)
	{
		printf("The sendata() argument connfd incorrect!\n");
		return -1;
	}

   getsockopt(sock_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
   if((info.tcpi_state == TCP_ESTABLISHED))
	{
		printf("Socket connected\n");
        return 0;
    }
	else
	{
		printf("socket disconnected\n");
		return -2;
	}
}


int send_data(int sock_fd, pack_info_t pack_info)
{
	int 	rv=-1;
	char 	buf[100];

	if(sock_fd < 0)
	{
		printf("The sendata() argument connfd incorrect!\n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	snprintf(buf,sizeof(buf), "%s/%s/%f",  pack_info.sn, pack_info.datime, pack_info.temper);
	

	while(strlen(buf)!=0)//判断buf中是否有数据
	{
		rv = write(sock_fd,buf,strlen(buf));
		if(rv<0)
		{
			printf("Send data to sever failure:%s\n", strerror(errno));
			return -1;
		} 
	}
	
	printf("Send data to sever successfully:%s\n",buf);
	return 0;
}

