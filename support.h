/*
 * Filename: /home/nilanjan/Desktop/cs744/lab1/support.h
 * Path: /home/nilanjan/Desktop/cs744/lab1
 * Created Date: Sunday, July 22nd 2018, 11:09:58 pm
 * Author: nilanjan
 */

int insert_into_list(pid_t* list, pid_t pid, int size, int type) {
  int status = -1;
  for(int i = 0; i < size; i++) {
    if (list[i] == 0) {
      list[i] = pid;
      status = 0;
      break;
    }
  }
  //print_list(list, size, type);
  return status;
}

int remove_from_list(pid_t *list, pid_t pid, int size, int type) {
  int status = -1;
  for(int i = 0; i < size; i++)
  {
    if (list[i] == pid) {
      list[i] = 0;
      status = 0;
      break;
    }
  }
   //print_list(list, size, type);
  if (type == BACKGROUND_PROCESS)
    printf("Background process with pid %d stopped\n", pid);
  else if (type == FOREGROUND_PROCESS)
    printf("Foreground process with pid: %d stopped\n", pid);
  return status;
}

void print_list(pid_t *list, int size, int type) {
  printf("\n %s pid list ", (type == BACKGROUND_PROCESS)? "Background": "Foreground");
  for(int i = 0; i < size; i++) {
    printf("%d ", list[i]);
  }
  printf("\n");
}

int check_process_type(char **tokens) {
  int last_token_index = 0;
  //printf("%s", tokens[0]);
  for(int i = 0; tokens[i] != NULL; i++) {
    last_token_index = i;
  }
  // printf("%d", last_token_index);
  if (last_token_index >= 0) {

    char *last_token = tokens[last_token_index];
    // printf("%s", last_token);
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

char **get_process_delimiters(char **tokens, int start_point, int *type, int *checkpoint) {
  // printf("hello\n\n");
  // for (int i = 0; tokens[i] != NULL; i++) {
  //   printf("%s ", tokens[i]);
  // }
  // printf("\nhello\n\n");
  // printf("start_point %d", start_point);
  int end_point = 0, i = 0;
  //*type = -1;
  // printf("\nhere 78\n");
  for(i = start_point; tokens[i] != NULL; i++) {
    end_point = i;
    if (strcmp(tokens[i], "&&&") == 0) {
      *type = PARALLEL_PROCESS;
      end_point--;
      break;
    }
    else if (strcmp(tokens[i], "&&") == 0) {
      *type = SEQUENTIAL_PROCESS;
      end_point--;
      break;
    }
  }
  // printf("\nhere 92\n");
  *checkpoint = (tokens[i] != NULL)? end_point + 2: -1;
  int number_of_tokens = end_point - start_point + 2;
  // printf("\n here 95 %d %d %d %d\n", (end_point - start_point + 1), start_point, end_point, *checkpoint);
  char **process = (char **)malloc( number_of_tokens * sizeof(char *));
  int counter = 0;
  for(i = start_point; i <= end_point; i++) {
    // printf("%s %d\n", tokens[i], i);
    process[counter] = (char*)malloc(strlen(tokens[i]) * sizeof(char));
    strcpy(process[counter++], tokens[i]);
  }
  // printf("%s", process[0]);
  process[counter] = NULL;
  // printf("process type: %s\n", (*type == SEQUENTIAL_PROCESS)? "Sequential": "Parallel");
  return process;
  // return NULL;
}

