/**
 * @file quash.h
 *
 * Quash essential functions and structures.
 */

#ifndef QUASH_H
#define QUASH_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

/**
 * Specify the maximum number of characters accepted by the command string
 */
#define MAX_COMMAND_LENGTH (1024)

/**
 * Holds information about a command.
 */
typedef struct command_t {
  char cmdstr[MAX_COMMAND_LENGTH]; ///< character buffer to store the
                                   ///< command string. You may want
                                   ///< to modify this to accept
                                   ///< arbitrarily long strings for
                                   ///< robustness.
  size_t cmdlen;                   ///< length of the cmdstr character buffer

  // Extend with more fields if needed
} command_t;


/**
 * Starts the main loop by setting running flag to true.
 */
static void start();

/**
 * Query if quash should accept more input or not.
 *
 * @return True if Quash should accept more input and false otherwise
 */
bool is_running();

/**
 * Causes the execution loop to end.
 */
void terminate();

/**
 *  Read in a command and setup the #command_t struct. Also perform some minor
 *  modifications to the string to remove trailing newline characters.
 *
 *  @param cmd - a command_t structure. The #command_t.cmdstr and
 *               #command_t.cmdlen fields will be modified
 *  @param in - an open file ready for reading
 *  @return True if able to fill #command_t.cmdstr and false otherwise
 */
bool get_command(command_t* cmd, FILE* in);

int read_command(command_t* cmd, char** cmdbuf);
void exec_command(char** cmdbuf, int len);
void store_commands_before_pipe(char** cmdbuf, int piploc);
void exec_command_with_pipe();
int check_for_pipe(char** cmdbuf);
bool in_cmd_set(char* input);

/**
 * Change the working directory to the path specified.
 * Uses the UNIX cwd() command.
 * @param path - a string indicating the desired path.
 */
void change_directory(const char* path);

/**
 * Print the working directory to the terminal.
 * Uses the UNIX pwd() command. No params, no return.
 */
void print_working_directory();

 /**
  * Print the content of the PATH and HOME environment variables to
  * the terminal.
  * Uses the UNIX pwd() command. No params, no return.
  */
void execute_echo(const char* path_to_echo);

/**
 * Print all of the crrently running background processess in the format:
 * [JOBID] PID COMMAND, where JOBID is the unique positive integer quash
 * assigns to a job to identify it, PID is the PID of the child process
 * used for the job and COMMAND is the command used to invoke the job.
 *
 * No params, no return
 */
void print_background_processes();




#endif // QUASH_H
