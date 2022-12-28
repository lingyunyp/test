#include<stdio.h>
#include<string.h>
#include<sys/types.h>         
#include<sys/socket.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<getopt.h>
#include<pthread.h>
#include<ctype.h>

#define BACKLOG     13

typedef  void *(THREAD_BODY)  (void *thread_arg);

void *thread_worker(void *args);
int  thread_start(pthread_t * thread_id,THREAD_BODY * thread_workbody, void *thread_arg);

void print_usage(char*progname)
{  
     printf("%s usage:\n",progname);
     printf("-p(--server_port): sepcify server port\n");
     printf("-h(--help): print this help information\n");

     return;
}

int main(int argc,char *argv[])
{
     int                  listen_fd,client_fd=-1;
     int                  rv=-1;
     struct sockaddr_in   serv_addr;
     struct sockaddr_in   cli_addr;
     socklen_t            cliaddr_len=0;
     int                  on=1;
     int                  listen_port=0;
     int                  ch;
     pthread_t            tid;
     

     struct option       opts[]={
                       {"server_port",required_argument,NULL,'p'},
                       {"help",no_argument,NULL,'h'},
                       {NULL,0,NULL,0}         
                 };
    while((ch=getopt_long(argc,argv,"p:h",opts,NULL))!=-1)
    {
              switch(ch)
              { 
                   case 'p':
                   listen_port=atoi(optarg);
                   break;
                   case 'h':
                   print_usage(argv[0]);
                   return 0;
              }
    }

    if(!listen_port)
    {
          print_usage(argv[0]);
          return 0;
    }

   listen_fd=socket(AF_INET,SOCK_STREAM,0);
   if(listen_fd<0)
   {
          printf("open socket failure:%s\n",strerror(errno));
          return -1;
   }
   printf("open socket :fd[%d]\n",listen_fd);
  
   setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
   memset(&serv_addr,0,sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   serv_addr.sin_port=htons(listen_port);
   serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
 

   if(bind(listen_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
   {
          printf("create socket bind failure:%s\n",strerror(errno));
          return -2;
   }
   printf("socket[%d] bind on port[%d] for all IP address ok\n",listen_fd,listen_port);

   listen(listen_fd,BACKLOG);

   while(1)
   {
         printf("start waiting and accept new client connect...\n");
         client_fd=accept(listen_fd,(struct sockaddr *)&cli_addr,&cliaddr_len);
         if(client_fd<0)
         {
              printf("accept new socket failure:%s\n",strerror(errno));
              return -2;
         }
         printf("accept new client[%s:%d] with fd[%d]\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port),client_fd);
  

         thread_start(&tid,thread_worker,(void *)client_fd);
   } 
 
   close(listen_fd);
 
   return 0;     
}       
              


thread_start(pthread_t * thread_id,THREAD_BODY * thread_workbody, void *thread_arg)
{
     int             rv=-1;
     pthread_attr_t  thread_attr;
     
     if(pthread_attr_init(&thread_attr))
     {
          printf("pthread_attr_init() failure: %s\n",strerror(errno));
          goto cleanup;   
     }
    
     if(pthread_attr_setstacksize(&thread_attr,120*1024))
     {
          printf("pthread_attr_setstacksize() failure:%s\n",strerror(errno));
          goto cleanup;  
     }

     if( pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) )
     {
          printf("pthread_attr_setdetachstate() failure: %s\n", strerror(errno));
          goto cleanup;
     }
 
     if( pthread_create(thread_id, &thread_attr, thread_workbody, thread_arg) )
     {
          printf("Create thread failure: %s\n", strerror(errno));
          goto cleanup; 
     }
  
     rv = 0;

cleanup:
       pthread_attr_destroy(&thread_attr);
       return rv;
}

void *thread_worker(void *ctx)
{
     int   client_fd;
     int   rv;
     char  buf[1024];
     int   i;
     
     if(!ctx)
     {
           printf("invalid input arguments in %s()\n",__FUNCTION__);
           pthread_exit(NULL);
     } 

     client_fd=(int)ctx;
     printf("Child thread start to commuicate with socket client...\n");
        
     while(1)
     {
	   memset(buf,0,sizeof(buf));
	   if((rv=read(client_fd,buf,sizeof(buf)))<0)
	   {
		 printf("read data failure:%s\n",strerror(errno));
		 close(client_fd);
		 pthread_exit(NULL);
	   }             
	   else if(rv==0)
	   {
		 printf("client socket[%d] disconnect\n",client_fd);
		 close(client_fd);
		 pthread_exit(NULL);
	   }
	   printf("read data from client[%d] and echo it back:'%s'\n",client_fd,buf);
           
           for(i=0; i<rv; i++)
           {
                 buf[i]=toupper(buf[i]);
           }

	   if(write(client_fd,buf,rv)<0)
	   {
		 printf("write data failure；%s",strerror(errno));
		 close(client_fd);
		 pthread_exit(NULL);
	   }
	   
	   
     }
              
}   
   
 
  

