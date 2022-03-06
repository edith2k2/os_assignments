
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
#include <semaphore.h>

using namespace std;
#define SHM_SIZE 1000
#define N 10
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
int insert_blocks(double**, job&, int);
int delete_mat(job*, int, int);
void print_info(int, int, int, int, int, int, int, int, string );

int main()
{
  key_t key = IPC_PRIVATE;
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
  int np, nw, numMat;
  printf("Enter number of producers: ");
  cin >> np;
  printf("Enter number of workers: ");
  cin >> nw;
  cout<<"\nNumber of Matrices to multiply : ";
  cin>>numMat;
  // initialisation of ProcessData
  shm -> len = 0;
  shm -> idx = 0;
  shm -> job_created = 0;
  // creating workers and producers
  int type_of_process = 2;
  // printf("%d process created with type_of_process %d\n", getpid(), 2);
  for (int i = 0; i < np; i++)
  {
    if (type_of_process == 2 && fork() == 0)
    {
      // printf("%d process created with type_of_process %d\n", getpid(), 1);
      time_t t;
      srand((int)time(&t) % getpid());
      type_of_process = 1;
    }
  }
  for (int i = 0; i < nw; i++)
  {
    if (type_of_process == 2 && fork() == 0)
    {
      // printf("%d process created with type_of_process %d\n", getpid(), 0);
      time_t t;
      srand((int)time(&t) % getpid());
      type_of_process = 0;
    }
  }
  if (type_of_process == 0)
  {
    while(1)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);
      int front = (shm -> idx ) % QUEUE_SIZE;
      int next = ((shm -> idx) + 1) % QUEUE_SIZE;
      int back = (shm -> idx + shm -> len) % QUEUE_SIZE;
      while (shm -> len < 2 || (shm -> job_queue)[front].status >= 8)
      {
        if ((shm -> job_created) == numMat) break;
      }
      if ((shm -> job_created) == numMat && shm -> len < 2) break;
      if ((shm -> job_queue)[front].status == -1)
      {
        create_job_rand(shm -> job_queue + back, 1);
        back = (back + 1) % QUEUE_SIZE;
      }
      double** result_mult;
      (shm -> job_queue)[front].status++;
      (shm -> job_queue)[next].status++;
      if ((shm -> job_queue)[front].status == 8)
      {
        print_info(
          getpid(),
          (shm -> job_queue)[front].prod_number,
          (shm -> job_queue)[next].prod_number,
          (shm -> job_queue)[front].matrix_id,
          (shm -> job_queue)[next].matrix_id,
          block[(shm -> job_queue)[next].status][0] - '0',
          block[(shm -> job_queue)[next].status][1] - '0',
          block[(shm -> job_queue)[next].status][3] - '0',
          "DELETE"
        );
        delete_mat(shm -> job_queue, front, next);
        shm -> idx = (shm ->idx + 2) % QUEUE_SIZE;
        shm -> len -= 2;
      }else
      {
        // printf("%d entered\n", getpid());
        // printf("%d\n", (shm -> job_queue)[front].status);
        print_info(
          getpid(),
          (shm -> job_queue)[front].prod_number,
          (shm -> job_queue)[next].prod_number,
          (shm -> job_queue)[front].matrix_id,
          (shm -> job_queue)[next].matrix_id,
          block[(shm -> job_queue)[next].status][0] - '0',
          block[(shm -> job_queue)[next].status][1] - '0',
          block[(shm -> job_queue)[next].status][3] - '0',
          "MULTIPLY"
        );
        printf("%d\n", (shm -> job_queue)[front].status);
        result_mult = multiply_blocks((shm -> job_queue)[front], (shm -> job_queue)[next]);
        // printf("\n");
        // for (int i = 0; i < N; i++)
        // {
        //   for (int j = 0; j < N; j++)
        //   {
        //     printf("%6lf ", result_mult[i][j]);
        //   }
        //   cout << "\n";
        // }
        // printf("\n");
        print_info(
          getpid(),
          (shm -> job_queue)[front].prod_number,
          (shm -> job_queue)[next].prod_number,
          (shm -> job_queue)[front].matrix_id,
          (shm -> job_queue)[next].matrix_id,
          block[(shm -> job_queue)[next].status][0] - '0',
          block[(shm -> job_queue)[next].status][1] - '0',
          block[(shm -> job_queue)[next].status][3] - '0',
          "INSERT"
        );
        insert_blocks(result_mult, (shm -> job_queue)[(back - 1 + QUEUE_SIZE) % QUEUE_SIZE], (shm -> job_queue)[front].status);
      }
    }
  }else if (type_of_process == 1)
  {
    while(1)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);
      if (shm -> job_created == numMat) break;
      while (shm -> len == QUEUE_SIZE)
      {
        if (shm -> job_created == numMat) break;
      }
      int back = (shm -> idx + shm -> len) % QUEUE_SIZE;
      printf("-- Creation %d\n", getpid());
      (shm -> job_created)++;
      (shm -> len)++;
      create_job_rand(shm -> job_queue + back, 0);
    }
  }
  // printf("%d Process completed of type %d \n", getpid(), type_of_process);
  if (type_of_process != 2) exit(0);
  while((wait(NULL)) != -1);
  // printf("%d Process completed of type %d \n", getpid(), type_of_process);
  // cout << shm -> len << '\n';
  int back = (shm -> idx + shm -> len) % QUEUE_SIZE;
  printf("the sum of diagonal elements is :");
  double res = 0;
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      printf("%6lf ", ((shm -> job_queue)[back - 1]).mat[i][j]);
    }
    cout << "\n";
    res = res + (((shm -> job_queue)[back - 1]).mat[i][i]);
  }
  printf("%lf\n", res);
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
  j -> prod_number = getpid();
  j -> matrix_id = 1 + ((double)rand() /RAND_MAX) * (100000 - 1);
  // cout << j -> matrix_id << "\n\n";
  j -> status = -1;
}
double** multiply_blocks(job& A, job& B)
{
  int i, j, k;
  int status = A.status;
  printf("%d\n", status);
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
int insert_blocks(double** result_mult, job& job, int status)
{
  // int status = job.status;
  printf("%d\n", status);
  status /= 2;
  for(int i = 0; i < N / 2; i++)
  {
    for(int j = 0; j < N / 2; j++)
    {
      job.mat[i + (status / 2) * (N / 2)][j + (status % 2) * (N / 2)] += result_mult[i][j];
    }
  }
  return 1;
}
int delete_mat(job* job_queue, int front, int next)
{
  for(int i = 0; i < N * N; i++)
  {
    job_queue[front].mat[i / N][i % N] =  -1;
    job_queue[front].prod_number = -1;
    job_queue[front].matrix_id = -1;
    job_queue[front].status = -1;

    job_queue[next].mat[i / N][i % N] =  -1;
    job_queue[next].prod_number = -1;
    job_queue[next].matrix_id = -1;
    job_queue[next].status = -1;
  }
  return 1;
}
void print_info(int w_pid, int p_pid1, int p_pid2, int m_id1, int m_id2, int i, int j, int k, string work )
{
  printf("\n---  Worker Pid : %d   ---\n", w_pid);
  printf("\n---  Producer Number : %d %d   ---\n", p_pid1, p_pid2);
  printf("\n---  Matrix IDs : %d %d   ---\n", m_id1, m_id2);
  printf("\n---  block Numbers I J K: %d %d %d   ---\n", i, j, k);
  printf("\n---  Work Done : %s   ---\n\n", work.c_str());
}
