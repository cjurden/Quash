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
#define MAX_BUFFER 1024
#define VALID_COMMAND_LENGTH 7
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

    return true;
  }
  else
    return false;
}

//returns number of commands
int read_command(command_t* cmd, char** cmdbuf){
  char* cstring = strtok(cmd->cmdstr, " ");
  int len = 0;
  //read cstring, break it up and go through each until you reach null.
  while(cstring != NULL){
    cmdbuf[len] = cstring;
    if(cstring == "|"){
      //PIPE_FLAG = 1;
    }
    cstring = strtok(NULL, " ");
    len++;
  }
  return len;
}

void exec_command(char** cmdbuf, int len){
  //search global variable containing execs
  bool p = check_for_pipe(cmdbuf);
  if(p != -1){
    int i = 0;
    while(cmdbuf[i] != "|"){
        argc
    }
    }
  } else {
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
      }
    }
  }

  //if not there, check path

}

void store_commands_before_pipe(char** cmdbuf, int piploc){
  for(int i = 0; i < piploc-1; i++)
}

void exec_command_with_pipe {

}

//see if pipes are in command line
int check_for_pipe(char** cmdbuf){
  for(int i = 0; i < 100; i++){
    if(cmdbuf[i] == "|"){
      return i;
    }
  }
  return -1;
}

bool in_cmd_set(char* input)
{

}
/**
* CHANGE_DIRECTORY will invoke the chdir() UNIX system call to
* change the working directory to that specified in path.
*/
void change_directory(const char* path) {

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
  char cmdbuf[MAX_BUFFER]; //< array holding individual commands
  for(int i = 0; i < 100; i++){
    cmdbuf[i] = NULL;
  }

  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.

    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit"))
      terminate(); // Exit Quash
    else
      puts(cmd.cmdstr); // Echo the input string
  }

  return EXIT_SUCCESS;
}
