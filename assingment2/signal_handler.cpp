#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;
typedef vector<string> vs;
typedef vector<vector<string>> vvs;

pid_t child_pid = -1;
void handle_sigint(int sig){
  printf("process id is %d\n", child_pid);
  kill(child_pid, SIGKILL);
  // exit(0);
}
void handle_sigtstp(int sig){
  printf("process id is %d\n", getpid());
  kill(child_pid, SIGTSTP);
}
int main(){
  while (true)
  {
    printf("[%d] >> ", getpid());
    string line;
    getline(cin, line);
    stringstream ss(line);

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    string word;
    vs commands;
    vs temp;
    while( ss >> word){
      commands.push_back(word);
    }
    if (commands.size() == 1 && commands[0] == "exit")
    {
      kill(getpid(), SIGKILL);
    }
    const char **c_args = new const char* [commands.size() + 1];
    for (int j = 0; j < (int)commands.size(); j++)
    {
      c_args[j] = commands[j].c_str();
    }
    c_args[commands.size()] = NULL;
    child_pid = fork();
    if (child_pid == 0){
      execvp(commands[0].c_str(), (char**)c_args);
    }else{
      wait(NULL);
    }

  }
}
