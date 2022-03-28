#include "memlab.h"
#include <stdio.h>
#include <string.h>

void createMem(int req_size)
{
  mem = malloc(req_size);
  printf("Created %d memory\n", req_size);
  
}

void createVar(const char* var_name,const char* data_type)
{
  if (strcmp(data_type, "int") == 0)
  {

  }else if (strcmp(data_type, "char"))
  {

  }else if (strcmp(data_type, "medium int"))
  {

  }else if (strcmp(data_type, "boolean"))
  {

  }else {
    printf("Invalid Datatype\n");
  }
}

void assignVar()
{

}

void createArr()
{

}

void freeElem()
{

}
