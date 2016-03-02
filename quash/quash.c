/**
 * @file quash.c
 * @authors Cole Jurden, Evan Nichols
 * @purpose main implementation file for Quash
 * @date 3-1-2016
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**************************************************************************
 * Global Variables
 **************************************************************************/
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
int jc = 0;
static job_t jobs[50];

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

void parse_command(char* cmd){
  bool bg = false;
  char* outc[100], outbg[100];
  strcpy(outc, cmd);
  strcpy(outbg, cmd);

  //CHECK FOR PIPE
  char* pch;
  char* ich;
  char* bch;
  char* och;
  char* bcmd[100];
  strcpy(bcmd, cmd);
  bch = strchr(cmd, '&');
  pch = strchr(cmd,'|');
  ich = strchr(cmd, '<');
  och = strchr(cmd, '>');

  if(pch!=NULL){
    /*
    * pipe character found.
    * use strtok to chop the strings
    */
    char* chop = strtok(cmd, "|\0");
    char* first_arg = chop;
    chop = strtok(NULL, "\0");
    char* second_arg = chop;

    //start pipe process.
    int fd_1[2];
    pid_t pid_1, pid_2;

    //instantiate pipes
    if (pipe(fd_1) == -1)
    {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

    pid_1 = fork();

    if(pid_1 == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid_1 == 0){
        dup2(fd_1[1], STDOUT_FILENO);
        close(fd_1[1]);
        close(fd_1[0]);

        if((execl(BASH_EXEC, BASH_EXEC, "-c", first_arg, (char*) 0))<0) {
    		fprintf(stderr, "\nError executing grep. ERROR#%d\n", errno);
        }
    }
    close(fd_1[1]);

    //open up second pipe
    pid_2 = fork();

    if(pid_2 == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid_2 == 0){
        dup2(fd_1[0], STDIN_FILENO);
        close(fd_1[0]);
        //parse_command(second_arg);

        if((execl(BASH_EXEC, BASH_EXEC, "-c", second_arg, (char*) 0))<0) {
    		fprintf(stderr, "\nError executing grep. ERROR#%d\n", errno);
        }
    }
    close(fd_1[0]);
  }//END PIPE CHECK

  else{
    //No pipe found. Continue with other checks.
    char* ptr;
    char* cmds[100] = {NULL};
    char* tempCmd[100];
    strcpy(tempCmd, cmd);
    ptr = strtok(tempCmd, " ");
    int ind = 0;

    while(ptr != NULL && (strcmp(ptr, "&") != 0)){
      cmds[ind] = ptr;
      ptr = strtok(NULL, " ");
      ind++;
    }

    //Check for a background command, denoted by "&".
    if(bch!=NULL){
      //do a check for valid command
      bg = true;
      char* temp;
      temp = strtok(bcmd, "&");
      pid_t mpid = fork();
      if (mpid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      else if (mpid == 0){
        printf("executing child in background\n");
        printf("\nadding job!\n");
        job_t job = (job_t){.pid = getpid(), .command=cmds[0]};
        jobs[jc] = job;
        jc = jc+1;
        printf("[%d] %d\n", jc-1, jobs[jc-1].pid);
        parse_command(temp);
        printf("finished executing %d in background\n", getpid());
      }
    }//END ELSE

    //Check for stdin redirection, denoted by "<".
    else if(ich!=NULL){
      int ind = -1;
      int i = 0;
      char buffer[MAX_BUFFER];
      while(cmds[i]!=NULL){
        if(!strcmp(cmds[i], "<")){
          ind = i;
        }
      }
      int in = open(cmds[ind+1], O_RDONLY);
      int size = read(in, buffer, MAX_BUFFER);
      dup2(in, STDIN_FILENO);
      close(in);
    }//END stdin check

    //Check for stdout redirection, denoted by ">".
    if(och!=NULL){
      int ind = -1;
      int i = 0;
      while(cmds[i]!=NULL){
        if(!strcmp(cmds[i], ">")){
          ind = i;
        }
      }
      pid_t pid = fork();

      if(pid == 0){
        char* wrcmd = strtok(outc, ">");
        printf("%s", wrcmd);
        int out = open(cmds[ind+1], O_WRONLY | O_CREAT | O_APPEND);
        dup2(out, STDOUT_FILENO);
        parse_command(wrcmd);
        close(out);
      } else {
          waitpid(pid, &status, 0);
      }
    }//END stdout check

    else if(!strcmp(cmds[0], "set")){
        printf("SET HERE\n");
        //set_env_variable(cmds[1],cmds[2]);
    }
/*
    else if(!strcmp(cmds[0], "echo")){

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
      }

    }
*/
    else if(!strcmp(cmds[0], "cd")){
        printf("CHANGE DIRECTORY HERE");
        if((execvp(cmds[0], cmd[0]) < 0)){
            fprintf(stderr, "Error while executing %s. Error #%d.\n",cmds[0],errno);
        }
    }

      /*char path[100];
      //int i = 1;
      //printf("in cd, second argument: %s\n", cmds[1]);
      //if(cmds[i]==NULL){
        //sprintf(path, "%s", getenv("HOME"));
        //change_directory(path);
      } else {
        change_directory(cmds[1]);
    }*/

    else if(!strcmp(cmds[0], "pwd")){
      if(cmds[1]==NULL){
        print_working_directory();
      }
      else{
        execvp_commands(cmds);
      }
    }

    else if(!strcmp(cmds[0], "quit")){
      terminate();
    }

    else if(!strcmp(cmds[0], "exit")){
      terminate();
    }

    else if(!strcmp(cmds[0], "jobs")){
      print_jobs();
    }

    else if(!strcmp(cmds[0], "")){
      //just want this to continue while doing nothing...
    }

    else {
      if(bg) {
        bg = false;
          //exec_commands_bg(cmds);
      } else {
          execvp_commands(cmds);
      }
    }
  }//END NO PIPE FOUND (Ln 175)
}//END PARSE COMMAND (Ln 100)

void exec_commands_bg(char** cmds)
{

  pid_t mpid = fork();
  if (mpid == -1){
    perror("fork");
    exit(EXIT_FAILURE);

  } else if (mpid == 0) {
    //printf("executing child in background\n");
    if((execvp(cmds[0], cmds))<0){
      fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmds[0], errno);
    }
    printf("finished executing %d\n", getpid());
  } else {
    //add job
    printf("\nadding job!\n");
    job_t job = (job_t){.pid = getpid(), .command=cmds[0]};
    jobs[jc] = job;
    jc = jc+1;
    printf("[%d] %d\n", jc-1, jobs[jc-1].pid);
    while(waitpid(-1, NULL, WNOHANG) > 0){
      printf("[%d] Done", getpid());
    }
    //exit(0);
  }
}

void check_jobs(){
  for(int i = 0; i < jc; i++){
    if(waitpid(jobs[i].pid, &status, WNOHANG) > 0)
    {
      printf("[%d] Done            %s\n\n", i, jobs[i].command);
      //remove_job(jobs[jc]);
    }
  }
}

void execvp_commands(char** cmds)
{
  pid_t mpid = fork();
  if (mpid == -1){
    perror("fork");
    exit(EXIT_FAILURE);
  } if(mpid == 0){
    //printf("executing with execvp\n");
    if((execvp(cmds[0], cmds))<0){
      fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmds[0], errno);
    }
    printf("done executing %d", getpid());
  } else {
    waitpid(mpid, &status, 0);
    check_jobs();
    //printf("back in parent process\n");
    //exit(0);
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
}

void print_working_directory(){
  char cwd[MAX_BUFFER];
  if(getcwd(cwd, sizeof(cwd)) != NULL){
    fprintf(stdout, "Current working directory: %s\n", cwd);
  }
  else{
    perror("getcwd() error");
  }
}

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
}

