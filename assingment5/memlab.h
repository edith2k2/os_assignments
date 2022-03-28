#include <stdlib.h>

#define WORD 4 // bytes
#define NAME_SIZE 10 // bytes s
#define BOOK_KEEP 1000 // words

void* mem;
typedef struct
{
  char name[NAME_SIZE];
  long long offset;
}page_entry;
// typedef struct {
//
// }word;
// void word_converter();

void createMem(int req_size);

void createVar(const char*, const char* data_type);

void assignVar();

void createArr();

void freeElem();
