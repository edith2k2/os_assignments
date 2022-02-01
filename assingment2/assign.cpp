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
void execute_simple(vs& tokens, int in_fd, int out_fd, int bg){
  pid_t child_pid = fork();
  if (child_pid == -1 )
  {
    cout << "error in fork\n";
    exit(0);
  }else if (child_pid == 0)
  {
    if (in_fd != 0){
      dup2(in_fd, 0);
      close(in_fd);
    }
    if (out_fd != 1){
      dup2(out_fd, 1);
      close(out_fd);
    }
    // close(out_fd);
    // close(in_fd);
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
  }else{
    if (!bg){
      wait(NULL);
    }
  }
}
void execute_pipe(vvs& commands, int in_fd, int out_fd, int bg)
{
  int no_pipes = (int)commands.size() - 1;
  int FD[2];
  int temp_in_fd = in_fd;
  for (int i = 0; i < (int)commands.size(); i++)
  {
    if (i == (int)commands.size() - 1)
    {
      execute_simple(commands[i], temp_in_fd, out_fd, 0);
    }else
    {
      pipe(FD);
      execute_simple(commands[i], temp_in_fd, FD[1], 1);
      close(FD[1]);
      temp_in_fd = FD[0];
    }
  }
}
void run(string &command, int bg)
{
  int in_fd, out_fd;
  vvs sep_commands;
  seperate(command, in_fd, out_fd, sep_commands);
  if ((int)sep_commands.size() == 1){
    execute_simple(sep_commands[0], in_fd, out_fd, bg);
  }else{
    execute_pipe(sep_commands, in_fd, out_fd, bg);
    pid_t child_pid = fork();
    if (child_pid == 0){

    }else{
      if (!bg){
        wait(NULL);
      }
    }
  }
}
int main()
{
  cout << "\n";
  while(true)
  {
    cout << ">> ";
    string line;
    getline(cin, line);
    vs commands_struct;
    int last_command_bg;
    parse(line, commands_struct, last_command_bg);
    pid_t child_pid;
    for (int i = 0; i < (int)commands_struct.size(); i++)
    {
      if (i == (int)commands_struct.size() - 1){
        run(commands_struct[i], last_command_bg);
      }else
      {
        run(commands_struct[i], 1);
      }
    }
  }
}