void print_jobs(){
  printf("we have %d jobs. printing now:\n", jc);
  for(int i = 0; i < jc; i++){
    job_t temp = jobs[i];
    printf("[%d] %d       %s\n", jc, temp.pid, temp.command);
  }
}

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
  char* cmdbuf[30]; //< array holding individual commands to be read in
  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");

  /*
  * Check argv to see if an input file was passed to run with Quash.
  */
  if(argv[1] != NULL){
    if(strcmp(argv[1], "<")){
      printf("recognized command file. filename: %s\n", argv[2]);
      FILE* in = fopen(argv[2], "r");

      if(in == 0){
        printf("error!");
        exit(1);
      }

      int ind = 0;
      while(ind < MAX_BUFFER && fgets(cmdbuf[ind], 100, in)){
        //replace newline with null
        cmdbuf[ind][(int)strlen(cmdbuf[ind]-1)] = "\0";
        ind++;
      }

      fclose(in);
      //read and parse commands
      for(int i = 0; i < ind; i++){
        parse_command(cmdbuf[i]);
      }
    }
   }//end input file checks
   else{
       //Display intial prompt
       printf("\033[1;36m %s>\033[0m", getcwd(NULL, 0));
   }

  while (is_running() && get_command(&cmd, stdin)) {
    //print prompt to user
    printf("\033[1;36m %s>\033[0m", getcwd(NULL, 0));
    parse_command(cmd.cmdstr);
   }

  return EXIT_SUCCESS;
};
