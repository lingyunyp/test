#include <stdio.h>
char lower2upper(char ch)
{
  return ch+('A'-'a');
}

int main(int argc,char **argv)
{
  char   ch=0x61;
  printf("ch value: %%c[%c] %%d[%d]  %%02x[0x%02x]\n",ch,ch,ch);

  return 0;
}
