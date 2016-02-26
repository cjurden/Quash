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
* parse_command takes the cmdstr and acts accordingly
*/
void parse_command(char* cmd){

  //CHECK FOR PIPE
  char* pch;
  pch = strchr(cmd,'|');
  if(pch!=NULL)
  {
    //then we have a pipe character, need to split the commands and then execute
    //use strtok to chop the strings, cleaner than strcpy
    char* chop = strtok(cmd, "|\0");
    char* first_arg = chop;
    printf("%s",first_arg);

    chop = strtok(NULL, "\0");
    char* second_arg = chop;
    printf("%s",second_arg);
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
        dup2(fd_1[1], STDOUT_FILENO);
        parse_command(first_arg);
        close(fd_1[1]);
        //close(fd_1[0]);
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
        parse_command(second_arg);
        close(fd_1[0]);
        //close(fd_1[1]);
        exit(0);
      }
  }
  else{
    //NO PIPE
    char* ptr;
    char* cmds[100] = {NULL};
    char* tempCmd = cmd;
    ptr = strtok(tempCmd, " ");
    int ind = 0;
    while(ptr != NULL)
    {
      cmds[ind] = ptr;
      printf("current string: %s\n", ptr);
      ptr = strtok(NULL, " ");
      ind++;
    }
    //printf("\n first string in command %s", cmds[0]);
    if(!strcmp(cmds[0], "set")){

    }else if(!strcmp(cmds[0], "echo")){

    }else if(!strcmp(cmds[0], "cd")){
      char path[100];
      int i = 1;
      printf("in cd, second argument: %s\n", cmds[1]);
      while(cmds[i]!=NULL)
      {
        char* temp = cmds[i];
        if(cmds[i+1]!=NULL)
        {
          strcat(temp, " ");
          strcat(path, temp);
        }
        strcat(path, temp);
        i++;
      }
      printf("path: %s", path);
      change_directory(path);
    }else if(!strcmp(cmds[0], "pwd")){

    }else if(!strcmp(cmds[0], "quit")){

    }else if(!strcmp(cmds[0], "exit")){

    }else if(!strcmp(cmds[0], "jobs")){

    }else if(!strcmp(cmds[0], "")){

    } else {
      printf("we are here!");
      printf("makign call to system with argument %s", cmds[0]);
      if((execvp(cmds[0], cmds))<0){
        fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmd, errno);
      }
    }
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
    parse_command(cmd.cmdstr);
    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit")){
      terminate(); // Exit Quash
    }
   }//end while

  return EXIT_SUCCESS;
};
