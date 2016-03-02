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
static bool opt;
//for use with wait PID
static int status;
static char* HOME_ENV;
static char* PATH_ENV;

//to keeep track of background and foreground processes...
int jc = 0;
static struct job_t jobs[50];

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
      //printf("%s\n", cmd->cmdstr);
    return true;
  }
  else
    return false;
}

/*
* mand takes the cmdstr and acts accordingly
* 3 cases for pipes, iniitial command that requires one std_out, middle case that requires in and out, end case that requires in
* keep track of each of those pipes
* can resuse arrays ,use 2 fd_1[2]s, kep track of beginning and end...
* cmd1[], cmd2[], cmdall[][] where cmd all holds all commands, and removes the front one after execution.
* keep track of PID
*/

void parse_command(char* cmd){
  bool bg = false;
  char* outbg[100];
  char* outc = strdup(cmd);
  char* inc = strdup(cmd);
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
    //char* cmds = create_cmd_arr(cmd);
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
      } else if (mpid == 0) {
        printf("executing child in background\n");
        printf("\nadding job!\n");
        int jobid = jc;
        parse_command(temp);
        printf("[%d] %d  finished COMMAND \n", jobid, getpid());

      } else {
        //parent process of mpid
        jobs[jc].pid = mpid;
        jobs[jc].command=cmds[0];
        printf("[%d] %d   %s\n", jc, jobs[jc].pid, jobs[jc].command);
        jc++;
        if(waitpid(mpid,&status,WNOHANG) > 0)
        {
          printf("[%d] Done          ", getpid());
        }
      }
    }//END ELSE

    //Check for stdin redirection, denoted by "<".
    else if(ich!=NULL){
      //opt = true;
      int ind = -1;
      int i = 0;
      char buffer[MAX_BUFFER];
      while(cmds[i]!=NULL){
        if(!strcmp(cmds[i], "<")){
          ind = i;
        }
        i++;
      }
      char* wrcmd;
      wrcmd = strtok(inc, "<");
      int in = open(cmds[ind+1], O_RDONLY);
      //int size = read(in, buffer, MAX_BUFFER);
      dup2(in, STDIN_FILENO);
      close(in);
      parse_command(wrcmd);
      /*
      pid_t pid = fork();
      if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
      } else if (pid == 0){
        int in = open(cmds[ind+1], O_RDONLY);
        //int size = read(in, buffer, MAX_BUFFER);
        dup2(in, STDIN_FILENO);
        close(in);
        parse_command(wrcmd);
      } else {
        waitpid(pid, &status, 0);
      }*/
    }//END stdin check

    //Check for stdout redirection, denoted by ">".
    if(och!=NULL){
      opt = true;
      int ind = -1;
      int i = 0;
      while(cmds[i]!=NULL){
        if(!strcmp(cmds[i], ">")){
          ind = i;
        }
        i++;
      }
        pid_t pid = fork();
        if(pid == -1){
          perror("fork");
          exit(EXIT_FAILURE);
        } else if (pid == 0){
          printf("output\n");
          char* wrcmd;
          wrcmd = strtok(outc, ">");
          int out = open(cmds[ind+1], O_WRONLY | O_CREAT | O_APPEND);
          dup2(out, STDOUT_FILENO);
          close(out);
          parse_command(wrcmd);
        } else {
          waitpid(pid, &status, 0);
        }

    }//END stdout check

    else if(!strcmp(cmds[0], "set")){

        char* dest = strtok(cmds[1],"=");
        char* path = strtok(NULL,"=");
        printf("%s\n",dest);
        printf("%s\n",path);

        if(!strcmp(dest,"HOME")){
            strcpy(HOME_ENV, path);
            puts(HOME_ENV);
            //set home variable
            set_env_variable("HOME",HOME_ENV);
        }
        else if(!strcmp(dest,"PATH")){
            //set path variable
            strcpy(PATH_ENV,path);
            puts(PATH_ENV);
            set_env_variable("PATH",PATH_ENV);
        }
        else{
            fprintf(stderr, "no such variable to set. Error#%d\n",errno);
        }
    }

        else if(!strcmp(cmds[0], "cd")){

        if(cmds[1] != NULL){
            //means users has passed an actual directory
            if(chdir(cmds[1]) < 0){
              puts("Error. Invalid directory.");
            }
        }
        else {
            //user passed only "cd." change to directory given by $HOME variable.
            chdir(getenv("HOME"));
        }
    }

    else if(!strcmp(cmds[0], "pwd")){
        char cwd[MAX_BUFFER];
        if(getcwd(cwd, sizeof(cwd)) != NULL){
          fprintf(stdout, "\n%s\n", cwd);
        }
        else{
          perror("getcwd() error");
        }
    }

    else if(!strcmp(cmds[0],"echo")){

        if(strcmp(cmds[1], "$PATH") == 0){
            //echo $PATH var
            fprintf(stdout,"PATH: %s\n",PATH_ENV);
        }
        else if(strcmp(cmds[1], "$HOME") == 0){
            //echo $HOME var
            fprintf(stdout,"HOME: %s\n",HOME_ENV);
        }
        else{
            int i = 1;
            while(cmds[i] != NULL){
                printf("%s ",cmds[i]);
                i++;
            }
            printf("\n");
        }
    }

    else if(!strcmp(cmds[0], "quit") || !strcmp(cmds[0], "exit")){
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
      } else if (opt){
        opt = false;
        if((execvp(cmds[0], cmds))<0){
          fprintf(stderr, "\nError executing %s. ERROR#%d\n", cmds[0], errno);
        }
      } else {
          execvp_commands(cmds);
      }
    }
  }//END NO PIPE FOUND (Ln 175)
}//END PARSE COMMAND (Ln 100)


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
    printf("done executing %d\n", getpid());
  } else {
    waitpid(mpid, &status, 0);
    check_jobs();
    //printf("back in parent process\n");
    //exit(0);
  }
}

void execute_echo(const char* path_to_echo){

}

void print_jobs(){
  printf("we have %d jobs. printing now:\n", jc);
  for(int i = 0; i<jc; i++){
    //job_t temp = jobs[i];
    printf("[%d] %d       %s\n", i, jobs[i].pid, jobs[i].command);
  }
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
  char* cmdbuf[30]; //< array holding individual commands to be read in
  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");
  printf("\033[1;36m %s>\033[0m", getcwd(NULL, 0));

  //save env variables initially
  HOME_ENV = getenv("HOME");
  PATH_ENV = getenv("PATH");

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
   }

  while (is_running() && get_command(&cmd, stdin)) {
    //print prompt to user
    parse_command(cmd.cmdstr);
    printf("\033[1;36m %s>\033[0m", getcwd(NULL, 0));
   }

  return EXIT_SUCCESS;
};
