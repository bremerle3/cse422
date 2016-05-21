#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <csignal>
#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"
#include <fcntl.h>
#include "getopt.h"
#include <map>
#include <dirent.h>
#include <sys/ioctl.h>
#include <fstream>

using namespace std;

/*****************************Function prototypes******************************/
//Shell build-in commands
void show_f(char** argv_c, int argc_c);
void wait_f(char** argv_c, int argc_c);
int exit_f(char** argv_c, int argc_c);
void kill_f(pid_t pid);
void clr_f();
void set_f(char** argv_c, int argc_c);
void unset_f(char** argv_c, int argc_c);
void export_f(char** argv_c, int argc_c);
void unexport_f(char** argv_c, int argc_c);
void dir_f();
void chdir_f(char** argv_c, int argc_c);
void history_f(char** argv_c, int argc_c);
void repeat_f(char** argv_c, int argc_c);
void pause_f();
void environ_f();
void help_f();
//Interrupt handlers
void SIGINT_handler(int signal);
void SIGTSTP_handler(int signal);
void SIGSTOP_handler(int signal);
void SIGQUIT_handler(int signal); 
void SIGCONT_handler(int signal); 
void SIGCHLD_handler(int signal); 
void SIGABRT_handler(int signal); 
void SIGALRM_handler(int signal); 
void SIGHUP_handler(int signal); 
void SIGUSR1_handler(int signal); 
void SIGUSR2_handler(int signal); 

//Utility Functions
bool builtinCmds(char** argv_c, int argc_c);
void showProcessList();
void addProcess(int pid, const std::vector<char*>& argv_vec, int debugLevel);
std::string trimWhitespace(std::string const& strIn);
void initShell(int argc,char **argv);
std::string vec2string(const std::vector<char*>& argv_vec);
int requiresPipe(char** argv_c, int argc_c);
void redir_IO();
void splitArgs(std::vector<char*>& argv_vec, int pipeIdx);
void exec_pipe(char** argv_c, int argc_c, int pipeIdx);
void deleteProcess(pid_t pid);
void nonBuiltInCmds();
void processVarSub(char** argv_c, int argc_c);
int evaluate(char** argv_c, int argc_c);
void removeComments(char** argv_c, int & argc_c);

/*******************************Global Variables*******************************/
extern char ** environ; //Get the real shell environment variables.
const int MAX_NUM_PROCESSES = 1<<4; //Max number of jobs 
const int MAX_CMD_LENGTH = 1<<10;  
const int MAX_CMD_ARGS = 1<<8;  //On average, 4 chars per argument seems reasonable.a
const int MAX_FNAME_SIZE = 1<<6;  //Don't pick a filename longer than 64 chars!
int CURR_NUM_PROCESSES;
int status;
/*Command line args to sish.cpp*/
int cmdlineSub = 0; //-x flag 
int debugLevel;  //-d flag
int fileArg;  //-f flag
/*                              */
const int DEBUG_LVL_0 = 0; //No debug output
const int DEBUG_LVL_1 = 1; //Print process info (PID of forked child, PID of reaped zombie, etc)
const int DEBUG_LVL_2 = 2; //More detailed debug info
//vector<char*> leftSideCmd, rightSideCmd;  //store the cmdline split into two sides of a pipe
bool stdin_redirect, stdout_redirect;
char infile[MAX_FNAME_SIZE];
char outfile[MAX_FNAME_SIZE];
char shell_infile[MAX_FNAME_SIZE];
pid_t fg_pid;
pid_t last_fg;
pid_t last_bg;
int fg = 0;
//Map containing the local variables in the shell
std::map<string, string> local_vars;
// Vector containing the history of the commands.
std::vector<string> command_history;
char* argv_c[MAX_CMD_ARGS];
int argc_c;
int last_FG_value = 0;
std::vector<pid_t> children;

