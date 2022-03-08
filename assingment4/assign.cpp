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
  struct NODe *next;
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
  // pthread_t* pro_id_arr = new pthread_t [P];
  // pthread_attr_t attr;
  // pthread_attr_init(&attr);
  // pthread_mutex_init(&job_lock, NULL);
  // pthread_mutex_init(&job_add, NULL);
  // for (int i = 0; i < P; i++)
  // {
  //   pthread_create(&(pro_id_arr[i]), &attr, producer_runner, base);
  //   cout << "--> producer "; print_thread(pro_id_arr[i]); cout << " created\n";
  //   fflush(stdout);
  // }
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
  // initialising
  for (int i = 0; i < 100 * P + 500; i++)
  {
    // if (base == NULL) cout << "Hello\n";
    // cout << base << '\n';
    initialise_job(base[i]);
  }
  // adding jobs
  // int job_target = rand() % 200 + 300;
  int job_target = 10;
  no_jobs = job_target;
  // Prufer sequence
  vector<vector<int>> edges(job_target + 1);
  create_rand_tree(job_target, edges);

  for (int i = 0; i < job_target; i++)
  {
    create_job(base[i]);
  }
  for (int i = 0; i < job_target; i++)
  {
    for (int j = 0; j < (int)edges[i].size(); j++)
    {
      base[i].children[j] = &(base[edges[i][j] - 1]);
    }
    for (int j = (int)edges[i].size(); j < CHILDREN_SIZE; j++)
    {
      base[i].children[j] = NULL;
    }
  }
}
void create_rand_tree(int job_target, vector<vector<int>>& edges)
{
  vector<int> rand_seq(job_target - 2); // array to store n - 2 numbers
  vector<int> degree(job_target + 1, 1);
  for (int i = 0; i < job_target - 2; i++)
  {
    rand_seq[i] = (rand() % job_target) + 1;
    // cout << rand_seq[i] << ' ';
    degree[rand_seq[i]] += 1;
  }
  for (int i = 0; i < job_target - 2; i++)
  {
    printf("Parent: %d\n", rand_seq[i]);
    for (int j = 1; j <= job_target; j++)
    {
      if (degree[j] == 1)
      {
        edges[rand_seq[i]].push_back(j);
        edges[j].push_back(rand_seq[i]);
        printf("-- %d\n", j);
        degree[j]--;
        degree[rand_seq[i]]--;
        break;
      }
    }
  }
  int u = 0, v = 0;
  for (int i = 1; i < job_target + 1; i++)
  {
    if (degree[i] == 1 && u == 0) u = i;
    else if (degree[i] == 1 && v == 0) v = i;
  }

  edges[u].push_back(v);
  edges[v].push_back(u);
}
void initialise_job(NODE& job)
{
  job.job_id = -1;
  job.status = -1;
  job.time = -1;
  for (int i = 0; i < CHILDREN_SIZE; i++) job.children[i] = NULL;
}
void create_job(NODE& job)
{
  job.job_id = rand() % (int)pow(10, 8);
  job.status = 0;
  job.time = rand() % 251;
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
    pthread_mutex_lock(&job_lock); // get add to queue permission
    int current_jobs = no_jobs;
    int idx = rand() % no_jobs;
    pthread_mutex_unlock(&job_lock);

    pthread_mutex_lock(&job_add); // get add to queue permission
    initialise_job(base[current_jobs]);
    create_job(base[current_jobs]);
    pthread_mutex_init(&base[current_jobs].lock, NULL);
    pthread_mutex_lock(&(base[idx].lock)); // get permission for idx job
    base[current_jobs].children[0] = &(base[idx]);
    print_status(pthread_self(), base[current_jobs].job_id, 0, base[current_jobs].time);
    pthread_mutex_unlock(&(base[idx].lock));
    pthread_mutex_unlock(&job_add);
    int i;
    pthread_mutex_lock(&(base[idx].lock)); // get permission for idx job
    // add job to idx job
    for (i = 0; i < CHILDREN_SIZE; i++)
    {
      if (base[idx].children[i] == NULL) break;
    }
    base[idx].children[i] = &(base[no_jobs]);
    pthread_mutex_unlock(&(base[idx].lock));
    pthread_mutex_lock(&job_lock);
    no_jobs++;
    pthread_mutex_unlock(&job_lock);

    // sleep
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
        int no_children = 0;
        for (int j = 0; j < CHILDREN_SIZE; j++)
        {
          if (base[i].children[j] != NULL)
          {
            no_children += 1;
          }
        }
        if (no_children == 1 && base[i].status == 0)
        {
          available = true;
          print_status(pthread_self(), base[i].job_id, 1, base[i].time);
          sleep(base[i].time);
          // delete
          for (int j = 0; j < CHILDREN_SIZE; j++)
          {
            if (base[i].children[0] -> children[j] == &(base[i]))
            {
              base[i].children[0] -> children[j] = NULL;
              break;
            }
          }
          for (int j = 0; j < CHILDREN_SIZE; j++)
          {
            base[i].children[j] = NULL;
          }
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
  for (int i = 0; i < no_jobs; i++)
  {
    printf("parent %d: ", tree[i].job_id);
    for (int j = 0; j < CHILDREN_SIZE; j++)
    {
      if (tree[i].children[j] == NULL) continue;
      printf("%d, ", tree[i].children[j] -> job_id);
    }
    printf("\n");
  }
}
