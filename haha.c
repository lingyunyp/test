#include <stdio.h>
#include<string.h>
#include<stdlib.h>
int main()
{
  FILE *fp1=NULL;
  FILE *fp2=NULL;
 fp1=fopen("test.txt","r+");
 fp2=fopen("test.log","w+");
 if(fp1==NULL)
 {
  printf("open file test.txt failed\n");
 }
 else
 {
	 
   printf("open file test.txt succeed\n");
   char buf[40]="dsao\n2323\nass\nwwwq\ndsdsa\nhello\nworld\n";
   fputs(buf,fp1);
 

  
 }
 char buf[40];
  while(!feof(fp1))
     {
        
       fgets(buf,sizeof(buf),fp1);	
          

          printf("%s\n",buf);
	  fputs(buf,fp2);
     }


  char buf1[6]="hello";
  char *ret=strstr(buf,buf1);
  if(ret == NULL)
  {
   printf("不存在\n");
  }
  else
  {
  printf("%s\n",ret);
  }

  while(!feof(fp2))
   {	
	fgets(buf,sizeof(buf),fp2);

	printf("%s\n",buf);
   }

fclose(fp1);
fclose(fp2);
 return 0;
}