/********************************Main Function*********************************/
int main(int argc,char *argv[])
{
    //Initialize signal handlers.
    signal(SIGINT, SIGINT_handler);
    signal(SIGTSTP, SIGTSTP_handler);
    signal(SIGQUIT, SIGQUIT_handler);
    signal(SIGCONT, SIGCONT_handler);
    signal(SIGSTOP, SIGSTOP_handler);

    signal(SIGABRT, SIGABRT_handler);
    signal(SIGALRM, SIGALRM_handler);
    signal(SIGHUP, SIGHUP_handler);
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGUSR2, SIGUSR2_handler);


    //Initialize the shell with its command line arguments.
    initShell(argc, argv);
    //Parse command line input to the shell.a
    int reapStatus;
    char cmdline[MAX_CMD_ARGS];
    while(true)
    {
        if(debugLevel >= 1)
            cout << "Reaping zombie children..." << endl;
        //Wait on any background children
        pid_t reapPID = waitpid(-1, &reapStatus, WNOHANG|WCONTINUED);
        while(reapPID > 0)
        {
            if(debugLevel >= 1)
            {
                if(WIFSTOPPED(reapStatus))
                    cout << "PID " << reapPID << " stopped." << endl;
                else if(WIFCONTINUED(reapStatus))
                    cout << "PID " << reapPID << " continued." << endl;
                else if(WIFEXITED(reapStatus))
                    cout << "PID " << reapPID << " exited." << endl;
                cout << "Waitpid reaped child at top of while loop: " << reapPID << endl;
            }
            reapPID = waitpid(-1, &reapStatus, WNOHANG|WCONTINUED);
        }
        //Parse the shell commands.
        cout << "sish >> ";  //Display the command prompt before every command.
	char* fstatus = fgets(cmdline, MAX_CMD_ARGS, stdin);

       if(fileArg == 1)
       {
	 cout << fstatus;
           if(fstatus == NULL)
           {
	     // cout << "Reach end of file" << endl;
	     cout << endl;
	     return 0;
           }
           else if (ferror(stdin))
           {
               cout << "Input error" << endl;
           }
       }
        //if ((fgets(cmdline, MAX_CMD_ARGS, stdin) == NULL) && ferror(stdin))
        //    cout << "input error" << endl;
        for(int i=0; i<MAX_CMD_ARGS; i++)
        {
            if(cmdline[i] == '\n')
            {
                cmdline[i] = '\0';
                break;
            }
        }
        //if (feof(stdin)) { /* End of file (ctrl-d) */
        //    fflush(stdout);
        //    exit(0);
        //} 
        char* strPtr = strtok(cmdline, " \n\t");
        argc_c =0;
        while(strPtr != NULL)
        {
            argv_c[argc_c] = strPtr;
            strPtr = strtok(NULL, " \n\t");
            argc_c++;
        }

        argv_c[argc_c] = NULL;
	
	// Remove Commments from argv_c
	removeComments(argv_c,argc_c);

	// Variable Substitution into argv_c
	processVarSub(argv_c,argc_c);	

	int eval_result = evaluate(argv_c, argc_c);
	// -1 Means the program will keep running, any other value is terminate.
	if (eval_result != -1)
	  {
	    return eval_result;
	  }
    }
}

int evaluate(char** argv_c, int argc_c)
{
  int returnValue = -1;
  if(debugLevel >= 2 | cmdlineSub == 1)
    {
      cout << "Input command: ";
      for(int i =0; i<argc_c; i++)
	{
	  cout << argv_c[i] << " ";
	}
      cout << endl;
    }
  bool print_endl = true;
  bool isBuiltIn = false;
  // if(fileArg == 1)
  //  {
  //    print_endl = false;
  //  }
  //Handle built-in commands first. Command "exit" is a special case since it
  //causes the shell to return, so handle it before other builtins.
  if(argc_c != 0)
    {
      if(strcmp(argv_c[0], "exit") == 0)
	{
	  //return exit_f(argv_c, argc_c);
	  returnValue = exit_f(argv_c, argc_c);
	}
      else
	{
	  isBuiltIn = builtinCmds(argv_c, argc_c);
	}
      //Don't want spaces after clr;
      if(strcmp(argv_c[0], "clr") | strcmp(argv_c[0], "set") |strcmp(argv_c[0], "unset") |strcmp(argv_c[0], "kill") |strcmp(argv_c[0], "export") |strcmp(argv_c[0], "unexport") |strcmp(argv_c[0], "wait"))
	{
	  print_endl = false;
	}
      //Use fork() to spawn child processes for non-built in commands.
      //But first, parse for bg or IO redirect
      if(isBuiltIn == false)
	{
	  nonBuiltInCmds();
	}
      //Don't print an extra endl after clr.
      if(print_endl == true)
	{      
	  cout << endl;
	}
    }
  return returnValue;
}

/**************************Function Implementations****************************/
//Shell build-in command: show
void show_f(char** argv_c, int argc_c)
{
    for(size_t idx=1; idx<argc_c; idx++)
    {
        std::cout << argv_c[idx] << " ";
    }
    cout << endl;
}

