#include <stdlib.h>

#define WORD 4 // bytes
#define NAME_SIZE 10 // bytes s
#define PAGE_TABLE 100 // page_entry
#define STACK_TABLE 1000 // stack_entry
#define FN_STACK_TABLE 100
#define PAGE_SIZE 10000 // words
// #define PAGE_SIZE 5

#define DEBUG 1
#define debug_print(fmt, ...) \
  do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
    __LINE__, __func__, ##__VA_ARGS__); } while (0)
// extern char data_types[][20] = {"int", "char", "medimum int", "boolean"};
// extern int data_sizes[] = {32, 8, 24, 1};
// 0 :int, 1: char, 2: medimum int, 3: boolean
typedef struct
{
  char fn_name[NAME_SIZE];
  char var_name[NAME_SIZE];
  int size, type;
  unsigned int local_address;
}stack_entry;
typedef struct
{
  unsigned int physical_page_no;
}page_entry;
typedef struct
{
  char fn_name[NAME_SIZE];
  int no_variables;
}fn_stack_entry;
typedef struct
{
  
}mark;
typedef struct
{
  unsigned char byte[WORD];
}word;


// extern page_entry *page_table, *page_table_top;
// extern stack_entry *stack, *stack_top;
// extern word* actual_mem;
// void word_converter();

void createMem(int req_size);

void createVar(const char*, const char*, const char* data_type);

void assignVar();

void createArr(const char*, const char*, const char*, int);

void freeElem();

void push_to_stack(const char*, const char*, const char*, int);

void allocate_to_page();

void access_mem(word*, int, int ,int);

void insert_page_entry(int, int);

void* garbage_runner(void*);

void mark_sweep();
