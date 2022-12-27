#include <stdio.h>

int main( int argc,char *argv[])
{
  unsigned char   byte;
  unsigned short  half_word;
  unsigned int    word;

  byte=255+2;
  half_word=4660;
  word=0x12345678;
  printf("byte:%d\n",byte);
  printf("half_word:%X\n",half_word);
  printf("word:%d\n",word);
  return 0;
}