//Shell build-in command: wait
void wait_f(char** argv_c, int argc_c)
{
    int wait_status;
    if(argc_c == 2)
    {
        pid_t waitPID = atoi(argv_c[1]);
        //Block while waiting.
        waitpid(waitPID, &wait_status, 0);
        if(debugLevel >= 2)
        {
            if(WIFSTOPPED(wait_status))
                cout << "PID " << waitPID << " stopped." << endl;
            else if(WIFCONTINUED(wait_status))
                cout << "PID " << waitPID << " continued." << endl;
            else if(WIFEXITED(wait_status))
                cout << "PID " << waitPID << " exited." << endl;
        }
    }
    else
        cout << "Wait syntax: wait PID" << endl;
}

//Shell build-in command: exit 
int exit_f(char** argv_c, int argc_c)
{
  for(int i=0; i<children.size();i++)
    {
      kill(children[i], SIGKILL);
    }
  if(argc_c == 2 && argv_c[1] == NULL) //If no exit value is given
    {
       return 0;
    }
    else if(argc_c > 1)
    {
        return atoi(argv_c[1]);
    }
    else 
    {
        return 0;
    }
}

//shell built-in command: kill
void kill_f(char** argv_c, int argc_c)
{
    pid_t pid;
    int sigLevel;
    int killStatus;
    string errMsg = "Format for kill:  kill [-n] pid";
    if(argc_c == 3)
    {
        pid = atoi(argv_c[2]);
        if(argv_c[1][0] == '-' && isdigit(argv_c[1][1]))
        {
            if(debugLevel >= 2)
                cout << "PID: " << pid << " sent signal SIGKILL" <<  endl;
            char* level_trunc = (argv_c[1]+1);
            sigLevel = atoi(level_trunc);
            cout << "sigLevel: " << sigLevel << endl;
            kill(pid, sigLevel);
            waitpid(pid, &killStatus, 0);
            if(debugLevel >= 2)
                cout << "PID: " << pid << " received signal SIGKILL" <<  endl;
        }
        else
            cout << "Format for kill:  kill [-n] pid" << endl;
    }
    else if(argc_c == 2)
    {
        pid = atoi(argv_c[1]);
        if(debugLevel >= 2)
            cout << "PID: " << pid << " sent signal SIGTERM" << endl;
        kill(pid, SIGKILL);
        waitpid(pid, &killStatus, 0);
        if(debugLevel >= 2)
            cout << "PID: " << pid << " received signal SIGTERM" <<  endl;
    }
    else
        cout << "Format for kill:  kill [-n] pid" << endl;
}

// Shell built-in command : set
void set_f(char** argv_c, int argc_c)
{
  if(argc_c < 3)
    {
      std::cout << "Error: Wrong number of inputs" << endl;
    }
  else
    {
      string var_name = std::string(argv_c[1]);
      string var_value = std::string(argv_c[2]);
      // Check if variable is already present
      std::map<string,string>::iterator it;
      // See if variable is currently defined.
      it = local_vars.find(var_name);
      if(it != local_vars.end())
	{
	  local_vars.erase(it);
	}
      // Create the new variable
      local_vars[var_name]=var_value;
      //local_vars.insert(std::pair<char*, char*>(var_name,var_value));
    }
}  
// Shell built-in command: unset
void unset_f(char** argv_c, int argc_c)
{
  if(argc_c < 2)
    {
      std::cout << "Error: Wrong number of inputs" << endl;
    }
  else
    {
      string var_name = std::string(argv_c[1]);
      // Check if variable is present
      std::map<string,string>::iterator it;
      // See if variable is currently defined.
      it = local_vars.find(var_name);
      if(it != local_vars.end())
	{
	  local_vars.erase(it);
	}
      else
	{
	  std::cout << "Error: The variable specified was not found." << endl;
	}
    }
}

// Shell built-in command: export
void export_f(char** argv_c, int argc_c)
{
  if(argc_c < 3)
    {
      std::cout << "Error: Wrong number of inputs" << endl;
    }
  else
    {
      string var_name = std::string(argv_c[1]);
      string var_value = std::string(argv_c[2]);
      setenv(var_name.c_str(),var_value.c_str(), true);
      }
}  
// Shell built-in command: unexport
void unexport_f(char** argv_c, int argc_c)
{
  if(argc_c < 2)
    {
      std::cout << "Error: Wrong number of inputs" << endl;
    }
  else
    {
      string var_name = std::string(argv_c[1]);
      unsetenv(var_name.c_str());
    }
}

