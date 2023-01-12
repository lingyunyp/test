/*********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  get_temperature.c
 *    Description:  This file get temperature
 *                 
 *        Version:  1.0.0(2023年01月05日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月05日 16时39分48秒"
 *                 
 ********************************************************************************/

#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include"get_temperature.h"


int get_temperature(float *temp)
{
	char             path[64]="/sys/bus/w1/devices/";
	char             buf[128];
    char             chip[32];
    DIR             *dirp=NULL;
    struct dirent   *direntp=NULL;
    int              fd=-1;
    char            *ptr=NULL;
    int              found=0;

	if(NULL==(dirp=opendir(path)))
	{
    	printf("opendir failure:%s\n",strerror(errno));
    	return -1;
	}

	memset(chip,0,sizeof(chip));
	while((direntp=readdir(dirp))!=NULL)
	{
    	if(strstr(direntp->d_name,"28-"))
    	{
        	strcpy(chip,direntp->d_name);
            found=1; 
    	}
	}
     
	closedir(dirp);

	if(!found)
	{
    	printf("readdir failure\n");
    	return -2;
    }
  

                             
	strncat(path,chip,sizeof(path)-strlen(path));
	strncat(path,"/w1_slave",sizeof(path)-strlen(path));

	if((fd=open(path,O_RDONLY))<0)
	{
    	printf("open file failure:%s\n",strerror(errno));
    	return -3;
	} 

	memset(buf,0,sizeof(buf));
	if(read(fd,buf,sizeof(buf))<0)
	{
    	printf("read file failure:%s\n",strerror(errno));
    	return -4;
	} 

	ptr=strstr(buf,"t=");
	ptr+=2;
 
	if(!ptr)
	{
    	printf("error:can not ptr\n");
    	return -5;
	} 

	*temp=atof(ptr)/1000;
	close(fd);

	return 0;
}
