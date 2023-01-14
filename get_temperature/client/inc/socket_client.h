/********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  client_init.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(2023年01月05日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月05日 20时50分38秒"
 *                 
 ********************************************************************************/

#ifndef  _SOCKET_CLIENT_H_
#define  _SOCKET_CLIENT_H_

#include "main.h"

//初始化client
int client_init(int port, char *serv_ip);

//获取与客户端连接状态
int get_sock_status(int sock_fd);

//上传数据给客户端
int send_data(int sock_fd, pack_info_t pack_info);

#endif
