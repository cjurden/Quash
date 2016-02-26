/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h" // Putting this above the other includes allows us to ensure
                   // this file's headder's #include statements are self
                   // contained.

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_BUFFER 1024
#define VALID_COMMAND_LENGTH 7
#define BASH_EXEC "/bin/bash"
#define BSIZE 256
/**************************************************************************
 * Private Variables
 **************************************************************************/
/**
 * Keep track of whether Quash should request another command or not.
 */
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;
//static char* VALID_COMMANDS[] = {"set", "echo", "cd", "pwd", "quit", "exit", "jobs"};

/**************************************************************************
 * Private Functions
 **************************************************************************/
/**
 * Start the main loop by setting the running flag to true
 */
static void start() {
  running = true;
}

/**************************************************************************
 * Public Functions
 **************************************************************************/
bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

bool get_command(command_t* cmd, FILE* in) {
  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];

    if (last_char == '\n' || last_char == '\r') {
      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;
    }
    else
      cmd->cmdlen = len;
      printf("%s\n", cmd->cmdstr);
    return true;
  }
  else
    return false;
}

/*
* parse command will return a pointer to a character
* array containing all commands passed on a single line
*/
void parse_command(char* cmd){

  char* pch;
  int pipe_loc = 0;
  pch = strchr(cmd,'|');
  if(pch!=NULL)
  {
    //then we have a pipe character, need to split the commands and then execute
    printf ("found at %d\n", pch-cmd+1);
    pipe_loc = (int)(pch-cmd);

    //copy from 0 to pipe_loc - 1
    char* chop = strtok(cmd, "|\0");
    char* first_arg = chop;
    chop = strtok(NULL, "\0");
    char* second_arg = chop;

    //char second_arg[VALID_COMMAND_LENGTH+1];
    //second_arg[VALID_COMMAND_LENGTH] = "\0";
    //f/irst_arg[VALID_COMMAND_LENGTH] = "\0";
    char* temp1 = cmd;
    char* temp2 = cmd;
  //  strncpy(first_arg, temp1, pipe_loc-1);
    //printf("%d\n",pipe_loc);


  //  strncpy(second_arg, temp2+pipe_loc+1, strlen(cmd)-1);
    printf("%s",second_arg);
    //printf(pipe_loc);
    //pch = strchr(pch+1,'s');
    /*
    while(first_arg!="\0")
    {
      st
    }*/

    //start pipe process...
    int fd_1[2];
    pid_t pid_1, pid_2;
    if (pipe(fd_1) == -1)
    {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

      pid_1 = fork();
      if(pid_1 == -1)
      {
        perror("fork");
        exit(EXIT_FAILURE);
      } else {
  /*
        char cmdbuf[BSIZE];
        bzero(cmdbuf, BSIZE);

        printf("in the first process! %s", first);
        sprintf(cmdbuf, "%s", first);
        */
        dup2(fd_1[1], STDOUT_FILENO);

        if((execl(BASH_EXEC, BASH_EXEC, "-c", first_arg, (char*) 0))<0) {
            fprintf(stderr, "\nError executing %s. ERROR#%d\n", first_arg, errno);
        }
        close(fd_1[1]);
        close(fd_1[0]);
      //  parse_command(first_arg);
        exit(0);
      }
      //might need to declare this above...
      pid_2 = fork();
      if(pid_2 == -1)
      {
        perror("fork");
        exit(EXIT_FAILURE);
      } else {
        dup2(fd_1[0], STDIN_FILENO);

        if((execl(BASH_EXEC, BASH_EXEC, "-c", second_arg, (char*) 0))<0) {
            fprintf(stderr, "\nError executing %s. ERROR#%d\n", second_arg, errno);
        }
        close(fd_1[0]);
        close(fd_1[1]);
        exit(0);
      }
  }
  else{
    printf("did we get here?? %s\n", cmd);
    if((execl(BASH_EXEC, BASH_EXEC, "-c", cmd, (char*) 0))<0){
      fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmd, errno);
    }
  }
}

