/*********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  database.c
 *    Description:  This database.c file 
 *                 
 *        Version:  1.0.0(2023年01月07日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月07日 21时39分57秒"
 *                 
 ********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sqlite3.h>

#include "main.h"
#include "database.h"
#include "sqlite3.h"


#define database_name    "temper.db"


sqlite3 *db; 


int create_database_table(void)
{
    sqlite3		*db=NULL;
    char		*zErrMsg = NULL;
    int			rc;
    int			len;
    char		sql[512];  

    //  Open database  若没有则创建
    len = sqlite3_open(database_name, &db);
    if(len != SQLITE_OK)
    {
        sqlite3_close(db);
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return  -1;  //子线程不能调用exit(0)
    }
    printf("Opened database successfully\n");

    
	snprintf(sql,sizeof(sql) ,"CREATE TABLE if not exists temperature(SN CHAR(10),DATIME CHAR(50),TEMPERATURE CHAR(10));");
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        sqlite3_close(db);
        fprintf(stderr, "Create table error: %s\n", zErrMsg);
        return 0;
   }
    printf("Table created successfully\n");

    
    return 1;
}


int data_insert(pack_info_t pack_info, char *table_name)
{
	char 	sql[512];
	int 	rc =-1;
	char 	*zErrMsg = 0;

	if(NULL == table_name)
	{
		printf("The data_insert() argument incorrect!\n");
		return -1;
	}

	snprintf(sql,sizeof(sql),"INSERT INTO %s VALUES ('%s', '%s', '%f');" ,\
	table_name, pack_info.sn, pack_info.datime, pack_info.temper);
	printf("data insert sql:%s\n", sql);

	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   	if ( rc != SQLITE_OK )
	{
      	printf("Sqlite_insert: %s\n", zErrMsg);
      	sqlite3_free(zErrMsg);
		return -2;
   	}

	printf("Last data insert table successfully: %s, %s, %02f" ,\
	pack_info.sn, pack_info.datime, pack_info.temper);
	return 0;
}


int check_table_data(char *table_name)
{
	char 	sql[100];
	int 	rc =-1;
	char 	*zErrMsg = 0;
	char 	**dbResult;
	int 	nRow = 0, nColumn = 0;//行数，列数

	if(NULL == table_name)
	{
		printf("The empty_table_determine() argument incorrect!\n");
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	snprintf(sql,sizeof(sql) ,"select * from %s limit 1", table_name);//查询第一行数据

	rc = sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);//查询第一行数据保存在dbResult中
	if (rc != SQLITE_OK)
	{
		printf("Execute sqlite_table_exist:%s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -2;
	}

	return nRow;
}


int get_table_data(pack_info_t *pack_info,  char *table_name)
{
	char 	sql[100];
	int 	rc =-1;
	char 	*zErrMsg = 0;
	char 	**dbResult;
	int 	nRow = 0, nColumn = 0;

	if((NULL == pack_info)||(NULL == table_name))
	{
		printf("The select_first_data() argument incorrect!\n");
		return -1;
	}

	memset(sql, 0, sizeof(sql));
	snprintf(sql,sizeof(sql), "select * from %s limit 1", table_name);
	
	//把得到的第一行数据保存到dbResult 从dbResult[nRow*nColumn+1(第一列)-1]开始 
	rc = sqlite3_get_table(db, sql, &dbResult, &nRow, &nColumn, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("Execute sqlite_table_exist:%s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -2;
	}

	memset(pack_info, 0, sizeof(pack_info));
	strcpy(pack_info->sn, dbResult[3]);//把dbResult中保存的数据复制到pack_info
	strcpy(pack_info->datime, dbResult[4]);
	pack_info->temper = atof(dbResult[5]);
	printf("get_table_data,pack_info->sn:%s  pack_info->datime:%s  pack_info->temper:%02f\n", \
	pack_info->sn,pack_info->datime,pack_info->temper);

	return 0;
}


int delete_table_data(char *table_name)
{
	char 	sql[100];
	int 	rc =-1;
	char 	*zErrMsg = 0;

	if(NULL == table_name)
	{
		printf("The data_delete() argument incorrect!\n");
		return -1;
	}

	snprintf(sql,sizeof(sql), "DELETE from %s limit 1;", table_name);
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   	if ( rc != SQLITE_OK )
   	{
    	printf("Sqlite_delete error: %s\n", zErrMsg);
    	sqlite3_free(zErrMsg);
		return -2;
  	}

	printf("Delete first row data successfully!\n");
	return 0;
}

int close_sqlite_db(void)
{
	int		rc;
	rc=sqlite3_close(db);
	if(rc==SQLITE_BUSY)
	{
		printf("Query not complete, disable closing!\n");
	}
	
	return 0;
}
