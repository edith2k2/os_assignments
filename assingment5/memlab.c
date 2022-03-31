#include "memlab.h"
#include <stdio.h>
#include <string.h>

// read 0, write 1
char data_types[][20] = {"int", "char", "medimum int", "boolean"};
int data_sizes[] = {32, 8, 24, 1};
page_entry *page_table, *page_table_top;
stack_entry *stack, *stack_top;
word* actual_mem;
void createMem(int req_size)
{
  page_table = malloc(PAGE_TABLE);
  page_table_top = page_table;
  stack = malloc(STACK_TABLE);
  stack_top = stack;
  actual_mem = malloc(req_size);
  printf("Created %d memory\n", req_size);
}

void createVar(const char* fn_name, const char* var_name,const char* data_type)
{
  if (strcmp(data_type, "int") != 0 && strcmp(data_type, "char") != 0 && strcmp(data_type, "medium int") != 0 && strcmp(data_type, "boolean") != 0)
  {
    printf("Not a valid data type\n");
    return;
  }
  push_to_stack(fn_name, var_name, data_type, 1);
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

void push_to_stack(const char* fn_name, const char* var_name,const char* data_type, int size)
{
  strcpy(stack_top -> fn_name, fn_name);
  stack_top -> local_address = (stack_top - stack);
  stack_top -> size = size;
  strcpy(stack_top -> type, data_type);
  strcpy(stack_top -> var_name, var_name);
  printf("Pushed to stack\n");
  debug_print("stack is fn: %s, data type: %s, var name:%s, address:%d, top:%d", stack_top -> fn_name, stack_top -> type, stack_top -> var_name, stack_top -> local_address, stack_top -> size);

  // allocate_to_page(stack_top -> local_address, size);
  // stack_top++;
}

void allocate_to_page(unsigned int local_address, int size)
{
  if ((local_address % PAGE_SIZE != 0) && ((local_address + size) / PAGE_SIZE) == ((local_address) / PAGE_SIZE))
  {
    // can be allocated to exisiting page
    int page_no = local_address / PAGE_SIZE;
    int page_index = local_address % PAGE_SIZE;
    for (int i = 0; i < size; i++)
    {
      // access_mem(page_no, page_index + 1 + i, 1, 0);
    }
  }else
  {
    // should create new page
    int page_no = local_address / PAGE_SIZE + 1;
    int num_pages = ;
    
  }
}

void access_mem(int page_no, int page_index)
{

}