// Shell built-in command: environ
void environ_f()
{
  // Print Local
  for(map<string, string>::iterator it = local_vars.begin(); it != local_vars.end(); it++)
    {
      cout << it->first << "=" << it->second << endl;
    }
  // Print Global
  int i = 1;
  char *str = *environ;
  while(str != NULL)
    {
      cout << str << endl;
      str = *(environ+i);
      i++;
    }
}

// Shell built-in command: dir
void dir_f()
{
  DIR *directory;
  struct dirent *dent;
  if((directory = opendir(".")) != NULL) 
    {
      while ((dent = readdir(directory)) != NULL) 
	{
	  std::cout<< dent->d_name << endl;
	}
      closedir(directory);
    }
}

// Shell built-in command: chdir
void chdir_f(char** argv_c, int argc_c)
{
    if(argc_c < 1)
    {
        std::cout << "Error: Wrong number of inputs" << endl;
    }
    else
    {
        int retval = chdir(argv_c[1]);
        if(retval == -1)
        {
            std::cout<< "Error: Unable to change directory" << endl;
        }
    }
}

// Shell built-in command: history
void history_f(char** argv_c, int argc_c)
{
    long int num;
    int startVal = 0;
    if (argv_c[1] == NULL)
    {
        num = 0;
    }
    else
    {
        num = strtol(argv_c[1], NULL, 10);
    }
    if(num == 0)
    {
        num = command_history.size();
    }
    else
    {
        startVal = command_history.size() - num;
    }
    for(int i=startVal; i<command_history.size(); i++)
    {
      std::cout << i+1  << "  "  << command_history[i] << endl;
    }
}

// Shell built-in command: repeat
void repeat_f(char** argv_c, int argc_c)
{
  long int num;
  if (argv_c[1] == NULL)
    {
      num = 0;
    }
  else
    {
      num = strtol(argv_c[1], NULL, 10);
    }
    if(num == 0)
    {
      num = command_history.size() - 1;
    }

    if(num < command_history.size())
      {
	cout << "Will execute: " << command_history[num-1] << endl;
	
	istringstream iss(command_history[num-1]);
	string s;
	
	vector<string> command;
	while(getline(iss, s, ' '))
	  {
	    command.push_back(s);
	  }
	char* arg_in[command.size()];
	for(int i=0; i<command.size(); i++)
	  {
	    string cString = command[i];
	    char * c_val = new char[cString.size() + 1];
	    std::copy(cString.begin(), cString.end(), c_val);
	    c_val[cString.size()] = '\0';
	    arg_in[i] = c_val;
	  }
	arg_in[command.size()] = NULL;

	evaluate(arg_in, command.size());
      }
    else
      {
	std::cout << "Error: Cannot repeat command.  Input Command Number Invalid." << endl;
      }
    
}    

// Shell built-in command: pause
void pause_f()
{
  cout << "Press ENTER to continue"<<endl;
  cin.ignore(1);
}

//Shell build-in command: clr 
void clr_f()
{
    /*ANSI/VT100 escape sequence
      \033  == <Esc>
      [2J   ==  Erases the screen with the background colour and moves the cursor to home. 
      [1;1H == Indicates that subsequent text will begin at row 1, col 1.
      */
    cout << "\033[2J\033[1;1H";
}

//Shell built-in command: help
void help_f()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int lines = w.ws_row;
  int cline = 0;
  string line;
  ifstream help_file ("README.txt");
  if(help_file.is_open())
    {
      while(getline(help_file, line))
	{
	  if (cline < lines)
	    {
	      std::cout << line << endl;
	      cline = cline + 1;
	    }
	  else
	    {
	      cout << "Press ENTER for MORE help" << endl;
	      cin.ignore(1);
	      cline = 0;
	    }
	}
      help_file.close();
    }
  else
    {
      std::cout << "Error: Unable to access help" << endl;
    }
}

//Interrupt handler: SIGINT (ctrl-c)
void SIGABRT_handler(int signal)
{
    return;
}//Interrupt handler: SIGINT (ctrl-c)
void SIGALRM_handler(int signal)
{
    return;
}//Interrupt handler: SIGINT (ctrl-c)
void SIGHUP_handler(int signal)
{
    return;
}//Interrupt handler: SIGINT (ctrl-c)
void SIGUSR1_handler(int signal)
{
    return;
}
//Interrupt handler: SIGINT (ctrl-c)
void SIGUSR2_handler(int signal)
{
    return;
}

//Interrupt handler: SIGINT (ctrl-c)
void SIGINT_handler(int signal)
{
    if(fg)
    {
        kill(fg_pid, SIGINT);
    }
    return;
}

