/********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  main.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(2023年01月11日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月11日 21时38分52秒"
 *                 
 ********************************************************************************/


#ifndef  _MAIN_H_
#define  _MAIN_H_

typedef struct pack_info_s
{
        char    sn[16];
        char    datime[32];
        float   temper;
}pack_info_t;

void set_socket_rlimit(void);

void print_usage(char *programe);

void signal_stop(int signum);

#endif
