/**
 * @Author: nilanjan
 * @Date:   2018-07-29T19:08:52+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: shell.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-08-09T18:39:44+05:30
 * @Copyright: Nilanjan Daw
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "definitions.h"
#include "support.h"

const char* SHELL_IMPLEMENTED_COMMANDS[] = {"cd", "history"};
const int (*SHELL_IMPLEMENTED_COMMANDS_POINTER[2]) (const char **);
bool exit_requested = false;
pid_t background_process_list[MAX_PROCESS_LIST], foreground_process_list[MAX_PROCESS_LIST];

/**
* tokenizer function to tokenize the given input into sub commands
**/
char **tokenize(char *line) {
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t') {

      token[tokenIndex] = '\0';
      if (tokenIndex != 0) {

        tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
        strcpy(tokens[tokenNo++], token);
        tokenIndex = 0;
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

/**
 * check if a shell command has been custom implemeted
**/
int check_shell_implementation(char *command) {

  int implementation_count = sizeof(SHELL_IMPLEMENTED_COMMANDS)/sizeof(SHELL_IMPLEMENTED_COMMANDS[0]);
  for(size_t i = 0; i < implementation_count; i++) {
    /**
     * check with list of available shell commands
    **/
    if (strcmp(SHELL_IMPLEMENTED_COMMANDS[i], command) == 0)
      return i;
  }
  return -1;
}

/**
 * reaps background precesses that were running on their termination
**/
void background_process_reaper() {
  pid_t pid;
  int status;
  for (int i = 0; i < MAX_PROCESS_LIST; i++) {

    pid = waitpid(-1, &status, WNOHANG); // reaping the process without blocking
    if (pid == -1) {
      break;
    }
    if (pid != 0 && pid != -1) {
      /**
       * on sucessful process reap, remove the process from the background process list
      **/
      int status = remove_from_list(background_process_list, pid, MAX_PROCESS_LIST, BACKGROUND_PROCESS);
      if (status == -1)
      /**
       * if the process reaped was not part of the background processes running, trying looking up
       * the foreground process list and if found remove it from the list
      **/
        remove_from_list(foreground_process_list, pid, MAX_PROCESS_LIST, FOREGROUND_PROCESS);
    }
  }
}

/**
 * foreground process reaper reaps foreground processes with a blocking system call
 * and waits (blocks) until all the foreground processes have died
 * and has been successfully reaped
**/
void foreground_process_reaper() {
  pid_t pid;
  int status;
  for(int i = 0; i < MAX_PROCESS_LIST; i++) {

    if (foreground_process_list[i] != 0) {
      /**
       * on sucessful process reap, remove the process from the foreground process list
      **/
      pid = waitpid(foreground_process_list[i], &status, 0);
      remove_from_list(foreground_process_list, pid, MAX_PROCESS_LIST, FOREGROUND_PROCESS);
    }
  }

}

/**
 * handles the SIGCHLD and the SIGINT system signals
**/
void signal_handler(int signal) {
  if (signal == SIGCHLD) {       // when a SIGCHLD signal is received, i.e a child has died
    background_process_reaper(); // the background process reaper is called to reap the dead child
                                 // and also any zombie processes left behind
  }
  if (signal == SIGINT) {

    /**
     * on receiving a SIGINT a kill command is sent to all the running foreground processes
     * with a SIGTERM flag
    **/
    pid_t pid;
    for(int i = 0; i < MAX_PROCESS_LIST; i++) {

      if (foreground_process_list[i] != 0) {

        kill(foreground_process_list[i], SIGTERM); // for each process in the foreground list send a SIGTERM
      }
    }
    exit_requested = true; // set the exit_requested flag
  }
}

/**
 * function to clean up any left out resources or running processes before shell exits
**/
void clean_up_resources() {

  printf("\nCleaning up processes\n");
  for(int i = 0; i < MAX_PROCESS_LIST; i++) {

    if (background_process_list[i] != 0) {
      /**
       * kill any running background processes
      **/
      kill(background_process_list[i], SIGTERM);
      pid_t pid = waitpid(background_process_list[i], NULL, 0); // reap background processes just killed
      if (pid != 0 && pid != -1)
        remove_from_list(background_process_list, pid, MAX_PROCESS_LIST, BACKGROUND_PROCESS);
    }
  }
  printf("exit shell\n");
}

/**
 * primary function to fork() child processes
 * determine type of process and execute processes as required
**/
int start_process(char **tokens, int type) {

  int implementation_code = check_shell_implementation(tokens[0]);
  if (implementation_code == 0) {
    // if command has been custom implemeted run accordingly
    int return_code;
    return_code = SHELL_IMPLEMENTED_COMMANDS_POINTER[implementation_code] ((const char **)tokens);
    if (return_code < 0) {
        printf("error in running command\n");
    }
    return 0;
  } else {

    pid_t pid = fork(); // fork a child process
    setpgid(pid, 0); // set the Group ID of the child process to the Process ID
    if (type == FOREGROUND_PROCESS) // insert the process ID into the process list according to their type
      insert_into_list(foreground_process_list, pid, MAX_PROCESS_LIST, type);
    else
      insert_into_list(background_process_list, pid, MAX_PROCESS_LIST, type);
    if (pid < 0) {
      fprintf(stderr, "Error Could not fork new process.\n");
      return -1; // if error in creating child process return immediately with error code -1
    }

    if (pid == 0) {
      int return_code;
      if (implementation_code >= 0) {
        // if process is shell implemented call the required function
        return_code = SHELL_IMPLEMENTED_COMMANDS_POINTER[implementation_code] ((const char **)tokens);

      } else {
        return_code = execvp(tokens[0], tokens); // execute the process with the arguments passed by the user
      }
      if (return_code < 0) {
        printf("error in running command\n");
        kill(pid, SIGTERM);
      }
    }
    return pid;
  }
}

/**
 * the main() function takes in user inputs and starts process execution accordingly
**/
int main(int argc, char const *argv[]) {

  char line[MAX_INPUT_SIZE];
  char **tokens;
  int i;

  SHELL_IMPLEMENTED_COMMANDS_POINTER[0] = cd;
  SHELL_IMPLEMENTED_COMMANDS_POINTER[1] = history; // shell implemented process functions

  // signal handlers
  signal(SIGCHLD, signal_handler);
  signal(SIGINT, signal_handler);

  while (1) {

    int checkpoint = 0;
    int process_execution_type = SEQUENTIAL_PROCESS;
    int type = -1;
    FILE *history_keeper = fopen(HISTORY_KEEPER_PATH, "a"); // open a command history keeper
    printf("Hello>");
    bzero(line, MAX_INPUT_SIZE);
    fgets(line, MAX_INPUT_SIZE, stdin);

    if (strlen(line) == 1)
      continue; // if user input was empty continue

    fprintf(history_keeper, "%s", line); // enter command in the history logs
    line[strlen(line)] = '\n';
    tokens = tokenize(line);

    if (strcmp(tokens[0], "exit") == 0) {

      // if command was to "exit" clean up resources and break out of loop
      clean_up_resources();
      break;
    }
    /**
     * run until the entire process (consisting of multiple sub processes) has been executed
    **/
    while (checkpoint != -1) {

      if (exit_requested) {
        // if a Ctrl + C was pressed do a foreground process reap to clean up any zombie process if any
        foreground_process_reaper();
        break;
      }

      /**
       * tokenize the commands into sub commands if
       * command contains any sequential or parallel execution operators
      **/
      char **process;
      process = get_process_delimiters(tokens, checkpoint, &process_execution_type, &checkpoint);
      type = check_process_type(process);

      if (process != NULL) {

        int pid = start_process(process, type);
        if (pid > 0 && type == FOREGROUND_PROCESS && process_execution_type == SEQUENTIAL_PROCESS) {
          foreground_process_reaper(); // if sequential and foreground then wait for current process
        }
        for (i = 0; process[i] != NULL; i++) {
          free(process[i]); // free resources
        }
        free(process);
      }
    }

    if (type == FOREGROUND_PROCESS && process_execution_type == PARALLEL_PROCESS)
        foreground_process_reaper(); // if process is parallel, wait for all the process at the simultaneously

    // Freeing the allocated memory
    for(i=0;tokens[i]!=NULL;i++){
        free(tokens[i]);
    }
    if (tokens != NULL)
      free(tokens);

    fclose(history_keeper);
    exit_requested = false;
  }
  return EXIT_SUCCESS;
}

/**
 * implementing cd shell command using chdir()
*/
int cd (const char **tokens) {

  int size = 0;
  for (int i = 0; tokens[i] != NULL; i++)
    size++;
  if (size > 2) {
    printf("too many arguments\n");
    return -1;
  }
  if (size == 2)
    chdir(tokens[1]);
  else
    chdir(getenv("HOME"));
  return 0;
}

/**
 * implementing history command
**/
int history(const char **tokens) {

  int size = 0;
  for (int i = 0; tokens[i] != NULL; i++)
    size++;
  if (size > 1) {
    printf("too many arguments\n");
    return -1;
  }
  printf("history\n");
  int c;
  FILE *history_keeper;
  history_keeper = fopen(HISTORY_KEEPER_PATH, "r");
  if (history_keeper) {

      while ((c = getc(history_keeper)) != EOF)
          putchar(c);
      fclose(history_keeper);
  }
  _exit(EXIT_SUCCESS);
  return 0;
}
