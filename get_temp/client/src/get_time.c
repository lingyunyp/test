/*********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  get_time.c
 *    Description:  This file is get current systrm time 
 *                 
 *        Version:  1.0.0(2023年01月05日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月05日 17时21分50秒"
 *                 
 ********************************************************************************/

#include<time.h>
#include <stdio.h>

#include "get_time.h"

void get_time(char *datime)
{
  time_t seconds;
  struct tm *tTM;

  time(&seconds);
  tTM=gmtime(&seconds);
  snprintf(datime,32,"%04d-%02d-%02d  %02d:%02d:%02d\n",1900+tTM->tm_year,1+tTM->tm_mon,tTM->tm_mday,8+tTM->tm_hour,tTM->tm_min,tTM->tm_sec);
  
  return ;
}



