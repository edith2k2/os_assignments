#include "memlab.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// read 0, write 1
// char data_types[][20] = {"int", "char", "medimum int", "boolean"};
// int data_sizes[] = {32, 8, 24, 1};
page_entry *page_table;
int page_table_size;
stack_entry *stack, *stack_top;
fn_stack_entry *fn_stack, *fn_stack_top;
word* actual_mem;
word* free_pages, *mark_pages;
int free_pages_size, mem_size, free_mem_size, mark_pages_size;

pthread_t garbage_collector;

void createMem(int req_size)
{
  mem_size = req_size;
  free_mem_size = req_size ;
  stack = malloc(STACK_TABLE);
  stack_top = stack;
  fn_stack = malloc(FN_STACK_TABLE);
  fn_stack_top = fn_stack;
  actual_mem = malloc(req_size);
  printf("%d", *(actual_mem + req_size + 1));
  free_pages_size = req_size / (32 * PAGE_SIZE) + 1;
  free_pages = malloc(free_pages_size);
  mark_pages_size= free_pages_size;
  mark_pages = malloc(mark_pages_size);
  page_table = malloc(PAGE_TABLE);
  for (int i = 0; i < free_pages_size; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      free_pages[i].byte[j] = 0;
    }
  }
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&garbage_collector, &attr, garbage_runner, NULL);
  printf("Created %d memory from %d\n", req_size, actual_mem);
}

void createVar(const char* fn_name, const char* var_name, const char* data_type)
{
  if (free_mem_size <= 4)
  {
    printf("Not sufficient space\n");
    return;
  }
  if (strcmp(data_type, "int") != 0 && strcmp(data_type, "char") != 0 && strcmp(data_type, "medium int") != 0 && strcmp(data_type, "boolean") != 0)
  {
    printf("Not a valid data type\n");
    return;
  }
  push_to_stack(fn_name, var_name, data_type, 1);
  free_mem_size = free_mem_size - 4;
}

void assignVar(const char* var_name, int element_no, int value)
{
  stack_entry* it = stack;
  int virtual_page_no ,physical_page ,physical_index;
  word* physical_location ;
  while (it != stack_top)
  {
    if (strcmp(var_name, it -> var_name) == 0)
    {
      virtual_page_no =  (it -> local_address + element_no) / PAGE_SIZE;
      physical_page = page_table[virtual_page_no].physical_page_no;
      physical_index = (it -> local_address + element_no) % PAGE_SIZE;
      physical_location = (actual_mem + PAGE_SIZE * physical_page + physical_index);
      debug_print("var_name: %s, virtual page: %d, physical_page: %d, physical_index: %d, physical_location: %d\n", it -> var_name, virtual_page_no, physical_page, physical_index, physical_location);
      access_mem(physical_location, 1, it -> type, value);
      access_mem(physical_location, 0, it -> type, value);
      break;
    }
    it++;
  }
}

void createArr(const char* fn_name, const char* var_name, const char* data_type, int size)
{
  if (free_mem_size <= 4 * size)
  {
    printf("Not sufficient space\n");
    return;
  }
  if (strcmp(data_type, "int") != 0 && strcmp(data_type, "char") != 0 && strcmp(data_type, "medium int") != 0 && strcmp(data_type, "boolean") != 0)
  {
    printf("Not a valid data type\n");
    return;
  }
  push_to_stack(fn_name, var_name, data_type, size);
  free_mem_size = free_mem_size - 4 * size;
}

void freeElem(const char* fn_name, const char* var_name)
{

  if (strcmp((fn_stack_top - 1) -> fn_name, fn_name) != 0)
  {
    printf("Fatal Error in fn_stack_top\n");
    return;
  }
  ((fn_stack_top - 1) -> no_variables) -= 1;
  debug_print("fn_stack is fn: %s, no_variables: %d \n", (fn_stack_top - 1) -> fn_name, (fn_stack_top - 1) -> no_variables);
  if ((fn_stack_top - 1) -> no_variables == 0)
  {
    while(stack_top != stack && strcmp(fn_name, (stack_top - 1) -> fn_name) == 0)
    {
      stack_top--;
    }
    debug_print("fn: %s popped \n", fn_name);
  }
}

