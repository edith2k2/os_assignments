#include "memlab.h"
#include <stdio.h>
#include <stdlib.h>
int main()
{
  createMem(80);
  createVar("main", "a", "int");
  assignVar("a", 0, 3);
  // createVar("main", "b", "int");
  createArr("main", "ab", "int", 10);
  for (int i = 0; i < 10; i++)
  {
    assignVar("ab", i, i + 2);
  }
  freeElem("main", "a");
  freeElem("main", "ab");
}
