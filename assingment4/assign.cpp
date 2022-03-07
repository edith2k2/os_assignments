#include <pthread.h>
#include <sys/shm.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// shmsize calculation:
// each process thread sleeps for 200-500ms and runs for 10-20s.
// the case for more memory created is each process sleeps less and runs more.
// which leads to sleep time = 200ms and run for 20s
// so each process thread creates 100 jobs in worst case (20s / 200ms)
// then we need to include the size of base tree
// shared memory size = (100 * p + 500) * job_size
#define Y 10
#define CHILDREN_SIZE 20

using namespace std;
vector<string> status({"not started", "on going", "done"});

struct NODE
{
  int job_id, status, time;
  pthread_mutex_t lock;
  struct NODE *children[CHILDREN_SIZE];
};
typedef struct NODE NODE;

void create_base_tree(NODE*);
void* producer_runner(void*);
void* consumer_runner(void*);
int no_jobs = 0;
pthread_mutex_t job_lock;
int main()
{
  int P;
  printf("enter P\n");
  scanf("%d\n", &P);
  key_t key = 7777;
  int shmid = shmget(key, sizeof(NODE) * (100 * P + 500), IPC_CREAT | 0666);
  NODE* base= (NODE*)shmat(shmid, NULL, 0);
  create_base_tree(base);
  pthread_t* pro_id_arr = new pthread_t [P];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_mutex_init(&job_lock, NULL);
  for (int i = 0; i < P; i++)
  {
    pthread_create(&(pro_id_arr[i]), &attr, producer_runner, base);
  }
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    int y = Y * P;
    pthread_t* con_id_arr = new pthread_t [y];
    for (int i = 0; i < y; i++)
    {
      pthread_create(&(con_id_arr[i]), &attr, consumer_runner, base);
    }
    for (int i = 0; i < y; i++)
    {
      pthread_join(con_id_arr[i], NULL);
    }
  }else
  {
    for (int i = 0; i < P; i++)
    {
      pthread_join(pro_id_arr[i], NULL);
    }
  }
}

void create_base_tree(NODE* base)
{
  int job_target = rand() % 200 + 300;
  no_jobs = job_target;
  for (int i = 0; i < job_target; i++)
  {
    // add job
  }
}
void* producer_runner(void* arg)
{
  NODE* base = (NODE*)arg;
  int run_time = (rand() % 10) + 10;
  int sleep_time = (rand() % 300) + 200;
  time_t start_time = time(NULL);
  while(time(NULL) - start_time < run_time)
  {
    // add job
    pthread_mutex_lock(&job_lock);
    int idx = rand() % no_jobs;
    pthread_mutex_unlock(&job_lock);
    pthread_mutex_lock(&((base + idx) -> lock));
    // add job to idx job
    pthread_mutex_unlock(&((base + idx) -> lock));
    // sleep
    sleep(sleep_time);
  }
  return NULL;
}
void* consumer_runner(void* arg)
{
  NODE* base = (NODE*)arg;
  bool available = true;
  while(available)
  {
    available = false;
    // check and sleep and delete
    // for (int i = 0; i < no_jobs; i++)
    // {
    //   if (base[i].children == NULL && base[i].status != 2)
    //   {
    //     available = true;
    //     // sleep
    //   }
    // }
  }
  return NULL;
}