//Interrupt handler: SIGTSTP (ctrl-z)
void SIGTSTP_handler(int signal)
{
    if(fg)
    {
        last_bg = fg_pid;
        kill(fg_pid, SIGTSTP);
    }
    return;
}

//Interrupt handler: SIGQUIT (ctrl-\)
void SIGQUIT_handler(int signal) 
{
    if(fg)
    {
        kill(fg_pid, SIGQUIT);
    }
    return;
}

//Interrupt handler: SIGCONT  (ctrl-q)
void SIGCONT_handler(int signal) 
{
    if(fg == 0)
    {
        kill(last_fg, SIGCONT);
    }
    return;
}

//Interrupt handler: SIGSTOP 
void SIGSTOP_handler(int signal) 
{
    if(fg)
    {
        kill(fg_pid, SIGSTOP);
    }
    return;
}

//Interrupt handler: SIGCHLD
void SIGCHLD_handler(int signal) 
{
    pid_t child_pid;
    while(child_pid = waitpid(-1, 0, WNOHANG > 0))
    {
    }
    return;
}

bool builtinCmds(char** argv_c, int argc_c)
{
    bool isBuiltIn;
    const int numBuiltins = 17;
    const string allBuiltins[numBuiltins] = {"show", "set", "unset", "export", "unexport", "environ", "chdir", "exit", "wait", "clr", "dir", "echo", "help", "pause", "history", "repeat", "kill"};
    std::string inputCmd(argv_c[0]);
    for(size_t idx=0; idx<numBuiltins; idx++)
    {
        if(inputCmd.compare(allBuiltins[idx]) == 0)
        {
            isBuiltIn = true;
            // Add command to history
            string cCommand = "";
            for (int j=0; j<argc_c;j++)
            {
                cCommand = cCommand + string(argv_c[j]) + " ";
            }
            command_history.push_back(cCommand);
            if(inputCmd.compare("show") == 0)
            {
                show_f(argv_c,argc_c);
            }
            else if(inputCmd.compare("wait") == 0)
            {
                wait_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("clr") == 0)
            {
                clr_f();
            }
            else if(inputCmd.compare("set") == 0)
            {
                set_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("unset") == 0)
            {
                unset_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("export") == 0)
            {
                export_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("unexport") == 0)
            {
                unexport_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("dir") == 0)
            {
                dir_f();
            }
            else if(inputCmd.compare("chdir") == 0)
            {
                chdir_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("echo") == 0)
            {
                show_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("history") == 0)
            {
                history_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("repeat") == 0)
            {
                repeat_f(argv_c, argc_c);
            }
            else if (inputCmd.compare("kill") == 0)
            {
                kill_f(argv_c, argc_c);
            }
            else if(inputCmd.compare("pause") == 0)
            {
                pause_f();
            }
            else if(inputCmd.compare("environ") == 0)
            {
                environ_f();
            }
            else if(inputCmd.compare("help") == 0)
            {
                help_f();
            }
            else
            {
                isBuiltIn = false;
            }
            break;
        }
    }
    return isBuiltIn;
}

