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
      remove_from_list(foreground_process_list, pid, MAX_PROCESS_LIST, FOREGROUND_PROCESS);
    }
  }
  
}


void signal_handler(int signal) {
  if (signal == SIGCHLD) {
    background_process_reaper();
  }
  if (signal == SIGINT) {
    for(int i = 0; i < MAX_PROCESS_LIST; i++) {
      if (foreground_process_list[i] != 0) {
        kill(foreground_process_list[i], SIGTERM);
      }
    }
  }
}

void clean_up_resources() {
  printf("\nCleaning up processes\n");
  for(int i = 0; i < MAX_PROCESS_LIST; i++) {
    if (background_process_list[i] != 0) {
      kill(background_process_list[i], SIGTERM);
      pid_t pid = waitpid(background_process_list[i], NULL, 0);
      if (pid != 0 && pid != -1)
        remove_from_list(background_process_list, pid, MAX_PROCESS_LIST, BACKGROUND_PROCESS);
    }
  }
  printf("exit shell\n");
}

int start_process(char **tokens, int type) {
  int implementation_code = check_shell_implementation(tokens[0]);
  if (implementation_code == 0) {
    int return_code;
    return_code = SHELL_IMPLEMENTED_COMMANDS_POINTER[implementation_code] ((const char **)tokens);
    if (return_code < 0) {
        printf("error in running command\n");
    }
    return 0;
  } else {
    pid_t pid = fork();
    setpgid(pid, 0);
    if (type == FOREGROUND_PROCESS)
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
        return_code = SHELL_IMPLEMENTED_COMMANDS_POINTER[implementation_code] ((const char **)tokens);
        
      } else {
        return_code = execvp(tokens[0], tokens);
      }
      if (return_code < 0) {
        printf("error in running command\n");
      }
    }
    return pid;
  }
}

int main(int argc, char const *argv[])
{
  char line[MAX_INPUT_SIZE];            
  char **tokens;              
  int i;
  int process_execution_type = SEQUENTIAL_PROCESS;
  SHELL_IMPLEMENTED_COMMANDS_POINTER[0] = cd;
  SHELL_IMPLEMENTED_COMMANDS_POINTER[1] = history;
  signal(SIGCHLD, signal_handler);
  signal(SIGINT, signal_handler);
  while (1) {        
    int checkpoint = 0; char **process;   
    FILE *history_keeper = fopen(HISTORY_KEEPER_PATH, "a");
    printf("Hello>");     
    bzero(line, MAX_INPUT_SIZE);
    fgets(line, MAX_INPUT_SIZE, stdin); 
    if (strlen(line) == 1)
      continue;
    fprintf(history_keeper, "%s", line);
    line[strlen(line)] = '\n'; 
    tokens = tokenize(line);
    if (strcmp(tokens[0], "exit") == 0) {
      clean_up_resources();
      break;
    }

    process = get_process_delimiters(tokens, checkpoint, &process_execution_type, &checkpoint);
    if (process != NULL) {
      // printf("%s", process[0]);
      int type = check_process_type(process);
      // printf("type %d endpoint %d execution_type %d\n", type, checkpoint, process_execution_type);
      // printf("checkpoint %d\n", checkpoint);
      // for(size_t i = 0; process[i] != NULL; i++) {
      //   printf(" args %s ", process[i]);
      // }
      
      int pid = start_process(process, type);
      if (pid > 0 && type == FOREGROUND_PROCESS) {
        printf("foreground process\n");
        foreground_process_reaper();
      }
    }
    // Freeing the allocated memory	
    for(i=0;tokens[i]!=NULL;i++){
        free(tokens[i]);
    }
    if (tokens != NULL)
      free(tokens);

    if (process != NULL) {
      for (i = 0; process[i] != NULL; i++) {
        free(process[i]);
      }
      free(process);  
    }
    fclose(history_keeper);
  }
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