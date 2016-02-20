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

void read_command(command_t* cmd, char** argc){
  char* cstring = strtok(cmd->cmdstr, " ");

  //read cstring, break it up and go through each until you reach null.
  while(cstring != NULL){
    g_array_append_val(argc, cstring);
    cstring = strtok(NULL, " ");
  }
}

void exec_command(char** argc){
  //search global variable containing execs

  //if not there, check path

}

bool in_cmd_set(char* input)
{

}

/**
* CHANGE_DIRECTORY will invoke the chdir() UNIX system call to
* change the working directory to that specified in path.
*/
void change_directory(const char* path) {
  int status = chdir(path);
  if(status < 0) {
    puts("Error. Invalid directory");
  }
  else {
    puts("Successfully changed to");
    puts(path);
  }
}

/**
* Print Working Directory (PWD) will invoke the getcwd() UNIX system
* call to print the current working directory to terminal
*/
void print_working_directory() {
  char cwd[MAX_BUFFER];
  if(getcwd(cwd, sizeof(cwd)) != NULL){
    fprintf(stdout, "Current Working Directory: %s\n", cwd);
  }
  else {
    perror("getcwd() error");
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
  char cmdbuf[MAX_BUFFER]; //< array holding individual commands

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
