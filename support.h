int insert_into_list(pid_t* list, pid_t pid,int size) {
  int status = -1;
  for(int i = 0; i < size; i++) {
    if (list[i] == 0) {
      list[i] = pid;
      status = 0;
      break;
    }
  }
  //print_list(list, size);
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
  // print_list(list, size, type);
  if (type == BACKGROUND_PROCESS)
    printf("Background process with pid %d stopped\n", pid);
  return status;
}

void print_list(pid_t *list, int size, int type) {
  printf("\n %s pid list", (type == BACKGROUND_PROCESS)? "Background": "Foreground");
  for(int i = 0; i < size; i++) {
    printf("%d ", list[i]);
  }
  printf("\n");
}