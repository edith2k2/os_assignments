#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

typedef vector<string> vs;
typedef vector<vector<string>> vvs;
void run(const char** args, int len, int in_fd, int out_fd, int wait_id)
{
  // cout << in_fd << " " << out_fd << "\n";
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    dup2(in_fd, 0);
    dup2(out_fd, 1);
    // close(out_fd);
    // closed();
    execvp(args[0], (char**)args);
  }else{
    if (wait_id)
    {
      wait(NULL);
    }
  }
}
int main(){
  while (true)
  {
    cout << ">> ";
    string line;
    getline(cin, line);
    stringstream ss(line);

    string word;
    vvs commands;
    vs temp;
    while (ss >> word)
    {
      if (word == "|")
      {
        commands.push_back(temp);
        temp.clear();
      }
      else
      {
        temp.push_back(word);
      }
    }
    commands.push_back(temp);
    int FD[2];
    int in_fd = 0;
    for (int i = 0; i < (int)commands.size(); i++)
    {
      const char **c_args = new const char *[commands[i].size() + 1];
      for (int j = 0; j < (int)commands[i].size(); j++)
      {
        c_args[j] = commands[i][j].c_str();
      }
      c_args[commands[i].size()] = NULL;
      if (i == (int)commands.size() - 1)
      {
        // cout << "last command\n";
        run(c_args, (int)commands[i].size() + 1, in_fd, 1, 1);
      }else
      {
        if (pipe(FD) == -1){
          cout << "Pipe error\n";
          exit(0);
        }else
        {
          run(c_args, (int)commands[i].size() + 1, in_fd, FD[1], 0);
        }
        in_fd = FD[0];
      }
      // cout << "executed " << i << "\n";
    }
    // cout << "finish execution" << "\n";
    fflush(stdin);
    fflush(stdout);
  }
}
