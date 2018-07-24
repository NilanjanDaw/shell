/*
 * Filename: /home/nilanjan/Desktop/cs744/lab1/definitions.h
 * Path: /home/nilanjan/Desktop/cs744/lab1
 * Created Date: Sunday, July 22nd 2018, 11:37:05 pm
 * Author: nilanjan
 */

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define HISTORY_KEEPER_PATH "/tmp/history"
#define MAX_PROCESS_LIST 64
#define BACKGROUND_PROCESS 0
#define FOREGROUND_PROCESS 1
#define SEQUENTIAL_PROCESS 2
#define PARALLEL_PROCESS 3


char **tokenize(char *line);
int cd(const char **tokens);
int history(const char **tokens);
int start_process(char **tokens, int type);
int check_shell_implementation(char *command);
void signal_handler(int signal);
void background_process_reaper();
void foreground_process_reaper();
int check_process_type(char **tokens, int start, int end);
int insert_into_list(pid_t* list, pid_t pid, int size, int type);
int remove_from_list(pid_t *list, pid_t pid, int size, int type);
void print_list(pid_t *list, int size, int type);
void clean_up_resources();