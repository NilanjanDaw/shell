
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

int check_process_type(char **tokens, int start, int end) {
  int last_token_index = 0;
  for(int i = 0; tokens[i] != NULL; i++) {
    last_token_index = i;
  }
  if (last_token_index >= 0) {
    char *last_token = tokens[last_token_index];
    if (last_token[strlen(last_token) - 1] == '&' && last_token[strlen(last_token) - 2] != '&') {
      last_token[strlen(last_token) - 1] = '\0';
      if (strlen(last_token) == 0)
        tokens[last_token_index] = NULL;
      return BACKGROUND_PROCESS;
    }
    return FOREGROUND_PROCESS;
  }
  return -1;
}

int get_process_delimiters(char **tokens, int start_point, int *type) {

  int end_point = -1;
  *type = -1;
  for(int i = start_point; tokens[i] != NULL; i++) {
    if (strcmp(tokens[i], "&&&") == 0) {
      end_point = i - 1;
      *type = PARALLEL_PROCESS;
      break;
    }
    else if (strcmp(tokens[i], "&&") == 0) {
      end_point = i - 1;
      *type = SEQUENTIAL_PROCESS;
      break;
    }
  }
  return end_point;
}

