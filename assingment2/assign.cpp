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
  string temp="";
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
  if (tokens[0] == "cd")
  {
    char dir[100];
    if (tokens.size() == 1 || tokens[1] == "~")
    {
      chdir(getenv("HOME"));
      printf("changed directory to %s",getcwd(dir, 100));
      return;
    }
    chdir(tokens[1].c_str());
    printf("changed directory to %s",getcwd(dir, 100));
    return;
  }
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
    bool is_cd = (commands[i][0] == "cd")? true : false;
    pipe_child_pid = fork();
    int act_out_fd = FD[1];
    if (i == (int)commands.size() - 1)
    {
      act_out_fd = out_fd;
    }
    if (pipe_child_pid == 0)
    {
      // int act_out_fd = FD[1];
      // if (i == (int)commands.size() - 1)
      // {
      //   act_out_fd = out_fd;
      // }
      if (!is_cd)execute_simple(commands[i], temp_in_fd, act_out_fd);
    }else
    {
      if (is_cd)execute_simple(commands[i], temp_in_fd, act_out_fd);
      wait(NULL);
      close(FD[1]);
      temp_in_fd = FD[0];
    }
  }
}
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
  if (child_pid > 0){
    printf("\nprocess id is %d\n", child_pid);
    kill(child_pid, SIGKILL);
  }else{
    // printf("Error\n About to kill %d\n", child_pid);
  }
}
bool bg_sig = false;
void handle_sigtstp(int sig)
{
  int new_child_pid = fork();
  if (new_child_pid == 0)
  {
  }else{
    bg_sig = true;
  }
}
bool command_is_cd(string& command)
{
  stringstream ss(command);
  string word;
  while(ss >> word)
  {
    if (word == "cd")
    {
      return true;
    }
  }
  return false;
}
int main()
{
  cout << "\n";
  while(true)
  {
    signal(SIGINT, handle_sigint);
    cout << getpid() << ">> ";
    // signal(SIGINT, handle_sigint);
    string line;
    getline(cin, line);
    vs commands_struct;
    int last_command_bg;
    parse(line, commands_struct, last_command_bg);
    // pid_t child_pid;
    if (commands_struct.size() == 1 && commands_struct[0] == " exit()")
    {
      printf("\nBye!\n");
      break;
    }

    for (int i = 0; i < (int)commands_struct.size(); i++)
    {
      bool is_cd = command_is_cd(commands_struct[i]);
      child_pid = fork();
      if (child_pid == 0)
      {
        if (!is_cd)
        {
          run(commands_struct[i]);
        }
        kill(getpid() ,SIGKILL);
      }else
      {
        // signal(SIGTSTP, handle_sigtstp);

        if (is_cd)
        {
          run(commands_struct[i]);
        }
        if (i == (int)(commands_struct.size())-1 && (last_command_bg == 0) && !bg_sig){
          int status;
          printf("\nchild process is %d\n", child_pid);
          pid_t wait_pid = (waitpid(child_pid, &status, 0));
          printf("Process %d completed with exit status %d\n", wait_pid, WEXITSTATUS(status));
        }
      }
    }
  }
}
