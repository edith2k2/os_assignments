#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <vector>
#include <random>
#include <time.h>

using namespace std;
#define SHM_SIZE 1000
#define N 10
#define NUM_JOBS 10
#define QUEUE_SIZE 8

vector<string> block {"0000", "0110", "0001", "0111", "1000", "1110", "1001", "1111"};
typedef struct
{
  double mat[N][N];
  int prod_number, matrix_id, status;
}job;
typedef struct
{
  job job_queue[QUEUE_SIZE];
  int job_created, len, idx;
}ProcessData;

void create_job_rand(job*, int);
double** multiply_blocks(job&, job&);
int insert_blocks(double**, job&);
int delete_mat(job*, int, int);

int main()
{
  key_t key = 5674;
  int shmid;
  ProcessData *shm;
  if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0)
  {
    printf("error in allocating shared memory\n");
    exit(0);
  }
  if((shm = (ProcessData *)shmat(shmid, NULL, 0)) == (void *)-1)
  {
    printf("error in attaching shared memory\n");
    exit(0);
  }
  int np, nw;
  printf("Enter number of producers: ");
  cin >> np;
  printf("Enter number of workers: ");
  cin >> nw;
  // initialisation of ProcessData
  shm -> len = 0;
  shm -> idx = 0;
  shm -> job_created = 0;
  cout << shm -> len;
  // creating workers and producers
  int type_of_process = 2;
  srand(time(0));
  for (int i = 0; i < np; i++)
  {
    if (type_of_process == 2 && fork())
    {
      type_of_process = 1;
    }
  }
  for (int i = 0; i < nw; i++)
  {
    if (type_of_process == 2 && fork())
    {
      type_of_process = 0;
    }
  }
  while(shm -> job_created != NUM_JOBS)
  {
    if (type_of_process == 0)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);
      int front = (shm -> idx ) % QUEUE_SIZE;
      int next = ((shm -> idx) + 1) % QUEUE_SIZE;
      int back = (shm -> idx + shm -> len) % QUEUE_SIZE;
      while (shm -> len < 2 || (shm -> job_queue)[front].status >= QUEUE_SIZE)
      {
        if (shm -> job_created == NUM_JOBS) break;
      }
      if ((shm -> job_queue)[front].status == -1)
      {
        create_job_rand(shm -> job_queue + back, 1);
        back = (back + 1) % QUEUE_SIZE;
      }
      double** result_mult;
      (shm -> job_queue)[front].status++;
      (shm -> job_queue)[next].status++;
      result_mult = multiply_blocks((shm -> job_queue)[front], (shm -> job_queue)[next]);
      insert_blocks(result_mult, (shm -> job_queue)[back]);
      if ((shm -> job_queue)[front].status == QUEUE_SIZE)
      {
        // delete_mat();
      }
    }else if (type_of_process == 1)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);
      while (shm -> len == QUEUE_SIZE)
      {
        if (shm -> job_created == NUM_JOBS) break;
      }
      int insert_idx = (shm -> idx + shm -> len) % QUEUE_SIZE;
      create_job_rand(shm -> job_queue + insert_idx, 0);
      (shm -> job_created)++;
      (shm -> len)++;
    }
  }
  if (type_of_process == 1 || type_of_process == 0) exit(0);

  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
}
void create_job_rand(job* j, int empty)
{
  double min = -9, max = 9;

  for(int i = 0; i < N * N; i++)
  {
    j -> mat[i / N][i % N] = (empty == 0) ? min + (double)(rand()) / ((double)(RAND_MAX / (max - min))) : 0;
  }
  j -> prod_number = 0;
  j -> matrix_id = 1 + (rand() / RAND_MAX) * (100000 - 1);
  j -> status = 0;
}
double** multiply_blocks(job& A, job& B)
{
  int i, j, k;
  int status = A.status;
  int A_i, A_j, B_i, B_j;
  A_i = block[status][0] - '0';
  A_j = block[status][1] - '0';
  B_i = block[status][2] - '0';
  B_j = block[status][3] - '0';
  double** result_mult = new double* [N / 2];
  for (int i = 0; i < N / 2; i++)
  {
    result_mult[i] = new double [N / 2];
  }
  for (int i = 0; i < N / 2; i++)
  {
    for (int j = 0; j < N / 2; j++)
    {
      result_mult[i][j] = 0;
      for (int k = 0; k < N / 2; k++)
      {
        result_mult[i][j] += (A.mat[A_i * (N / 2) + i][A_j * (N / 2) + k]) * (B.mat[B_i * (N / 2) + k][B_j * (N / 2) + j]);
      }
    }
  }
  return result_mult;
}
int insert_blocks(double** result_mult, job& job)
{
  return 1;
}
int delete_mat(job* job_queue, int front, int next)
{
  return 1;
}
