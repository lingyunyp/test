#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<dirent.h>

#define test_dir  "dir99"

int main(int argc,char **argv)
{

  int           rv;
  int           fd1;
  int           fd2;
  DIR           *dirp;
  struct dirent *direntp; 


  if(mkdir(test_dir, 0755)<0)
  {
    printf("creat '%s' failure :%s\n",test_dir,strerror(errno));
    return -1;
  }


  if(chdir(test_dir)<0)
  {
    printf("change dir to '%s' failure:%s\n",test_dir,strerror(errno));
    rv=-2;
    goto clearup;
  }


  if((fd1=creat("file11.txt",0644))<0)
  {
    printf("creat file1.txt failure:%s\n",strerror(errno));
    rv=-3;
    goto clearup;
  }
  if((fd2=creat("file22.txt",0644))<0)
  {
    printf("creat file1.txt failure:%s\n",strerror(errno));
    rv=-4;
    goto clearup;
  }



  if(chdir("../")<0)
  {
  
     printf("change dir '%s' failure:%s\n ",test_dir,strerror(errno));
     rv=-5;
     goto clearup;
  }



  if((dirp=opendir(test_dir))==NULL)
  {
    printf("open dir '%s' failure:%s\n",test_dir,strerror(errno));
    rv=-6;
    goto clearup;
  }


  while((direntp=readdir(dirp))!=NULL)
  {
   printf("%s\n",direntp->d_name);

  }
  closedir(dirp);

 clearup:
  if(fd1>=0)
  {
   close(fd1);
  }
    
  if(fd2>=0)
  {
    close(fd2);
  }
 
  
}	
