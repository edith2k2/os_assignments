#include "memlab.h"
#include <stdio.h>
int main()
{
  createMem(12);
  createVar("abc" ,"int");
  // void* mem1 = malloc(12);
  // printf("%d %d\n", mem1, mem1 + 10);
  // *(int*)(mem1 + 8) = 12;
  // int temp = *(int*)(mem1 + 8);
  // long temp1 = temp;
  // printf("%d", temp);
}