void nonBuiltInCmds()
{
    //Detect background jobs.
    bool isBG;
    if(argv_c[argc_c-1][0] == '!')
    {
        argv_c[argc_c-1] = NULL;
        argc_c--;
        isBG= true;
    }
    else 
        isBG = false;

    //Decide if command has any pipes.
    int piped = requiresPipe(argv_c, argc_c);
    if(debugLevel >= DEBUG_LVL_2)
        cout << "Command line index of first pipe is: " << piped << endl;
    //Fork process to execute non-buildin command.
    pid_t pid = fork();
    children.push_back(pid);
    if(pid < 0)
    {
        if(debugLevel >= DEBUG_LVL_1)
            cout << "Fork failed!" << endl;
    }
    else if(pid > 0)  //Parent process
    {
        //Parent only waits on FG processes.
        if(!isBG)
        {
            int status;
            if(debugLevel >= DEBUG_LVL_2)
                cout << "Parent Process: Adding child process to job list." << endl;
            pid_t wait_pid;
            wait_pid = waitpid(pid, &status, WUNTRACED); //FG process blocks parent
            if(pid > 0)
                last_fg = wait_pid;
            if(debugLevel >= DEBUG_LVL_1)
            {
                cout << "last_fg = " << last_fg << endl;
                cout << "fg = " << fg << endl;
                if(WIFSTOPPED(status))
		  cout << "PID " << last_fg << " stopped." << endl;
                else if(WIFCONTINUED(status))
		  cout << "PID " << last_fg << " continued." << endl;
                else if(WIFEXITED(status))
		  cout << "PID " << last_fg << " exited." << endl;
            }
	    if(WIFEXITED(status))
	      {
		last_FG_value = WEXITSTATUS(status);
	      }
        }
        else
            last_bg = pid;
    }
    else if(pid == 0)  //Child process
    {
        if(!isBG)
        {
            fg_pid = getpid(); //There can only be one FG process at a time.
            fg = 1;
        }
        if(debugLevel >= DEBUG_LVL_2)
            cout << "Child thinks pipe index is: " << piped << endl;
        if(piped < 0)
        {
            if(debugLevel >= 2)
            {
                cout << "Finished checking for IO redirection. Command: " << endl;
                for(int i=0; i<argc_c; i++)
                {
                    cout << argv_c[i] << endl;
                }
            }
            if(debugLevel >= DEBUG_LVL_1)
            {
                if(isBG)
                    cout << "Background ";
                else
                    cout << "Foreground ";
                cout << "Child Process(PID " << getpid() << ")" << " has been forked to execute the command: " << argv_c[0] << endl;
            }
            //Detect and set up IO redirection.
            //redir_IO();
            
            //Handle output redirect if necessary 
            if(stdout_redirect)
            {
                int fd1 = open(outfile, O_WRONLY|O_CREAT, 0644);
                if(fd1<0)
                    cout << "Failed to open outfile " << outfile << endl;
                dup2(fd1, 1); //redirect stdout to fd1
                close(fd1);
            }
            //Handle input redirect if necessary.
            if(stdin_redirect)
            {
                int fd2 = open(infile, O_RDONLY);
                if(fd2<0)
                    cout << "Failed to open infile " << infile << endl;
                dup2(fd2, 0); //redirect stdin to fd2
                close(fd2);
            }
            
            if(execvp(argv_c[0], argv_c) < 0)
            {
                if(debugLevel >= DEBUG_LVL_1)
                    cout << "Error: shell tried to execute an unknown command." << endl;
                exit(1);  //Exit from the child process
            }
        }
        else
        {
            exec_pipe(argv_c, argc_c, piped);
        }
    }
}

//Utility Function: redir_IO()
void redir_IO()
{
    //Reset redirection to false from previous loops.
    stdin_redirect = false;
    stdout_redirect = false;
    const int fileIOidx1 = argc_c-2;
    const int fileIOidx2 = argc_c-4;
    bool eraseidx1 = false;
    bool eraseidx2 = false;
    if(argc_c >= 3)
    {
        std::string fileIO1 = std::string(argv_c[fileIOidx1]);
        if(fileIO1.compare("<") == 0)
        {
            strcpy(infile, argv_c[fileIOidx1+1]);
            eraseidx1 = true;
            stdin_redirect = true;
        }
        else if (fileIO1.compare(">") == 0)
        {
            strcpy(outfile, argv_c[fileIOidx1+1]);
            eraseidx1 = true;
            stdout_redirect = true;
        }
    }
    else if (argc_c >= 5)
    {
        std::string fileIO2 = std::string(argv_c[fileIOidx2]);
        if (fileIO2.compare("<") == 0)
        {
            strcpy(infile, argv_c[fileIOidx2+1]);
            eraseidx2 = true;
            stdin_redirect = true;
        }
        else if (fileIO2.compare(">") == 0)
        {
            strcpy(outfile, argv_c[fileIOidx2+1]);
            eraseidx2 = true;
            stdout_redirect = true;
        }
    }
    if(eraseidx2 == true)
    {
        argv_c[fileIOidx2] = NULL;
        argc_c = argc_c - 2;
    }
    if(eraseidx1 == true)
    {  
        argv_c[fileIOidx1] = NULL;
        argc_c = argc_c - 2; 
    }
    //Handle output redirect if necessary 
    if(stdout_redirect)
    {
        int fd1 = open(outfile, O_WRONLY|O_CREAT, 0644);
        if(fd1<0)
            cout << "Failed to open outfile " << outfile << endl;
        dup2(fd1, 1); //redirect stdout to fd1
        close(fd1);
    }
    //Handle input redirect if necessary.
    if(stdin_redirect)
    {
        int fd2 = open(infile, O_RDONLY);
        if(fd2<0)
            cout << "Failed to open infile " << infile << endl;
        dup2(fd2, 0); //redirect stdin to fd2
        close(fd2);
    }
}

//Utility Function: trimWhiteSpace
std::string trimWhitespace(std::string const& strIn)
{
    if(strIn.empty() == true)
    {
        return strIn;
    }
    std::size_t first = strIn.find_first_not_of(' ');
    std::size_t last = strIn.find_last_not_of(' ');
    if(first == std::string::npos)
    {
        first = strIn.length();
    }
    return strIn.substr(first, last-first+1);
}

