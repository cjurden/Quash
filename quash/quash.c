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
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_BUFFER 1024
#define VALID_COMMAND_LENGTH 7
#define BASH_EXEC  "/bin/bash"
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
static char* VALID_COMMANDS[] = {"set", "echo", "cd", "pwd", "quit", "exit", "jobs"};

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

//returns number of commands
void read_command(command_t* cmd, char** cmdbuf){
  char* cstring = strtok(cmd->cmdstr, " ");
  int len = 0;
  //read cstring, break it up and go through each until you reach null.
  while(cstring != NULL){
    cmdbuf[len] = cstring;
  //  printf("these are individual command: %s\n", cstring);
    if(cstring == "|"){
      //PIPE_FLAG = 1;
    }
    cstring = strtok(NULL, " ");
    len++;
  }
  return len;
}

void exec_command(char* cmdbuf){
  //search global variable containing execs
  int p = check_for_pipe(cmdbuf);
  //printf("checking for pipe, returned %d\n", p);
  if(p > -1){
    printf("pipe!");
//char* args[2][100];
    char arg1[100];
    char arg2[100];
    for(int i = 0; i < 100; i++)
    {
      arg1[i] = NULL;
      arg2[i] = NULL;
    }
    for(int i = 0; i < p; i++)
    {
      //args[0][i] = cmdbuf[i];
      arg1[i] = cmdbuf[i];
      printf(cmdbuf[i]);
    }
    int j = 0;
    int i = p+1;
    while(cmdbuf[i] != NULL)
    {
      //args[1][j] = cmdbuf[i];
      arg2[j] = cmdbuf[i];
      j++;
      i++;
    }

    //will have to make exec command with pipe...
    exec_command_with_pipe(arg1, arg2);
  }
  else {
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
      char buf[] = "";
      join(cmdbuf, &buf);
      printf("did we get here?? %s\n", buf);
      if((execl(BASH_EXEC, BASH_EXEC, "-c", buf, (char*) 0))<0){
        fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmdbuf[0], errno);
      }
    }

      //if not there, check path
  }//end exec_command
/*
void store_commands_before_pipe(char** cmdbuf, int piploc){
  for(int i = 0; i < piploc-1; i++){

  }
}*///end store_commands_before_pipe

//passed 2D array with commands before and after pipe
//for more than one pipe, add parameter for # of pipes (same as # of rows in array)

//void exec_command_with_pipe(char*** argbuf)
void exec_command_with_pipe(char** arg1, char** arg2){
  //create pipe structure
  int fd_1[2];
  int fd_2[2];

  if (pipe(fd_1) == -1)
  {
    perror("pipe");
	  exit(EXIT_FAILURE);
  } else {
    //might need to declare this above...
    pid_t pid_1 = fork();
    if(pid_1 == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    } else {
      char buf[] = "";
      join(arg1, &buf);

      char cmdbuf[256];
      bzero(cmdbuf, 256);

      sprintf(cmdbuf, "%s", buf);
      printf("in the first process! %s", cmdbuf);
      dup2(fd_1[1], STDOUT_FILENO);

      close(fd_1[0]);
      close(fd_1[1]);
      if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0))<0) {
  		    fprintf(stderr, "\nError executing %s. ERROR#%d\n", arg1[0], errno);
      }
      //exit(0);
    }
  }
  if (pipe(fd_2) == -1)
  {
    perror("pipe");
	  exit(EXIT_FAILURE);
  } else {
    //might need to declare this above...
    pid_t pid_2 = fork();
    if(pid_2 == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    } else {


      char buf[] = "";
      join(arg2, &buf);

      char cmdbuf[256];
      bzero(cmdbuf, 256);

      sprintf(cmdbuf, "%s", buf);

      dup2(fd_2[0], STDIN_FILENO);
      dup2(fd_2[1], STDOUT_FILENO);

      close(fd_2[0]);
      close(fd_2[1]);
      if((execl(BASH_EXEC, BASH_EXEC, "-c", cmdbuf, (char*) 0))<0) {
  		    fprintf(stderr, "\nError executing %s. ERROR#%d\n", arg2[0], errno);
      }
      //exit(0);
    }
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

//see if pipes are in command line
int check_for_pipe(char** cmdbuf){
  int i = 0;
  while(cmdbuf[i]!=NULL){
    printf("this is cmdbuf[%d]: %s\n", i, cmdbuf[i]);

  //  sprintf(cur, "%s", cmdbuf[i]);
    if(strcmp(cmdbuf[i],"|")==0){
      printf("\napparent pipe at %d, %s\n",i, cmdbuf[i]);
      return i;
    }
    i++;
  }
  return -1;
}//end check_for_pipe

bool in_cmd_set(char* input)
{

}

//-------------EXECUTION METHODS ------------------//
void change_directory(const char* path) {
  if(chdir(path) < 0){
    puts("Error. Invalid directory.");
  }
  else{
    puts("Successfully changed to");
    puts(path);
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

}//end echo

void print_background_processes(){
  //need to implement

}//end print_background_processes

void set_env_variable(const char* var, const char* val){

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

    read_command(&cmd, &cmdbuf);
    printf(cmdbuf[0]);
    exec_command(&cmdbuf);
    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit"))
      terminate(); // Exit Quash
    else
      puts(cmd.cmdstr); // Echo the input string
  }

  return EXIT_SUCCESS;
}