void push_to_stack(const char* fn_name, const char* var_name,const char* data_type, int size)
{
  strcpy(stack_top -> fn_name, fn_name);
  if (stack == stack_top)
  {
    stack_top -> local_address = 0;
  }else
  {
    int prev_addr = (stack_top - 1) -> local_address  + (stack_top - 1) -> size;
    stack_top -> local_address = prev_addr;
  }
  stack_top -> size = size;
  if (strcmp(data_type, "int") == 0)
  {
    stack_top -> type = 0;
  }else if (strcmp(data_type, "char") == 0)
  {
    stack_top -> type = 1;
  }else if (strcmp(data_type, "medimum int") == 0)
  {
    stack_top -> type = 2;
  }else if (strcmp(data_type, "boolean") == 0)
  {
    stack_top -> type = 3;
  }
  strcpy(stack_top -> var_name, var_name);
  if (fn_stack == fn_stack_top)
  {
    strcpy(fn_stack_top -> fn_name, fn_name);
    (fn_stack_top -> no_variables) += 1;
    fn_stack_top++;
  }else
  {
    if (strcmp((fn_stack_top - 1) -> fn_name, fn_name) == 0)
    {
      ((fn_stack_top - 1) -> no_variables) += 1;
    }else
    {
      strcpy(fn_stack_top -> fn_name, fn_name);
      (fn_stack_top -> no_variables) += 1;
      fn_stack_top++;
    }
  }
  debug_print("Pushed to stack\n");
  debug_print("stack entry is fn: %s, data type: %d, var name:%s, address:%d, size:%d \n", stack_top -> fn_name, stack_top -> type, stack_top -> var_name, stack_top -> local_address, stack_top -> size);
  debug_print("fn_stack is fn: %s, no_variables: %d \n", (fn_stack_top - 1) -> fn_name, (fn_stack_top - 1) -> no_variables);
  allocate_to_page(stack_top -> local_address, size);
  stack_top++;
}

void allocate_to_page(unsigned int local_address, int size)
{
  if ((local_address % PAGE_SIZE != 0) && ((local_address + size) / PAGE_SIZE) == ((local_address) / PAGE_SIZE))
  {
    // can be allocated to exisiting page
    // int page_no = local_address / PAGE_SIZE;
    // int page_index = local_address % PAGE_SIZE;
    // for (int i = 0; i < size; i++)
    // {
    //   // access_mem(page_no, page_index + 1 + i, 1, 0);
    // }
  }else
  {
    // should create new page
    // int page_no = ;
    debug_print("should create new page, local address %d\n", local_address);
    int start_word = (local_address) % PAGE_SIZE;
    int num_pages = (local_address + size) / PAGE_SIZE + 1 - local_address / PAGE_SIZE;
    int page_no = (local_address) / PAGE_SIZE;
    int actual_pages;
    if (start_word == 0)
    {
      // should allocate num_pages pages
      actual_pages = num_pages;
    }else
    {
      // should allocate num_pages - 1 pages
      actual_pages = num_pages - 1;
      page_no++;
    }
    printf("numpages: %d %d acutal pages\n", num_pages, actual_pages);
    for (int i = 0; i < actual_pages; i++)
    {
      int set_flag = 1;
      for (int j = 0; j < free_pages_size; j++)
      {
        for (int byte_no = 0; byte_no < 4; byte_no++)
        {
          for (int k = 0; k < 8 ; k++)
          {
            if (32 * j + 8 * byte_no + k > mem_size * 8) break;
            if (!((free_pages[j].byte[byte_no] >> k) & 1))
            {
              free_pages[j].byte[byte_no] = free_pages[j].byte[byte_no] | (1 << k);
              debug_print("virtual page %d -> physical page %d\n", page_no + i, 32 * j + 8 * byte_no + k);
              insert_page_entry(page_no + i, 32 * j + 8 * byte_no + k);
              set_flag = 0;
              break;
            }
            if (!set_flag) break;
          }
          if (!set_flag) break;
        }
        if (!set_flag) break;
      }
    }
  }
}