void exec_command(char* cmdbuf){
  /*
    for(int i = 0; i < len; i++){
        if(cmdbuf[i] == "set"){
          }else if(cmdbuf[i] == "echo"){
        }else if(cmdbuf[i] == "cd"){
        }else if(cmdbuf[i] == "pwd"){
        }else if(cmdbuf[i] == "quit"){
        }else if(cmdbuf[i] == "exit"){
        }else if(cmdbuf[i] == "jobs"){
        }else if(cmdbuf[i] == ""){
        }
    }*/

      //char buf[] = "";
    //  join(cmdbuf, &buf);
      printf("did we get here?? %s\n", cmdbuf);
      if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0))<0){
        fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmdbuf, errno);
      }
}

      //if not there, check path
  //end exec_command
/*
void store_commands_before_pipe(char** cmdbuf, int piploc){
  for(int i = 0; i < piploc-1; i++){

  }
}*///end store_commands_before_pipe

//passed 2D array with commands before and after pipe
//for more than one pipe, add parameter for # of pipes (same as # of rows in array)

void exec_command_with_pipe(char* first, char* second){
  //create pipe structure
  int fd_1[2];
  pid_t pid_1, pid_2;
  if (pipe(fd_1) == -1)
  {
    perror("pipe");
	  exit(EXIT_FAILURE);
  }

    pid_1 = fork();
    if(pid_1 == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    } else {
/*
      char cmdbuf[BSIZE];
      bzero(cmdbuf, BSIZE);

      printf("in the first process! %s", first);
      sprintf(cmdbuf, "%s", first);
      */
      dup2(fd_1[1], STDOUT_FILENO);

      /*close(fd_1[0]);
      if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0))<0) {
  		    fprintf(stderr, "\nError executing %s. ERROR#%d\n", first, errno);
      }*/
      parse_command(first);
      close(fd_1[1]);
      exit(0);
    }
    //might need to declare this above...
    pid_2 = fork();
    if(pid_2 == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    } else {
      dup2(fd_1[0], STDIN_FILENO);
      parse_command(second);
      close(fd_1[0]);
      exit(0);
    }
  }//end exec_command_with_pipe

//helper function for rejoining strings
void join(char** cmdbuf, char* buf)
{
  char** tempBuf = cmdbuf;
  int i = 0;

  //rebuild command string
  while(tempBuf[i] != NULL)
  {
    strcat(tempBuf[i], " ");
    strcat(buf, tempBuf[i]);
    i++;
  }
}

bool in_cmd_set(char* input)
{

}

//-------------EXECUTION METHODS ------------------//
void change_directory(const char* path) {
  if(chdir(path) < 0){
    puts("Error. Invalid directory.");
  }
  else{
    fprintf(stdout, "Successfully changed to %s\n",path );
  }
}//end change_directory

void print_working_directory(){
  char cwd[MAX_BUFFER];
  if(getcwd(cwd, sizeof(cwd)) != NULL){
    fprintf(stdout, "Current working directory: %s\n", cwd);
  }
  else{
    perror("getcwd() error");
  }
}//end print_working_directory

void execute_echo(const char* path_to_echo){

    if(strcmp(path_to_echo, "$PATH") == 0){
        //echo $PATH var
        fprintf(stdout,"PATH: %s\n",getenv("PATH"));
    }
    else if(strcmp(path_to_echo, "$HOME") == 0){
        //echo $HOME var
        fprintf(stdout,"HOME: %s\n",getenv("HOME"));
    }
    else{
        //just echo whatever was inputted
        puts(path_to_echo);
    }

}//end echo

void print_jobs(){

}//end print_background_processes

void set_env_variable(const char* var, const char* val){

    if(setenv(var,val,0) < 0){
        perror("set_env_variable() error\n");
    }

    else{
        fprintf(stdout, "Successfully set %s to %s.\n",var, val);
    }
}

/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */
int main(int argc, char** argv) {
  command_t cmd; //< Command holder argument
  char* cmdbuf[MAX_BUFFER]; //< array holding individual commands

  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.

    parse_command(cmd.cmdstr);
    //puts(check_for_pipe(&cmdbuf);

    //exec_command(&cmdbuf);

    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit")){
      terminate(); // Exit Quash
    }
    else{
      //puts(cmd.cmdstr); // Echo the input string
     }
   }//end while

  return EXIT_SUCCESS;
};
