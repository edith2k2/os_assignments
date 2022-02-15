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
  int mat[N][N];
  int prod_number, matrix_id, status;
}job;
typedef struct
{
  job job_queue[QUEUE_SIZE];
  int job_created, len, idx;
}ProcessData;

void create_job_rand(job*, int);

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
  cout << sizeof(*shm) << '\n';
  // initialisation of ProcessData
  shm -> len = 0;
  shm -> idx = 0;
  shm -> job_created = 0;
  cout << shm -> len;
  // creating workers and producers
  bool type_of_process = 2;
  srand(time(0));
  for (int i = 0; i < np; i++)
  {
    if (fork() == 0)
    {
      type_of_process = 1;
    }
  }
  for (int i = 0; i < nw; i++)
  {
    if (fork() == 0)
    {
      type_of_process = 0;
    }
  }
  while(shm -> job_created != 10)
  {
    if (type_of_process == 0)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);
      int front = (shm -> idx ) % QUEUE_SIZE;
      int next = ((shm -> idx) + 1) % QUEUE_SIZE;
      while (shm -> len < 2 || (shm -> job_queue)[front].status >= 8)
      {
        if (shm -> job_created == 10) break;
      }
      if ((shm -> job_queue)[front].status == -1)
      {
        int insert_idx = (shm -> idx + shm -> len) % QUEUE_SIZE;
        create_job_rand(shm -> job_queue + insert_idx, 1);
      }
      multiply_blocks();
      insert_blocks();
      (shm -> job_queue)[front].status++;
      (shm -> job_queue)[next].status++;
      if ((shm -> job_queue)[front].status == 8) delete_mat();
    }else if (type_of_process == 1)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);
      while (shm -> len == QUEUE_SIZE)
      {
        if (shm -> job_created == 10) break;
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
  j -> status = -1;
}
