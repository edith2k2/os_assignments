#include <pthread.h>
#include <sys/shm.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <iostream>
#include <queue>

// shmsize calculation:
// each process thread sleeps for 200-500ms and runs for 10-20s.
// the case for more memory created is each process sleeps less and runs more.
// which leads to sleep time = 200ms and run for 20s
// so each process thread creates 100 jobs in worst case (20s / 200ms)
// then we need to include the size of base tree
// shared memory size = (100 * p + 500) * job_size
#define Y 10
#define CHILDREN_SIZE 50

using namespace std;
vector<string> status_string({"created", "on going", "done"});

struct NODE
{
  int job_id, status, time;
  pthread_mutex_t lock;
  struct NODE *child;
  struct NODE *next;
};
typedef struct NODE NODE;

void create_base_tree(NODE*, int);
void* producer_runner(void*);
void* consumer_runner(void*);
void create_rand_tree(int, vector<vector<int>>&);
void print_status(pthread_t, int, int, int);
void print_thread(pthread_t);
void print_tree(NODE* );
void initialise_job(NODE&);
void create_job(NODE&);
void insert_job(NODE*, int);

int no_jobs = 0;
pthread_mutex_t job_lock;
pthread_mutex_t job_add;
int main()
{
  int P;
  printf("enter P: ");
  scanf("%d", &P);
  key_t key = 777;
  int shm_size ;
  shm_size = 100 * P + 500;
  // shm_size = 500;
  int shmid = shmget(key, sizeof(NODE) * (shm_size), IPC_CREAT | 0666);
  if (shmid < 0)
  {
    perror("Error in shmget:\n");
    exit(0);
  }
  NODE* base= (NODE*)shmat(shmid, NULL, 0);
  if (base == NULL)
  {
    perror("Error in shmat:\n");
    exit(0);
  }
  create_base_tree(base, P);
  print_tree(base);
  pthread_t* pro_id_arr = new pthread_t [P];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_mutex_init(&job_lock, NULL);
  pthread_mutex_init(&job_add, NULL);
  for (int i = 0; i < P; i++)
  {
    pthread_create(&(pro_id_arr[i]), &attr, producer_runner, base);
    // cout << "--> producer "; print_thread(pro_id_arr[i]); cout << " created\n";
    fflush(stdout);
  }
  // for (int i = 0; i < P; i++)
  // {
  //   pthread_join(pro_id_arr[i], NULL);
  // }
  // print_tree(base);
  // pid_t child_pid = fork();
  // if (child_pid == 0)
  // {
  //   int y = Y * P;
  //   pthread_t* con_id_arr = new pthread_t [y];
  //   for (int i = 0; i < y; i++)
  //   {
  //     pthread_create(&(con_id_arr[i]), &attr, consumer_runner, base);
  //     cout << "--> consumer "; print_thread(con_id_arr[i]); cout << " created\n";
  //     fflush(stdout);
  //   }
  //   for (int i = 0; i < y; i++)
  //   {
  //     pthread_join(con_id_arr[i], NULL);
  //   }
  // }else
  // {
  //   for (int i = 0; i < P; i++)
  //   {
  //     pthread_join(pro_id_arr[i], NULL);
  //   }
  // }
  shmdt(base);
  shmctl(shmid, IPC_RMID, NULL);
}
void create_base_tree(NODE* base, int P)
{
  //initialising
  for (int i = 0; i < 100 * P + 500; i++)
  {
    // if (base == NULL) cout << "Hello\n";
    // cout << base << '\n';
    initialise_job(base[i]);
  }
    // adding jobs
    int job_target = rand() % 200 + 300;
    // int job_target = 30;
    create_job(base[0]);
    for (int i = 1; i < job_target; i++)
    {
      insert_job(base, i);
    }
}
void initialise_job(NODE& job)
{
  job.job_id = -1;
  job.status = -1;
  job.time = -1;
  job.child = NULL;
  job.next = NULL;
}
void insert_job(NODE* base, int i)
{
  int target = rand() % i;
  int option = rand() % 2; // 0 for child 1 for sibling
  if (target == 0) option = 0;
  create_job(base[i]);
  NODE* it = &base[target];
  if (option == 0)
  {
    it = base[target].child;
    if (it == NULL)
    {
      base[target].child = &base[i];
      return;
    }
  }
  while(it -> next != NULL)
  {
    it = it -> next;
  }
  it -> next = &base[i];
}
void create_job(NODE& job)
{
  job.job_id = rand() % (int)pow(10, 8);
  job.status = 0;
  job.time = rand() % 251;
  job.child = NULL;
  job.next = NULL;
  pthread_mutex_init(&job.lock, NULL);
  print_status(pthread_self(), job.job_id, 0, job.time);
}
void* producer_runner(void* arg)
{
  NODE* base = (NODE*)arg;
  int run_time, sleep_time;
  run_time = (rand() % 10) + 10;
  sleep_time = (rand() % 300) + 200;
  time_t start_time = time(NULL);
  while(time(NULL) - start_time < run_time)
  {
    // add job
    pthread_mutex_lock(&job_lock); // get permission for no_jobs
    int idx = no_jobs;
    no_jobs++;
    pthread_mutex_unlock(&job_lock);

    pthread_mutex_lock(&job_add);
    insert_job(base, idx);
    pthread_mutex_unlock(&job_add);

    // pthread_mutex_lock(&(base[idx].lock));
    // print_status(pthread_self(), base[idx].job_id, 0, base[idx].time);
    // pthread_mutex_unlock(&(base[idx].lock));
    int err = pthread_mutex_trylock(&base[idx].lock);
    if (err == EBUSY) continue;
    else if (err < 0)
    {
      perror("Error in try lock\n");
      exit(0);
    }else print_status(pthread_self(), base[idx].job_id, 0, base[idx].time);
    pthread_mutex_unlock(&(base[idx].lock));

    usleep(sleep_time * 1000);
  }
  return NULL;
}
void* consumer_runner(void* arg)
{
  NODE* base = (NODE*)arg;
  bool available = true;
  pthread_mutex_lock(&job_lock);
  int current_jobs = no_jobs;
  pthread_mutex_unlock(&job_lock);
  printf("Current jobs are %d\n", current_jobs);
  while(available)
  {
    available = false;
    // check and sleep and delete
    for (int i = 0; i < current_jobs; i++)
    {
      int err = pthread_mutex_trylock(&base[i].lock);
      if (err == EBUSY) continue;
      else if (err < 0)
      {
        perror("Error in try lock\n");
        exit(0);
      }else
      {
        int no_children;
        if (base[i].child == NULL) no_children = 0;
        if (no_children == 0 && base[i].status == 0)
        {
          available = true;
          print_status(pthread_self(), base[i].job_id, 1, base[i].time);
          sleep(base[i].time);
          base[i].status = 2;
          print_status(pthread_self(), base[i].job_id, 2, base[i].time);
        }
        pthread_mutex_unlock(&base[i].lock);
      }

    }
  }
  return NULL;
}
void print_thread(pthread_t tid)
{
  unsigned char *ptc = (unsigned char*)(void*)(&tid);
  printf("thread : 0x");
  for (size_t i=0; i<sizeof(tid); i++) {
    printf("%02x", (unsigned)(ptc[i]));
  }
  fflush(stdout);
}
void print_status(pthread_t tid, int job_id, int status, int time)
{
  print_thread(tid);
  printf(" \t JOB : %d \t status : %s \t time : %d\n", job_id, status_string[status].c_str(), time);
  fflush(stdout);
}
void print_tree(NODE* tree)
{
  printf("Number of jobs %d\n", no_jobs);
  queue<NODE*> q;
  q.push(tree);
  while(q.size() != 0)
  {
    printf("Parent %d \t child:", q.front() -> job_id);
    NODE* it = q.front() -> child;
    q.pop();
    while(it != NULL)
    {
      printf("%d, ", it -> job_id);
      q.push(it);
      it = it -> next;
    }
    printf("\n");
  }
}


// void run(NODE* base)
// {
//   if (base == NULL) return;
//   if (base -> child == NULL || base )
//   {
//
//   }else
//   {
//     NODE* it = base;
//     while (it != NULL)
//     {
//       run(it);
//       it = it -> next;
//     }
//   }
// }
