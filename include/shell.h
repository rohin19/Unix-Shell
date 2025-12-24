#include <stdbool.h>
#include <stdio.h>

void add_to_history(char *user_input);
void remove_oldest_input();
void print_history();
void free_history();
void handle_sigint(int signum);
void registerSignalHandler();
void prompt_user();
void read_input(char *user_input);
void handle_cmd(char *user_input, char **arguments_ptr, int *background_flag);
void execute_cmd(char **arguments_ptr, int background_flag);
void zombie_terminate();
void internal_exit(char **arguments_ptr);
void internal_pwd(char **arguments_ptr);
void internal_cd(char **arguments_ptr);
void internal_help(char **arguments_ptr);
void internal_history(char **arguments_ptr);
void print_help();

void run_shell();
