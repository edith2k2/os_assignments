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
#define SHM_SIZE 10000000
#define N 4
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
  int job_created;
  int len;
  int idx;
}ProcessData;

void create_job_rand(job*, int);
double** multiply_blocks(job&, job&);  // 2 jobs, status index refering block
int insert_blocks(double**, job&);
int delete_mat(job*, int front, int next);

int main()
{
  // srand(time(NULL));
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


  double* Mat[numMat];

  for(int i=0;i<numMat;i++){
    Mat[i]=*(shm->job_queue[i].mat);
    // create_job_rand(shm->job_queue+i);
  }

  // initialisation of ProcessData
  shm -> len = 0;
  shm -> idx = 0;
  shm -> job_created = 0;
  cout << sizeof(*shm) << '\n';
  // creating workers and producers
  int type_of_process = 2;     // 2-parent 1-producer 0-worker
  if(type_of_process==2){
  for (int i = 0; i < np; i++)
    {
      if (fork()==0)
      {
        // cout<<"# p : "<<i<<"--"<<getpid()<<"\n";
        time_t t;
        srand((int)time(&t) % getpid());
        type_of_process = 1;
        break;
      }
    }
  }
  if(type_of_process==2){
    for (int i = 0; i < nw; i++)
    {
      if (fork() == 0)
      {
        // cout<<"# w : "<<i<<"--\n";
        type_of_process = 0;
        break;
      }
    }
  }
  while(shm -> job_created < numMat)
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
        if (shm -> job_created == numMat) break;
      }
      if ((shm -> job_queue)[front].status == -1)
      {
        create_job_rand(shm -> job_queue + back, 1);
        back = (back + 1) % QUEUE_SIZE;
      }
      double** result_mult;
      result_mult = multiply_blocks((shm -> job_queue)[front], (shm -> job_queue)[next]);

      insert_blocks(result_mult,(shm->job_queue)[back]);
      (shm -> job_queue)[front].status++;
      (shm -> job_queue)[next].status++;
      if ((shm -> job_queue)[front].status == QUEUE_SIZE){
        delete_mat(shm->job_queue,front,next);
        shm->idx+=2;
      }
    }
    else if (type_of_process == 1)
    {
      int wait_time = rand() % 4;
      sleep(wait_time);

      // if (shm -> job_created == numMat){
      //   cout<<"All jobs created"<<endl;
      //   break;
      // }
      int back = (shm -> idx + shm -> len) % QUEUE_SIZE;
      create_job_rand(shm -> job_queue + back, 0);
      // cout<<shm->idx<<" "<<shm->len<<" "<<back<<"....\n";
      // for(int i=0;i<N;i++){
      //   for(int j=0;j<N;j++){
      //     cout<<shm->job_queue[back].mat[i][j]<<" ";
      //   }
      //   cout<<"\n";
      // }
      // cout<<"\n";
      (shm -> job_created)++;
      (shm -> len)++;
    }
  }
  if (type_of_process == 1 || type_of_process == 0) exit(0);

  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  return 0;
}
void create_job_rand(job* j, int empty)
{
  double min = -9, max = 9;

  for(int i = 0; i < N * N; i++)
  {
    j -> mat[i / N][i % N] = (empty == 0) ? min + (double)(rand()) / ((double)(RAND_MAX / (max - min))) : 0;
  }
  j -> prod_number = 0;
  j -> matrix_id = 1 + (double)(rand()/RAND_MAX) * (100000 - 1);
  j -> status = -1;
}

double** multiply_blocks(job& A, job& B){
 return nullptr;
}

int insert_blocks(double** mat, job& A){
  return 0;
}

int delete_mat(job* A, int front, int next){
  return 0;
}
