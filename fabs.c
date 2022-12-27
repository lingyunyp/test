#include <stdio.h>
#include <math.h>
int main(int argc,char *argv[])
{
  float a=3.1415;
  float b=3.1415926;
  if(fabs(a-b) < 0.00001)
  {
   printf("equal\n");

  }
  else
  {
    printf("not equal\n");
  }
  return 0;

}
