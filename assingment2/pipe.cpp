#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

typedef vector<string> vs;
typedef vector<vector<string>> vvs;
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
    int no_pipes = (int)commands.size() - 1;
    int** pipes = new int*[no_pipes];
    for (int i = 0; i < no_pipes; i++){
      pipes[i] = new int[2];
      pipe(pipes[i]);
    }
    for (int i = 0; i < (int)commands.size(); i++)
    {
      const char **c_args = new const char *[commands[i].size() + 1];
      for (int j = 0; j < (int)commands[i].size(); j++)
      {
        c_args[j] = commands[i][j].c_str();
      }
      c_args[commands[i].size()] = NULL;
      if (fork() == 0){
        if(i == 0){
          close(1); // closing output
          dup(pipes[i][1]); // dup write end at stdout's place
        }else if (i == (int)commands.size() - 1){
          close(0); // closing input
          dup(pipes[i - 1][0]); // dup read end at stdin's place
        }else{
          close(0);
          dup(pipes[i - 1][0]);
          close(1);
          dup(pipes[i][1]);
        }
        execvp(commands[i][0].c_str(), (char**)c_args);
      }else{
        wait(NULL);
      }
    }
  }
}