//Utility Function: initShell
void initShell(int argc,char **argv)
{
    //Set some defaults
    stdin_redirect = false;
    stdout_redirect = false;
    int option = 0;
    debugLevel = 0;  //No debug info be default
    while((option = getopt(argc, argv, "xd:f:")) != -1)
    {
        switch(option)
        {
            case 'x':
                cmdlineSub = 1;
                break;
            case 'd':
                debugLevel = atoi(optarg);
                break;
            case 'f':
                {
                    fileArg = 1;
                    strcpy(shell_infile, optarg);
                    cout << "infile: " << shell_infile << endl;
                    int fd = open(shell_infile, O_RDONLY);
                    if(fd < 0)
                        cout << "Failed to open shell input file." << endl;
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    break;
                }
            default:
                cout << "Usage: sish [-x] [-d <level>] [-f file [arg] ... ]" << endl;
                exit(0);
        }           
    }
}

//Utility Function: vec2string
std::string vec2string(const std::vector<char*>& argv_vec)
{
    std::string cmdString = "";
    for(int idx=0; idx<argv_vec.size()-1; idx++)
    {
        if(idx==0)  //No leading spaces for first arg
        {
            cmdString = cmdString + std::string(argv_vec[idx]);
        }
        else
        {
            cmdString = cmdString + " " + std::string(argv_vec[idx]);
        }
    }
    //Stick in the null terminator
    cmdString = cmdString + "\0";
    return cmdString;
}

//Utility Function: exec_pipe
void exec_pipe(char** argv_c, int argc_c, int pipeIdx)
{
    //Divide the input arg into a left side and a right side.
    char** leftSideCmd = argv_c;
    char** rightSideCmd = argv_c+pipeIdx+1;
    int nLHS = pipeIdx;
    int nRHS = argc_c - pipeIdx -1;
    leftSideCmd[pipeIdx] = '\0';
    //set up the pipe
    int pd[2];
    pipe(pd);
    pid_t pid;
    if((pid = fork()) < 0)
    {
      children.push_back(pid);  
      if(debugLevel >= DEBUG_LVL_1)
            cout << "Error: fork() failed in function exec_pipe." << endl;
    } 
    else if(pid == 0)  //begin child
    {
        dup2(pd[1],1); //dup into stdout
        close(pd[0]); //close(pd[0]), this side of pipe is producer and doesn't need any stdin
        close(pd[1]);
        //redir_IO();
       stdin_redirect = false;
           if((nLHS>= 2) && (leftSideCmd[nLHS-2][0] == '<'))
           {
              stdin_redirect = true;
              strcpy(infile, leftSideCmd[nLHS-1]);
              leftSideCmd[nLHS-2] = NULL;
              nLHS = 2;
           } 

            if(stdin_redirect)
            {
                int fd2 = open(infile, O_RDONLY);
                if(fd2<0)
                    cout << "Failed to open infile " << infile << endl;
                dup2(fd2, 0); //redirect stdin to fd2
                close(fd2);
            }
        if(debugLevel >= DEBUG_LVL_1)
        {        cout << "leftSIdeCmd: ";
            for (int i=0; i<nLHS; i++)
            {
                cout << leftSideCmd[i] << endl;
            }
        }
        if(execvp(leftSideCmd[0], leftSideCmd) < 0)
        {
            if(debugLevel >= DEBUG_LVL_1)
                cout << "Error: In function exec_pipe, LHS of pipe failed to execvp" << endl;
        }
        exit(1);
    } //end child
    else if(pid > 0)  //begin parent
    {
        dup2(pd[0],0);  //dup into stdin
        close(pd[1]);  //close pd[1], this side of pipe is consumer and doesn't need any stdout
        close(pd[0]);
        if(debugLevel >= DEBUG_LVL_1)
        {        cout << "rightSideCmd: ";
            for (int i=0; i<nRHS; i++)
            {
                cout << rightSideCmd[i] << endl;
            }
        }
        int nextPipeIdx = requiresPipe(rightSideCmd, nRHS);
        if(debugLevel >= DEBUG_LVL_1)
            cout << "Grandchild thinks next pipe is at: " << nextPipeIdx << endl;
        if(!(nextPipeIdx < 0))
        {
            if(debugLevel >= 2)
            {
                cout << "Parent thinks no further pipe needed. Exec command :";
                for(int i=0; i<nRHS; i++)
                {
                    cout << rightSideCmd[i];
                }
                cout << endl;
            }
            exec_pipe(rightSideCmd, nRHS, nextPipeIdx);
        }
        else 
        {
          stdout_redirect = false; 
           if((nRHS>= 2) && (rightSideCmd[nRHS-2][0] == '>'))
           {
              stdout_redirect = true;
              strcpy(outfile, rightSideCmd[nRHS-1]);
              rightSideCmd[nRHS-2] = NULL;
              nRHS = 2;
           } 
    if(stdout_redirect)
    {
        int fd1 = open(outfile, O_WRONLY|O_CREAT, 0644);
        if(fd1<0)
            cout << "Failed to open outfile " << outfile << endl;
        dup2(fd1, 1); //redirect stdout to fd1
        close(fd1);
    }
            for(int i=0; i<nRHS; i++)
            {
                cout << rightSideCmd[i];
            }
            cout << endl;
            if(execvp(rightSideCmd[0], rightSideCmd) < 0)
            {
                if(debugLevel >= DEBUG_LVL_1)
                    cout << "Error: In function exec_pipe, RHS of pipe failed to execvp" << endl;
            }
            exit(2);
            //end parent
        }
    }
}

