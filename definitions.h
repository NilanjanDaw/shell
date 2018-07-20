#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define HISTORY_KEEPER_PATH "/tmp/history"
#define MAX_PROCESS_LIST 64
#define BACKGROUND_PROCESS 0
#define FOREGROUND_PROCESS 1


char **tokenize(char *line);
int cd(const char **tokens);
int history(const char **tokens);
int start_process(char **tokens);
int check_shell_implementation(char *command);
void signal_handler(int signal);
void background_process_reaper();
void foreground_process_reaper();
int insert_into_list(pid_t* list, pid_t pid,int size);
int remove_from_list(pid_t *list, pid_t pid, int size, int type);
void print_list(pid_t *list, int size, int type);