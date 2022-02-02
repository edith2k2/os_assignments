#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

using namespace std;
typedef vector<string> vs;
typedef vector<vs> vvs;
typedef tuple<vs, string, string> tvsss;

typedef tuple<vs, int, int, int> ciop; // commands, input, output, piped

void parse(string &line, vs& commands_struct, int& last_command_bg)
{
  stringstream ss(line);
  string word;
  string temp;
  vs commands;
  last_command_bg = 1;
  while(ss >> word)
  {
    if (word == "&")
    {
      commands_struct.push_back(temp);
      temp = "";
    }else
    {
      temp = temp + " " + word;
    }
  }
  if (temp != ""){
    last_command_bg = 0;
    commands_struct.push_back(temp);
  }
}
void seperate(string& command, int& in_fd, int& out_fd, vvs& tokens)
{
  stringstream ss(command);
  string word;
  vs temp;
  in_fd = 0;
  out_fd = 1;
  while(ss >> word)
  {
    if (word == ">"){
      ss >> word;
      string output = word;
      out_fd = open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    }
    else if (word == "<")
    {
      ss >> word;
      string input = word;
      in_fd = open(input.c_str(), O_RDONLY);
    }
    else if (word == "|")
    {
      tokens.push_back(temp);
      temp.clear();
    }else
    {
      temp.push_back(word);
    }
  }
  tokens.push_back(temp);
}
void execute_simple(vs& tokens, int in_fd, int out_fd){
  if (in_fd != 0){
    dup2(in_fd, 0);
    close(in_fd);
  }
  if (out_fd != 1){
    dup2(out_fd, 1);
    close(out_fd);
  }
  const char **c_args = new const char *[tokens.size() + 1];
  for (int i = 0; i < tokens.size(); i++)
  {
    c_args[i] = tokens[i].c_str();
  }
  c_args[tokens.size()] = NULL;
  if (execvp(c_args[0], (char**)c_args) < 0){
    printf("Wrong command\n");
    exit(0);
  }
}
void execute_pipe(vvs& commands, int in_fd, int out_fd)
{
  int no_pipes = (int)commands.size() - 1;
  int FD[2];
  int temp_in_fd = in_fd;
  pid_t pipe_child_pid;
  for (int i = 0; i < (int)commands.size(); i++)
  {
    pipe(FD);
    pipe_child_pid = fork();
    if (pipe_child_pid == 0)
    {
      int act_out_fd = FD[1];
      if (i == (int)commands.size() - 1)
      {
        act_out_fd = out_fd;
      }
      execute_simple(commands[i], temp_in_fd, act_out_fd);
    }else{
      wait(NULL);
      close(FD[1]);
      temp_in_fd = FD[0];
    }
  }
}
// void execute_simple(vs& tokens, int in_fd, int out_fd){
//   pid_t child_pid = fork();
//   if (child_pid == -1 )
//   {
//     cout << "error in fork\n";
//     exit(0);
//   }else if (child_pid == 0)
//   {
//     printf("\nProcess %ld started\n", getpid());
//     if (in_fd != 0){
//       dup2(in_fd, 0);
//       close(in_fd);
//     }
//     if (out_fd != 1){
//       dup2(out_fd, 1);
//       close(out_fd);
//     }
//     const char **c_args = new const char *[tokens.size() + 1];
//     for (int i = 0; i < tokens.size(); i++)
//     {
//       c_args[i] = tokens[i].c_str();
//     }
//     c_args[tokens.size()] = NULL;
//     if (execvp(c_args[0], (char**)c_args) < 0){
//       printf("Wrong command\n");
//       exit(0);
//     }
//   }else{
//     int status;
//     // printf("child process is %d\n", child_pid);
//     pid_t wait_pid = (waitpid(child_pid, &status, 0));
//     printf("Process %ld completed with exit status %d\n", wait_pid, WEXITSTATUS(status));
//     // int status;
//     // if (waitpid(child_pid, &status, 0) == -1){
//     //   perror("wait error\n");
//     //   exit(0);
//     // }
//   }
// }
// void execute_pipe(vvs& commands, int in_fd, int out_fd)
// {
//   int no_pipes = (int)commands.size() - 1;
//   int FD[2];
//   int temp_in_fd = in_fd;
//   for (int i = 0; i < (int)commands.size(); i++)
//   {
//     if (i == (int)commands.size() - 1)
//     {
//       execute_simple(commands[i], temp_in_fd, out_fd);
//     }else
//     {
//       pipe(FD);
//       execute_simple(commands[i], temp_in_fd, FD[1]);
//       close(FD[1]);
//       temp_in_fd = FD[0];
//     }
//   }
// }
void run(string &command)
{
  int in_fd, out_fd;
  vvs sep_commands;
  seperate(command, in_fd, out_fd, sep_commands);
  if ((int)sep_commands.size() == 1){
    execute_simple(sep_commands[0], in_fd, out_fd);
  }else{
    execute_pipe(sep_commands, in_fd, out_fd);
  }
  if (out_fd == 1){
    fflush(stdout);
  }
}
pid_t child_pid = -1;
void handle_sigint(int sig){
  printf("process id is %d\n", child_pid);
  kill(child_pid, SIGKILL);
  // exit(0);
}
int main()
{
  cout << "\n";
  while(true)
  {
    cout << getpid() << ">> ";
    string line;
    getline(cin, line);
    vs commands_struct;
    int last_command_bg;
    parse(line, commands_struct, last_command_bg);
    // pid_t child_pid;
    signal(SIGINT, handle_sigint);
    for (int i = 0; i < (int)commands_struct.size(); i++)
    {
      child_pid = fork();
      if (child_pid == 0){
        run(commands_struct[i]);
        kill(getpid(), SIGKILL);
      }else
      {
        if (i == (int)(commands_struct.size())-1 && (last_command_bg == 0)){
          int status;
          printf("child process is %d\n", child_pid);
          pid_t wait_pid = (waitpid(child_pid, &status, 0));
          printf("Process %ld completed with exit status %d\n", wait_pid, WEXITSTATUS(status));
        }
        // wait(NULL);
        // if (i == (int)commands_struct.size() - 1 && !last_command_bg){
        //   wait(NULL);
        // }
      }
      // if (child_pid = (fork() == 0)){
      //
      // }else{
      //   int status;
      //   waitpid(child_pid, status, 0)
      // }
    }
  }
}
