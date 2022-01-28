#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include "stdio.h"
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>

using namespace std;
typedef vector<string> vs;
typedef tuple<vs, string, string> tvsss;

tvsss tokenise(string& command){
  stringstream ss(command);
  vs ret;
  string word;
  // checking for output and input files
  string output="STDOUT", input="STDIN";
  while(ss >> word)
  {
    if (word != ">" && word != "<")
    {
      ret.push_back(word);
      continue;
    }
    if (word == ">")
    {
      ss >> output;
    }
    if (word == "<"){
      ss >> input;
    }
  }
  return tie(ret, input, output);
}
void run(vs& args, string& input, string& output){
  const char **c_args = new const char *[args.size() + 1];
  for (int i = 0; i < args.size(); i++)
  {
    c_args[i] = args[i].c_str();
  }
  c_args[args.size()] = NULL;

  if (input != "STDIN")
  {
    int in_file = open(input.c_str(), O_RDONLY);
    close(0);
    dup(in_file);
    close(in_file);
  }
  if (output != "STDOUT")
  {
    int out_file = open(output.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    close(1);
    dup(out_file);
    close(out_file);
  }
  execvp(args[0].c_str(), (char**)c_args);
}
int main(){
  cout << "\n";
  while(true)
  {
    cout << ">> ";
    // input command
    string command;
    getline(cin, command);
    if (command == "exit")
    {
      cout << "Bye\n";
      break;
    }
    vs args;
    string input, output;
    tie(args, input, output) = tokenise(command);
    // simple command
    if(fork() == 0){
      run(args, input, output);
    }else{
      wait(NULL);
    }
  }
}
