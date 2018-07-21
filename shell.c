/*
 * Filename: /home/nilanjan/Desktop/cs744/lab1/shell.c
 * Path: /home/nilanjan/Desktop/cs744/lab1
 * Created Date: Thursday, July 19th 2018, 4:10:33 pm
 * Author: nilanjandaw
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "definitions.h"
#include "support.h"

const char* SHELL_IMPLEMENTED_COMMANDS[] = {"cd", "history"};
const int (*SHELL_IMPLEMENTED_COMMANDS_POINTER[2]) (const char **);
pid_t background_process_list[MAX_PROCESS_LIST], foreground_process_list[MAX_PROCESS_LIST];
int kill_count = 0;
char **tokenize(char *line) {
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
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

int check_shell_implementation(char *command) {
  int implementation_count = sizeof(SHELL_IMPLEMENTED_COMMANDS)/sizeof(SHELL_IMPLEMENTED_COMMANDS[0]);
  for(size_t i = 0; i < implementation_count; i++) {
    if (strcmp(SHELL_IMPLEMENTED_COMMANDS[i], command) == 0)
      return i;
  }
  return -1;
}

void background_process_reaper() {
  pid_t pid;
  int status;
  for (int i = 0; i < MAX_PROCESS_LIST; i++) {
    pid = waitpid(-1, &status, WNOHANG);
    //pid = wait(NULL);
    if (pid == -1) {
      break;
    }
    if (pid != 0 && pid != -1)
      remove_from_list(background_process_list, pid, MAX_PROCESS_LIST, BACKGROUND_PROCESS);
  }
}

void foreground_process_reaper() {
  pid_t pid;
  int status;
  for(int i = 0; i < MAX_PROCESS_LIST; i++) {
    if (foreground_process_list[i] != 0) {
      pid = waitpid(foreground_process_list[i], &status, 0);
      printf("foregroud process reaped pid: %d\n", pid);
      remove_from_list(foreground_process_list, pid, MAX_PROCESS_LIST, FOREGROUND_PROCESS);
    }
  }
  
}


void signal_handler(int signal) {
  if (signal == SIGCHLD)
    background_process_reaper();
  if (signal == SIGINT) {
    for(int i = 0; i < MAX_PROCESS_LIST; i++) {
      if (foreground_process_list[i] != 0) {
        kill(foreground_process_list[i], SIGTERM);
      }
    }

    if (kill_count == 1)
      kill(getpid(), SIGTERM);
    kill_count++;
  }
}

int start_process(char **tokens, int type) {
  pid_t pid = fork();
  if (type == FOREGROUND_PROCESS)
    insert_into_list(foreground_process_list, pid, MAX_PROCESS_LIST, type);
  else
    insert_into_list(background_process_list, pid, MAX_PROCESS_LIST, type);
  if (pid < 0) {
    fprintf(stderr, "Error Could not fork new process.\n");
    return -1; // if error in creating child process return immediately with error code -1
  }

  if (pid == 0) {
    int return_code = execvp(tokens[0], tokens);
    int implementation_code = check_shell_implementation(tokens[0]);

    if (return_code < 0 && implementation_code >= 0) {
      return_code = SHELL_IMPLEMENTED_COMMANDS_POINTER[implementation_code] ((const char **)tokens);
      
    }
    if (return_code < 0) {
      printf("error in running command\n");
    }
  } else {
      //foreground_process_reaper();
    }
  
  return pid;
}

int check_process_type(char **tokens, int start, int end) {
  int last_token_index = 0;
  for(int i = 0; tokens[i] != NULL; i++) {
    last_token_index = i;
  }
  if (last_token_index >= 0) {
    char *last_token = tokens[last_token_index];
    if (last_token[strlen(last_token) - 1] == '&') {
      last_token[strlen(last_token) - 1] = '\0';
      if (strlen(last_token) == 0)
        tokens[last_token_index] = NULL;
      return BACKGROUND_PROCESS;
    }
    return FOREGROUND_PROCESS;
  }
  return -1;
}

int main(int argc, char const *argv[])
{
  char  line[MAX_INPUT_SIZE];            
  char  **tokens;              
  int i;
  
  SHELL_IMPLEMENTED_COMMANDS_POINTER[0] = cd;
  SHELL_IMPLEMENTED_COMMANDS_POINTER[1] = history;
  signal(SIGCHLD, signal_handler);
  signal(SIGINT, signal_handler);
  while (1) {           
    FILE *history_keeper = fopen(HISTORY_KEEPER_PATH, "a");
    printf("Hello>");     
    bzero(line, MAX_INPUT_SIZE);
    fgets(line, MAX_INPUT_SIZE, stdin);  
    fprintf(history_keeper, "%s", line);
    kill_count = 0;
    line[strlen(line)] = '\n'; 
    tokens = tokenize(line);
    int type = check_process_type(tokens, 0, 0);
    printf("type %d\n", type);
    int pid = start_process(tokens, type);
    if (pid > 0 && type == FOREGROUND_PROCESS) {
      printf("foreground process\n");
      foreground_process_reaper();
    }
    // Freeing the allocated memory	
    for(i=0;tokens[i]!=NULL;i++){
        free(tokens[i]);
    }
    free(tokens);
    fclose(history_keeper);
  }

  
  printf("closed history");
  return 0;
}

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

int history(const char **tokens) {
  printf("history\n");
  int c;
  FILE *history_keeper;
  history_keeper = fopen(HISTORY_KEEPER_PATH, "r");
  if (history_keeper) {
      while ((c = getc(history_keeper)) != EOF)
          putchar(c);
      fclose(history_keeper);
  }

  return 0;
}