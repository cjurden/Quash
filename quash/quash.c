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

//for use with wait PID
static int status;

//to keeep track of background and foreground processes...
static int jc = 0;
static struct Job* jobs[50];


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
* 3 cases for pipes, iniitial command that requires one std_out, middle case that requires in and out, end case that requires in
* keep track of each of those pipes
* can resuse arrays ,use 2 fd_1[2]s, kep track of beginning and end...
* cmd1[], cmd2[], cmdall[][] where cmd all holds all commands, and removes the front one after execution.
* keep track of PID
*/

void add_job(struct Job* job){
  jobs[jc] = job;
  jc++;
  printf("added job %s to index %d with pid %d\n", job->command, jc, job->pid);
}

//NOTE: need to work on this part
void remove_job(struct Job* job){
  for(int i = 0; i < jc; i++){
    if(jobs[jc]->pid == job->pid){
      jobs[jc] = NULL;
    }
    jc--;
    //limit is fifty because that is the size of the array...
    for(int i = 0; i < 50; i ++){
      struct Job* temp = jobs;
    }
  }
}

void parse_command(char* cmd){
  bool bg = false;
  //CHECK FOR PIPE
  char* pch;
  char* ich;
  pch = strchr(cmd,'|');
  ich = strchr(cmd, '>');
  if(ich!=NULL)
  {

  }
  if(pch!=NULL)
  {
    //then we have a pipe character, need to split the commands and then execute
    //use strtok to chop the strings, cleaner than strcpy
    char* chop = strtok(cmd, "|\0");
    char* first_arg = chop;
    //printf("%s",first_arg);

    chop = strtok(NULL, "\0");
    char* second_arg = chop;
    //printf("%s",second_arg);
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
      } else if (pid_1 == 0){
        dup2(fd_1[1], STDOUT_FILENO);
        close(fd_1[1]);
        close(fd_1[0]);

        if((execl(BASH_EXEC, BASH_EXEC, "-c", first_arg, (char*) 0))<0) {
    		fprintf(stderr, "\nError executing grep. ERROR#%d\n", errno);
        }
      }
      //might need to declare this above...
      close(fd_1[1]);
      //support multiple processes, open up another pipe here
      pid_2 = fork();
      if(pid_2 == -1)
      {
        perror("fork");
        exit(EXIT_FAILURE);
      } else if(pid_2 == 0) {
        dup2(fd_1[0], STDIN_FILENO);
        close(fd_1[0]);
        //parse_command(second_arg);

        if((execl(BASH_EXEC, BASH_EXEC, "-c", second_arg, (char*) 0))<0) {
    		fprintf(stderr, "\nError executing grep. ERROR#%d\n", errno);
        }
      }
      close(fd_1[0]);
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
    if(!strcmp(cmds[ind-1], "&"))
    {
      //do a check for valid command

      //set global background variable to true
      bg = true;
      printf("in the background conditional!");
      //remove the back of cmds array, this might not work, try to set to null
      cmds[ind-1] = NULL;

      //run whatever command was inside in process...

    }
    //printf("\n first string in command %s", cmds[0]);
    if(!strcmp(cmds[0], "set")){

    }else if(!strcmp(cmds[0], "echo")){
      /*
      char path[100];
      int i = 1;
      if(cmds[i]==NULL){
        path = "";
      } else {
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
      }*/
    }else if(!strcmp(cmds[0], "cd")){
      char path[100];
      int i = 1;
      printf("in cd, second argument: %s\n", cmds[1]);
      if(cmds[i]==NULL){
        sprintf(path, "%s", getenv("HOME"));
      } else {
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
      }
      printf("path: %s\n", path);
      change_directory(path);
    }else if(!strcmp(cmds[0], "pwd")){
      if(cmds[1]==NULL)
      {
        print_working_directory();
      } else {
        execvp_commands(cmds, bg);
      }
    }else if(!strcmp(cmds[0], "quit")){
      terminate();
    }else if(!strcmp(cmds[0], "exit")){
      terminate();
    }else if(!strcmp(cmds[0], "jobs")){
      printf("we have %d jobs. printing now:\n", jc);
      for(int i = 0; i < jc; i++){
        struct Job* temp = jobs[jc];
        printf("[%d] %d       %s\n", jc, temp->pid, temp->command);
      }
    }else if(!strcmp(cmds[0], "")){
      //just want this to continue while doing nothing...
    } else {
      printf("we are here!\n");
      printf("makign call to system with argument %s\n", cmds[0]);
      pid_t pid = fork();
      if (pid == 0){
      execvp_commands(cmds, bg);
      }
    }
  }
}


void execvp_commands(char** cmds, bool bg)
{
  if(bg){
    //add job
    printf("\nadding job!\n");
    char cmd[100];
    int i = 0;

    //this doesnt work! fuck! not conjoining the strings correctly.
    while(cmds[i]!=NULL)
    {
      char* temp = cmds[i];
      if(cmds[i+1]!=NULL)
      {
        strcat(temp, " ");
        strcat(cmd, temp);
      }
      strcat(cmd, temp);
      i++;
    }
    struct Job* job = malloc(sizeof *job);
    job->command = cmd;
    printf("new job command: %s\n", job->command);

    printf("in execvp bg, background variable = %d\n", bg);
    bg = false;
    pid_t mpid = fork();
    //printf("pid: %d\n", getpid());
    job->pid = getpid();
    printf("new job pid: %d\n", job->pid);
    add_job(job);
    if (mpid == -1){
      perror("fork");
      exit(EXIT_FAILURE);
    } else if (mpid == 0) {
      printf("executing child in background\n");
      setpgid(0,0);
      if((execvp(cmds[0], cmds))<0){
        fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmds[0], errno);
      }
      //remove_job(job);
    } else {
      waitpid(-1, &status, WNOHANG);
      exit(0);
    }

  } else {
    pid_t mpid = fork();
    printf("pid: %d", getpid());
    if (mpid == -1){
      perror("fork");
      exit(EXIT_FAILURE);
    } if(mpid == 0){
      printf("executing with execvp\n");
      if((execvp(cmds[0], cmds))<0){
        fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmds[0], errno);
      }
    } else {
      waitpid(mpid, &status, 0);
      printf("executing in parent process\n");
      exit(0);
    }
  }
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

//Jamie asking Dr. Yun
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

  //check main argv to see if there are additional arguments to quash like a file to read commands from
  // Main execution loop
  if(strcmp(argv[1], "<")){
    
  }
  while (is_running() && get_command(&cmd, stdin)) {
    parse_command(cmd.cmdstr);
   }//end while

  return EXIT_SUCCESS;
};
