/**
 * @Author: nilanjan
 * @Date:   2018-07-29T19:10:22+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: support.h
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-08-09T18:39:55+05:30
 * @Copyright: Nilanjan Daw
 */


/**
 * Insert PID into the list received in the first empty cell available
**/
int insert_into_list(pid_t* list, pid_t pid, int size, int type) {

  int status = -1;

  for(int i = 0; i < size; i++) {
    if (list[i] == 0) {
      list[i] = pid;
      status = 0;
      break;
    }
  }

  return status;
}

/**
 * remove process ID received from the list of IDs received
**/
int remove_from_list(pid_t *list, pid_t pid, int size, int type) {

  int status = -1;
  for(int i = 0; i < size; i++) {

    if (list[i] == pid) {
      list[i] = 0;
      status = 0;
      break;
    }
  }

  if (status != -1 && type == BACKGROUND_PROCESS)
    printf("Background process with pid %d stopped\n", pid);
  return status;
}

/**
 * @Depricated Function to print processes in a received list
**/
void print_list(pid_t *list, int size, int type) {
  printf("\n %s pid list ", (type == BACKGROUND_PROCESS)? "Background": "Foreground");
  for(int i = 0; i < size; i++) {
    printf("%d ", list[i]);
  }
  printf("\n");
}

/**
 * function to determine process type
**/
int check_process_type(char **tokens) {

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

/**
 * function to tokenize command into sub commands
**/
char **get_process_delimiters(char **tokens, int start_point, int *type, int *checkpoint) {

  int end_point = 0, i = 0;
  for(i = start_point; tokens[i] != NULL; i++) {

    end_point = i;
    if (strcmp(tokens[i], "&&&") == 0) {
      *type = PARALLEL_PROCESS;
      end_point--;
      break;
    }
    else if (strcmp(tokens[i], "&&") == 0) {

      end_point--;
      break;
    }
  }
  *checkpoint = (tokens[i] != NULL)? end_point + 2: -1;
  int number_of_tokens = end_point - start_point + 2;
  char **process = (char **)malloc( number_of_tokens * sizeof(char *));
  int counter = 0;
  for(i = start_point; i <= end_point; i++) {

    process[counter] = (char*)malloc(strlen(tokens[i]) * sizeof(char));
    strcpy(process[counter++], tokens[i]);
  }
  process[counter] = NULL;

  return process;
}
