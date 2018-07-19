#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

const char* SHELL_IMPLEMENTED_COMMANDS[] = {"cd", "history", "rpn", "plan"};

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
    printf("%s\n", SHELL_IMPLEMENTED_COMMANDS[i]);
    if (strcmp(SHELL_IMPLEMENTED_COMMANDS[i], command) == 0)
      return i;
  }
  return 0;
}

int start_process(char **tokens) {

  pid_t pid = fork();
  
  if (pid < 0) {
    fprintf(stderr, "Error Could not fork new process.\n");
    return -1; // if error in creating child process return immediately with error code -1
  }

  if (pid == 0) {
    int return_code = execvp(tokens[0], tokens);
    int implementation_code = check_shell_implementation(tokens[0]);
    if (return_code < 0 && implementation_code >= 0) {
      printf("will need shell implementation");
      return_code = implementation_code;
    }
    if (return_code < 0) {
      printf("error in running command\n");
    }
  } else if (pid > 0) {
    pid_t wait_return_code = wait(NULL);
    if (wait_return_code < 0) {
      fprintf(stdin, "error in running command\n");
    }
    printf("Parent %d child pid %d\n", getpid(), (int) pid);
  }
  
  return 0;
}


int main(int argc, char const *argv[])
{
  char  line[MAX_INPUT_SIZE];            
  char  **tokens;              
  int i;

  while (1) {           
    printf("Hello>");     
    bzero(line, MAX_INPUT_SIZE);
    fgets(line, MAX_INPUT_SIZE, stdin);           
    line[strlen(line)] = '\n'; 
    tokens = tokenize(line);

    int status = start_process(tokens);
    
    // Freeing the allocated memory	
    for(i=0;tokens[i]!=NULL;i++){
        free(tokens[i]);
    }
    free(tokens);
  }
  return 0;
}
