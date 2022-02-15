#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;
#define SHM_SIZE 1000
typedef struct _process_data
{
  double **A;
  double **B;
  double **C;
  int veclen, i, j;
}ProcessData;

void* mult(void* arg);

int row, col;
int main()
{
  key_t key = 5679;
  int shmid;
  double* shm;
  // shmdt();
  if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0)
  {
    printf("error in allocating shared memory\n");
    exit(0);
  }
  if((shm = (double*)shmat(shmid, NULL, 0)) == (void*) -1)
  {
    printf("error in attaching shared memory\n");
    exit(0);
  }
  int r1, c1, r2, c2;
  cout << "Enter r1, c1, r2, c2\n";
  cout << "Enter r1 : "; cin >> r1 ;
  cout << "Enter c1 : "; cin >> c1;
  cout << "Enter r2 : "; cin >> r2;
  while(r2 != c1)
  {
    cout << "r2 should be equal to c1. Please re enter r2 : ";
    cin >> r2;
  }
  cout << "Enter c2 : "; cin >> c2;

  row = r1;
  col = c2;

  double *A;
  double *B;
  double *C;
  A = shm;
  B = shm + (r1 * c1);
  C = shm + (r1 * c1) + (r2 * c2);
  cout << "Enter array A : ";
  double db;
  double *temp = A;
  for (int i = 0; i < r1 * c1; i++)
  {
    cin >> db;
    *temp = db;
    temp++;
  }
  printf("\n");
  for (int i = 0; i < r1; i++)
  {
    for (int j = 0; j < c1; j++)
    {
      printf("%f ", *(A + r1 * i + j));
    }
    printf("\n");
  }
  cout << "\nEnter array B : ";
  temp = B;
  for (int i = 0; i < r2 * c2; i++)
  {
    cin >> db;
    *temp = db;
    temp++;
  }
  printf("\n");
  for (int i = 0; i < r2; i++)
  {
    for (int j = 0; j < c2; j++)
    {
      printf("%f ", *(B + r2 * i + j));
    }
    printf("\n");
  }

  pid_t child_pid;
  for (int i = 0; i < r1 ; i++)
  {
    for (int j = 0; j < c2; j++)
    {
      child_pid = fork();
      if (child_pid == 0)
      {
        ProcessData data;
        data.A = &A;
        data.B = &B;
        data.C = &C;
        data.veclen = r2;
        data.i = i;
        data.j = j;
        mult(&data);
        exit(0);
      }
    }
  }
  while((wait(NULL)) != -1);
  printf("\n");
  for (int i = 0; i < r1; i++)
  {
    for (int j = 0; j < c2; j++)
    {
      printf("%f ", *(C + r1 * i + j));
    }
    printf("\n");
  }
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
}
void* mult(void* argvoid)
{
  ProcessData* arg=(ProcessData*)argvoid;
  double *A, *B, *C;
  A = *(arg -> A);
  B = *(arg -> B);
  C = *(arg -> C);
  int i, j, veclen;
  i = arg -> i;
  j = arg -> j;
  veclen = arg -> veclen;
  *(C + row * i + j) = 0;
	for(int k = 0; k < veclen; k++)
  {
    *(C + row * i + j) += (*(A + row * i + k)) * (*(B + col * k + j));
	}
}