//Utility Function: requiresPipe
int requiresPipe(char** argv_c, int argc_c)
{
    for(int idx=0; idx<argc_c; idx++)
    {
        if(strcmp(argv_c[idx], "|") == 0)
        {
            return idx;
        }
    }
    return -1;
}

// Utility Function: processVarSub
void processVarSub(char** argv_c, int argc_c)
{
  for(int i=0;i<argc_c;i++)
    {
      if(argv_c[i][0] == '$')
	{
	  if(debugLevel >= DEBUG_LVL_1)
	    {
	      cout << "$ Detected" << endl;
	    }
	  if(argv_c[i][1] == '$')
	    {
	      int pid_value = getpid();
	      std::stringstream ss;
	      ss << pid_value;
	      string pid_str = ss.str();
	      char * newVal_c = new char[pid_str.size() + 1];
	      std::copy(pid_str.begin(), pid_str.end(), newVal_c);
	      newVal_c[pid_str.size()] = '\0';
	      argv_c[i] = newVal_c; 

	    }
	  else if(argv_c[i][1] == '?')
	    {
	      std::stringstream ss;
	      ss << last_FG_value;
	      string pid_str = ss.str();
	      char * newVal_c = new char[pid_str.size() + 1];
	      std::copy(pid_str.begin(), pid_str.end(), newVal_c);
	      newVal_c[pid_str.size()] = '\0';
	      argv_c[i] = newVal_c;	      
	    }
	  else if(argv_c[i][1] == '!')
	    {
	      int pid_value = last_bg;
	      std::stringstream ss;
	      ss << pid_value;
	      string pid_str = ss.str();
	      char * newVal_c = new char[pid_str.size() + 1];
	      std::copy(pid_str.begin(), pid_str.end(), newVal_c);
	      newVal_c[pid_str.size()] = '\0';
	      argv_c[i] = newVal_c;
	    }
	  else
	    {
	      string search_term = string(argv_c[i]);
	      search_term = search_term.substr(1);
	      // Check local variables
	      if(local_vars.count(search_term) == 1)
		{
		  string newVal = local_vars[search_term];
		  char * newVal_c = new char[newVal.size() + 1];
		  std::copy(newVal.begin(), newVal.end(), newVal_c);
		  newVal_c[newVal.size()] = '\0';
		  argv_c[i] = newVal_c;
		}
	      else
		{
		  // Check global variables
		  char *str = *environ;
		  int j = 1;
		  while(str != NULL)
		    {
		      string variable = string(str);
		      size_t equalPos = variable.find("=");
		      string key = variable.substr(0, equalPos);
		      string value = variable.substr(equalPos+1);
		      if(key == search_term)
			{
			  char * newVal_c = new char[value.size() + 1];
			  std::copy(value.begin(), value.end(), newVal_c);
			  newVal_c[value.size()] = '\0';
			  argv_c[i] = newVal_c;
			  break;
			}
		      str = *(environ+j);
		      j++;
		    }
		}
	    }
	}
    }
}

void removeComments(char** argv_c, int & argc_c)
{
  for(int i=0; i < argc_c; i++)
    {
      for(int j=0; j < strlen(argv_c[i]); j++)
	{
	  if(argv_c[i][j] == '#')
	    {
	      argv_c[i] = NULL;
	      argc_c = i;
	      break;
	    }
	}
    }
}
