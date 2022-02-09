#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

using namespace std;
typedef vector<string> vs;
typedef vector<vs> vvs;
typedef tuple<vs, string, string> tvsss;

typedef tuple<vs, int, int, int> ciop; // commands, input, output, piped

#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108
void parse(string &line, vs& commands_struct, int& last_command_bg)
{
  stringstream ss(line);
  string word;
  string temp="";
  vs commands;
  last_command_bg = 1;
  while(ss >> word)
  {
    // cout << word;
    if (word == "multiWatch")
    {
      commands_struct.push_back("multiWatch");
      string temp;
      vs args;
      int start = 0;
      for (int i = 0; i < line.size(); i++)
      {
        if (line[i] == '"' && start == 0)
        {
          start = 1;
        }else if (line[i] == '"' && start == 1)
        {
          if (temp != "")
          {
            stringstream ss(temp);
            commands_struct.push_back(temp);
          }
          temp ="";
          start = 0;
        }else if (start == 1)
        {
          temp = temp + line[i];
        }
      }
      last_command_bg = 0;
      return;
    }
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
void execute_simple(vs& tokens, int in_fd, int out_fd)
{
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
// void handle_sigint(int sig){
//   if (child_pid > 0){
//     printf("\nprocess id is %d\n", child_pid);
//     kill(child_pid, SIGKILL);
//   }else{
//     // printf("Error\n About to kill %d\n", child_pid);
//   }
// }
void handle_sigint(int sig)
{
  kill(getpid(), SIGKILL);
}
void handle_sigint_nt(int sig)
{
  cout << '\n';
  cout << getpid() << ">> ";
  fflush(stdout);
}
void handle_sigtstp_nt(int sig)
{
  printf("caught tstp\n");
  signal(SIGTSTP, handle_sigtstp_nt);
}
bool bg_sig = false;
void handle_sigtstp(int sig)
{
  // kill(child_pid, SIGKILL);
  printf("Caught control z\n");
  signal(SIGTSTP, handle_sigtstp);
  int new_child_pid = fork();
  if (new_child_pid == 0)
  {
  }else{
    kill(getpid(), SIGKILL);
  }
  // kill(getpid(), SIGSTOP);
  // int new_child_pid = fork();
  // if (new_child_pid == 0)
  // {
  //   // kill(getpid(), SIGCONT);
  // }else{
  //   bg_sig = true;
  // }
}
void handle_sigchld(int sig)
{
  printf("\nchild process %d is completed\n", child_pid);
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
bool command_is_multi(string& command)
{
  stringstream ss(command);
  string word;
  while(ss >> word)
  {
    if (word == "multiWatch")
    {
      return true;
    }
  }
  return false;
}
void get_file_list(vs& files)
{
  DIR *d;
  struct dirent* dir;
  d = opendir(".");
  if (d)
  {
    while((dir = readdir(d)) != NULL)
    {
      string temp(dir -> d_name);
      files.push_back(dir->d_name);
    }
    closedir(d);
  }
}
void divide(string &input, string& rest_string, string& last_key)
{
  int pos = input.find_last_of(' ');
  rest_string = input.substr(0, pos + 1);
  last_key = input.substr(pos+1);
  last_key.pop_back();
}
string autocomplete(string input)
{
  string key;
  string rest_string;
  divide(input ,rest_string ,key);
  vs files;
  get_file_list(files);
  int len = key.size();
  vs results;
  for (int i = 0; i < files.size(); i++)
  {
    if (key == files[i].substr(0, len))
    {
      results.push_back(files[i]);
    }
  }
  if(results.size() == 0)
  {
    printf("No file matches\n");
    return "";
  }else
  {
    cout << "\n";
    for (int i = 0;i < (int)results.size(); i++)
    {
      cout << i + 1 << ". "<< results[i] << '\n';
    }
    printf("\nChoose[or -1 for exit]: ");
    int option;
    cin >> option;
    // cout << results[option - 1];
    if (option == -1)
    {
      // printf("Hello");
      return "";
    }
    cout << rest_string << results[option -1];
    string ret = rest_string + results[option - 1];
    return ret;
  }
}
char getch()
{
  struct termios old_settings, new_settings;
  tcgetattr(STDIN_FILENO, &old_settings);
  new_settings = old_settings;
  new_settings.c_lflag = new_settings.c_lflag & ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
  char ch;
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
  return ch;
}
void readinput(string& s)
{
  int cursor = 0;
  while(1)
  {
    int ch = getch();
    if (ch == 127 || ch == 8)
    {
      if (cursor == s.size())
      {
        if(s.size())s.pop_back();
      }else
      {
        s.erase(cursor - 1, 1);
      }
      // cout << "\n" << cursor << '\n';
      // cout << "\n" << s << '\n';
      cursor = (cursor - 1 > 0) ? cursor - 1 : 0;
      printf("\b ");
      printf("\033[1D");
    }else if (ch == '\n')
    {
      printf("\n");
      break;
    }else if (ch == '\t')
    {
      s = s + (char)ch;
      break;
    }//else if (ch == '[')
    // {
    //   ch = getch();
    //   if (ch == 'D')
    //   {
    //     // cout << "\nLeft\n";
    //     cursor = (cursor - 1 > 0) ? cursor - 1 : 0;
    //     printf("\033[1D");
    //     // printf("a");
    //   }
    //   else if (ch == 'C')
    //   {
    //     // cout << "\r\nRight\n";
    //     cursor = (cursor + 1 < s.size()) ? cursor + 1 : s.size();
    //     printf("\033[1C");
    //   }
    //   // cout << "Left\n";
    //   // cursor = (cursor - 1 > 0) ? cursor - 1 : 0;
    //   // printf("\033[1D");
    //   // else if (ch == KEY_RIGHT)
    //   // {
    //   //   cout << "right\n";
    //   //   cursor = (cursor + 1 < s.size()) ? cursor + 1 : s.size();
    //   //   printf("\033[1C");
    //   // }
    //}
    else
    {
      if (cursor == s.size())
      {
        putchar(ch);
        s = s + (char)ch;
        cursor++;
      }else
      {
        s.insert(cursor, to_string(ch));
        cursor++;
      }
    }
    // cout << '\n' << cursor << '\n';
  }
}
void seperate(vs& commands, vvs& tokens)
{
  for (int i = 1; i < commands.size(); i++)
  {
    stringstream ss(commands[i]);
    string word;
    vs temp;
    while (ss >> word)
    {
      temp.push_back(word);
    }
    if (temp.size() > 0)tokens.push_back(temp);
  }
}
void run_multiWatch(vs& commands)
{
  vvs tokens;
  cout << "multiWatch\n";
  seperate(commands, tokens);
  for (int i = 0; i < tokens.size(); i++)
  {
    for (int j = 0; j < tokens[i].size(); j++)
    {
      cout << tokens[i][j] << ' ';
    }
    cout << '\n';
  }
  while(1)
  {
    vector<int> fds(tokens.size());
    vs temp_file_names;
    for (int i = 0; i < (int)tokens.size(); i++)
    {
      pid_t child_pid = fork();
      if (child_pid == 0)
      {
        string temp = ".temp." + to_string(getpid()) +  ".txt";
        fds[i] = open(temp.c_str(), O_RDWR | O_CREAT | O_TRUNC , S_IRWXG);
        dup2(fds[i], 1);
        const char **c_args = new const char *[tokens[i].size() + 1];
        for (int j = 0; j < tokens[i].size(); j++)
        {
          c_args[j] = tokens[i][j].c_str();
        }
        execvp(c_args[0], (char**)c_args);
      }else
      {
        string temp = ".temp." + to_string(child_pid) +  ".txt";
        temp_file_names.push_back(temp);
      }
    }
    struct pollfd *pfds = new struct pollfd [tokens.size()];
    time_t rawtime;
    struct tm * timeinfo;
    for (int i = 0; i < tokens.size(); i++)
    {
      fds[i] = open(temp_file_names[i].c_str(), O_RDONLY);
      cout << fds[i] << '\n';
      pfds[i].fd = fds[i];
      pfds[i].events = POLLIN;
    }
    cout << "started polling\n";
    poll(pfds, (int)tokens.size(), -1);
    cout << "finished polling\n";
    char buf[1024];
    int fd;
    struct winsize w;
    int width;
    for (int i = 0; i < (int)tokens.size(); i++)
    {
      if (pfds[i].revents & POLLIN)
      {
        if (fds[i] > 0)
        {
          time(&rawtime);
          timeinfo = localtime(&rawtime);
          printf("\n\"%s\", %02d:%02d:%02d\n", commands[i + 1].c_str(), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
          fflush(stdout);
          ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
          // printf ("lines %d\n", w.ws_row);
          // printf ("columns %d\n", w.ws_col);
          width = w.ws_col;
          for (int fill = 0; fill < width / 2; fill++) printf("<-");
          fflush(stdout);
          while((fd = read(fds[i], buf, 1024)) > 0)
          {
            write(1, buf, fd);
          }
          for (int fill = 0; fill < width / 2; fill++) printf("->");
          fflush(stdout);
          printf("\n");
          close(fds[i]);
        }
      }
    }
    // sleep(10);
  }

  // while(1)
  // {
  //
  // }
  cout << "killed multiWatch\n";
}
int main()
{
  cout << "\n";
  while(true)
  {
    // signal(SIGINT, handle_sigint);
    // signal(SIGTSTP, handle_sigtstp_nt);
    cout << getpid() << ">> ";
    // signal(SIGINT, handle_sigint_nt);
    fflush(stdout);
    // signal(SIGTSTP, handle_sigtstp);
    string line;
    // getline(cin, line);
    readinput(line);
    // signal(SIGINT, SIG_DFL);
    // cout << line;
    if (line.back() == '\t')
    {
      line = autocomplete(line);
      if (line == "")continue;
      cout << line << '\n';
    }
    if (line == "")continue;
    vs commands_struct;
    int last_command_bg;
    parse(line, commands_struct, last_command_bg);
    // pid_t child_pid;
    if (commands_struct.size() == 1 && commands_struct[0] == " exit()")
    {
      printf("\nBye!\n");
      break;
    }
    if (commands_struct[0] == "multiWatch")
    {
      child_pid = fork();
      if (child_pid == 0)
      {
        signal(SIGINT, handle_sigint);
        run_multiWatch(commands_struct);
      }else
      {
        signal(SIGINT, handle_sigint_nt);
        waitpid(child_pid, NULL, 0);
        continue;
      }
    }

    for (int i = 0; i < (int)commands_struct.size(); i++)
    {
      bool is_cd = command_is_cd(commands_struct[i]);
      child_pid = fork();
      // signal(SIGCHLD, handle_sigchld);
      if (child_pid == 0)
      {
        signal(SIGTSTP, handle_sigtstp);
        signal(SIGINT, handle_sigint);
        printf("\nchild process is %d\n", getpid());
        if (!is_cd)
        {
          run(commands_struct[i]);
        }
        kill(getpid() ,SIGKILL);
      }else
      {
        signal(SIGINT, handle_sigint_nt);
        signal(SIGTSTP, handle_sigtstp_nt);
        if (is_cd)
        {
          run(commands_struct[i]);
        }
        if (i == (int)(commands_struct.size())-1 && (last_command_bg == 0) && !bg_sig){
          int status;
          pid_t wait_pid = (waitpid(child_pid, &status, 0));
          // printf("HELLO parent\n");
          // printf("Process %d completed with exit status %d\n", wait_pid, WEXITSTATUS(status));
        }
      }
    }
  }
}
