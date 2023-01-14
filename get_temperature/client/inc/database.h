/********************************************************************************
 *      Copyright:  (C) 2023 Yangpeng
 *                  All rights reserved.
 *
 *       Filename:  database.h
 *    Description:  This database.h file 
 *
 *        Version:  1.0.0(2023年01月07日)
 *         Author:  Yangpeng <1023769078@qq.com>
 *      ChangeLog:  1, Release initial version on "2023年01月07日 23时03分07秒"
 *                 
 ********************************************************************************/


#ifndef  _DATABASE_H_
#define  _DATABASE_H_




//创建数据库和表
int create_database_table(void);

//向数据库表中插入数据
int data_insert(pack_info_t pack_info, char *table_name);

//检查表中是否有数据
int check_table_data(char *table_name);

//获取表第一行数据
int get_table_data(pack_info_t *pack_info,  char *table_name);

//删除表中第一行数据
int delete_table_data(char *table_name);

//关闭数据库
int close_sqlite_db(void);


#endif