void access_mem(word* physical_location, int type, int data_type, int value)
{
  if (type == 0)
  {
    int res = 0;
    if (data_type == 0)
    {
      res = res | ((physical_location -> byte)[0]);
      res = res | ((physical_location -> byte)[1] << 8);
      res = res | ((physical_location -> byte)[2] << 16);
      res = res | ((physical_location -> byte)[3] << 24);
      printf("int : %d at %d\n", res, physical_location);
    }else if (data_type == 1)
    {
      res = res | ((physical_location -> byte)[0]);
      printf("char : %c at %d\n", res, physical_location);
    }else if (data_type == 2)
    {
      res = res | ((physical_location -> byte)[0]);
      res = res | (8 << (physical_location -> byte)[1]);
      res = res | (16 << (physical_location -> byte)[2]);
      printf("Medimum int : %d at %d\n", res, physical_location);
    }else if (data_type == 3)
    {
      res = res | ((physical_location -> byte)[0]);
      printf("Boolean : %d at %d\n", res, physical_location );
    }
    return;
  }
  if (data_type == 0)
  {
    for (int i = 0; i < 4; i++)
    {
      physical_location -> byte[i] = 0;
    }
    physical_location -> byte[0] = (value >> 0) & (0xff);
    physical_location -> byte[1] = (value >> 8) & (0xff);
    physical_location -> byte[2] = (value >> 16) & (0xff);
    physical_location -> byte[3] = (value >> 24) & (0xff);
  }else if (data_type == 1)
  {
    if (value < -128 || value > 127)
    {
      printf("Invalid value to char\n");
      return;
    }
    for (int i = 0; i < 4; i++)
    {
      physical_location -> byte[i] = 0;
    }
    physical_location -> byte[0] = value;
  }else if (data_type == 2)
  {
    if (value < -8388608 || value > 8388607)
    {
      printf("Invalid value to medimum int\n");
      return;
    }
    for (int i = 0; i < 4; i++)
    {
      physical_location -> byte[i] = 0;
    }
    physical_location -> byte[0] = (value >> 0) & (0xff);
    physical_location -> byte[1] = (value >> 8) & (0xff);
    physical_location -> byte[2] = (value >> 16) & (0xff);
  }else if (data_type == 3)
  {
    if (value != 0 && value != 1)
    {
      printf("Invalid value to a boolean\n");
      return;
    }
    for (int i = 0; i < 4; i++)
    {
      physical_location -> byte[i] = 0;
    }
    physical_location -> byte[0] = (physical_location -> byte[0]) | (value);
  }
  debug_print("allocated memory at %d\n", physical_location);
  return;
}

void insert_page_entry(int virtual_page, int physical_page)
{
  page_table[virtual_page].physical_page_no = physical_page;
}

void* garbage_runner(void* arg)
{
  while(1)
  {
    usleep(1000);
    mark_sweep();
  }
}

void mark_sweep()
{
  mark();
}

void mark()
{
  int set_flag = 1;

  for (int j = 0; j < mark_pages_size; j++)
  {
    for (int byte_no = 0; byte_no < 4; byte_no++)
    {
      for (int k = 0; k < 8 ; k++)
      {
        if (32 * j + 8 * byte_no + k > mem_size * 8) break;
        if (!((free_pages[j].byte[byte_no] >> k) & 1))
        {
          free_pages[j].byte[byte_no] = free_pages[j].byte[byte_no] | (1 << k);
          set_flag = 0;
          break;
        }
        if (!set_flag) break;
      }
      if (!set_flag) break;
    }
    if (!set_flag) break;
  }
  stack_entry* it = stack;
  int virtual_page_no, physical_page;
  while(it != stack_top)
  {
    virtual_page_no =  (it -> local_address + element_no) / PAGE_SIZE;
    physical_page = page_table[virtual_page_no].physical_page_no;
    mark_pages[]
  }
}
