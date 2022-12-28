#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


#define  SOCKET_PATH   "/tmp/socket.domain"

int main(int argc,char *argv[])
{
      struct sockaddr_un    serv_addr;
      struct sockaddr_un    cli_addr; 
      int                   listen_fd,client_fd=-1;
      socklen_t             cliaddr_len;
      char                  buf[1024];
      int                   rv=-1;

      listen_fd=socket(AF_UNIX,SOCK_STREAM,0);
      if(listen_fd < 0 )
      {
           printf("create socket failure: %s\n", strerror(errno));
           return -1;
      }
      printf("socket create fd[%d]\n", listen_fd);
 
      if( !access(SOCKET_PATH, F_OK) )
      {
           unlink(SOCKET_PATH);
      }

      memset(&serv_addr, 0, sizeof(serv_addr));
      serv_addr.sun_family=AF_UNIX;
      strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path)-1);
      if( bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
      {
           printf("create socket failure: %s\n", strerror(errno));
           unlink(SOCKET_PATH);
           return -2;
      }
      printf("socket[%d] bind on path \"%s\" ok\n", listen_fd, SOCKET_PATH);
      listen(listen_fd, 13);

      while(1)
      {
           printf("\nStart waiting and accept new client connect...\n");
           client_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &cliaddr_len);
           if(client_fd < 0)
           {
                printf("accept new socket failure: %s\n", strerror(errno));
                return -2;
           }
           memset(buf, 0, sizeof(buf));
           if( (rv=read(client_fd, buf, sizeof(buf))) < 0)
           {
                printf("Read data from client socket[%d] failure: %s\n", client_fd, strerror(errno));
                close(client_fd);
                continue;
           }
           else if( rv == 0 )
           {
                printf("client socket[%d] disconnected\n", client_fd);
                close(client_fd);
                continue;
           }
           printf("read %d bytes data from client[%d] and echo it back: '%s'\n", rv, client_fd, buf);
         
           if( write(client_fd, buf, rv) < 0 )
           {
                printf("Write %d bytes data back to client[%d] failure: %s\n", rv, client_fd,strerror(errno));
           close(client_fd);
           }
 
           sleep(1);
           close(client_fd);
      }

      close(listen_fd);
}


