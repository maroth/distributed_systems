#include <stdio.h>

#include "helloworld.ph"
#define size 4

  int main(int argc,char *argv[])
  {
    int sumObj = 0;
    HelloWorld obj[size];
    for (int i=0; i<size; i++) obj[i].setID(i);
    for (int i=0; i<size; i++)sumObj = sumObj + obj[i].getID();
    printf("Sum of objects objects id = %d\n", sumObj);
    return 0;
  }
