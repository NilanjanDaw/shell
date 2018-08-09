/**
 * @Author: nilanjan
 * @Date:   2018-07-24T23:39:21+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: definitions.h
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-08-09T18:39:49+05:30
 * @Copyright: Nilanjan Daw
 */


/**
 * Constant Declarations
**/
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define HISTORY_KEEPER_PATH "/tmp/history"
#define MAX_PROCESS_LIST 64
#define BACKGROUND_PROCESS 0
#define FOREGROUND_PROCESS 1
#define SEQUENTIAL_PROCESS 2
#define PARALLEL_PROCESS 3

/**
 * function definitions
**/
char **tokenize(char *line);
int cd(const char **tokens);
int history(const char **tokens);
int start_process(char **tokens, int type);
int check_shell_implementation(char *command);
void signal_handler(int signal);
void background_process_reaper();
void foreground_process_reaper();
int check_process_type(char **tokens);
char **get_process_delimiters(char **tokens, int start_point, int *type, int *checkpoint);
int insert_into_list(pid_t* list, pid_t pid, int size, int type);
int remove_from_list(pid_t *list, pid_t pid, int size, int type);
void print_list(pid_t *list, int size, int type);
void clean_up_resources();
