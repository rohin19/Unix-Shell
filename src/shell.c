#define _POSIX_C_SOURCE 200809
#include "shell.h"
#include "msgs.h"
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// define max size buffer and max len for history array
#define MAX_SIZE 1000
#define MAX_LEN 10
// initialize global array and counters for history
char *history[MAX_LEN];
int history_count = 0;
int total_cmds = 0;

void add_to_history(char *user_input) {
  // if no space in array rewrite over oldest
  if (history_count >= MAX_LEN) {
    remove_oldest_input();
  }
  // allocate space for the new input in history and copy it in there
  history[history_count] = malloc(strlen(user_input) + 1);
  strcpy(history[history_count], user_input);
  // increment both counters
  history_count++;
  total_cmds++;
}

void remove_oldest_input() {
  // if there is something in the array
  if (history_count > 0) {
    // free the memory there
    free(history[0]);
    // slide all the elements in front back one
    for (int i = 1; i < history_count; i++) {
      history[i - 1] = history[i];
    }
    // decrement counter for elements in history
    history_count--;
  }
}

void print_history() {
  // find proper start point for each command past 10
  int start = (total_cmds > MAX_LEN) ? total_cmds - MAX_LEN : 0;
  // print last 10 commands in descending order
  for (int i = history_count - 1; i >= 0; i--) {
    if (history[i] != NULL) {
      char print_his[MAX_SIZE];
      snprintf(print_his, MAX_SIZE, "%d\t%s", start + i, history[i]);
      write(STDOUT_FILENO, print_his, strlen(print_his));
    }
  }
}

void free_history() {
  // free all allocated memory in history
  for (int i = 0; i < history_count; i++) {
    free(history[i]);
  }
  // reset counter to 0
  history_count = 0;
}

void handle_sigint(int signum) {
  // only handle sigint not other signals
  if (signum == SIGINT) {
    char *newline = "\n";
    write(STDOUT_FILENO, newline, strlen(newline));
    print_help();
    prompt_user();
  }
}

void registerSignalHandler() {
  // register the signal handler
  struct sigaction act;
  act.sa_handler = handle_sigint;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);

  int ret = sigaction(SIGINT, &act, NULL);
  if (ret == -1) {
    perror("Sigaction() failed");
    exit(EXIT_FAILURE);
  }
}

void prompt_user() {
  // initialize buffer to store cwd
  char cwd[MAX_SIZE];
  // call getcwd and check if it returns an error
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    // print cwd error message
    char cwd_err[MAX_SIZE];
    snprintf(cwd_err, MAX_SIZE, "shell: %s\n", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, cwd_err, strlen(cwd_err));
    return;
  }
  // write the cwd followed by a $ to stdout
  write(STDOUT_FILENO, cwd, strlen(cwd));
  write(STDOUT_FILENO, "$ ", 2);
}

void read_input(char *user_input) {
  // initialize number of bytes read var
  ssize_t bytes_read;
  // loop used so that function can retry after SIGINT
  while (true) {
    // read in user input
    bytes_read = read(STDIN_FILENO, user_input, MAX_SIZE);
    // if number of bytes read is -1 then an error occured
    if (bytes_read < 0) {
      // if errno is set by SIGINT, dont print error msg
      if (errno == EINTR) {
        // retry reading
        continue;
      }
      // print read error message
      char read_err[MAX_SIZE];
      snprintf(read_err, MAX_SIZE, "shell: %s\n", READ_ERROR_MSG);
      write(STDERR_FILENO, read_err, strlen(read_err));
      return;
    }
    // exit loop as read succeeded and SIGINT does not need to be checked
    break;
  }
  // strip new line character from end
  user_input[bytes_read] = '\0';
  // only add the input to history if it didnt start with a '!'
  if (user_input[0] != '!') {
    add_to_history(user_input);
  }
}

void handle_cmd(char *user_input, char **arguments_ptr, int *background_flag) {
  // intialize chars for strtok_r
  char *ret = NULL;
  char *saveptr = NULL;
  // to move through our array of strings
  int i = 0;
  // start with flag as false
  *background_flag = 0;

  // tokenize on space, tab, or newline and save to ret
  ret = strtok_r(user_input, " \t\n", &saveptr);
  // set input as NULL so loop tokenizes from saveptr
  user_input = NULL;
  // evaluate each token
  while (ret != NULL) {
    // get length of each token
    int ret_len = strlen(ret);

    // check if last character is '&'
    if (ret[ret_len - 1] == '&') {
      // set a flag that says if it is a background process
      *background_flag = 1;

      // token string is just the '&' character
      if (ret_len == 1) {
        // continue tokenizing and do not store this '&' command
        ret = strtok_r(user_input, " \t\n", &saveptr);
        // continue with next iteration so we dont access bad
        // memory at the end
        continue;
      } else {
        // remove the '&' from the end so we can store it
        // '&' command is never stored, just used for flag
        ret[ret_len - 1] = '\0';
      }
    }
    // store the argument
    arguments_ptr[i++] = ret;
    // continue tokenizing the input
    ret = strtok_r(user_input, " \t\n", &saveptr);
  }
  // set last index of array to be NULL so the commands are
  // NULL terminating and can be used in execv
  arguments_ptr[i] = NULL;
  // variable ptr that holds the 0th arg
  char *first_arg = arguments_ptr[0];
  // if an argument was recgonized
  if (arguments_ptr[0] != NULL) {
    // if the first arg was an internal command
    // execute that command through its function
    // not through execute_cmd which will fork it
    if (strcmp(first_arg, "exit") == 0) {
      internal_exit(arguments_ptr);
    } else if (strcmp(first_arg, "pwd") == 0) {
      internal_pwd(arguments_ptr);
    } else if (strcmp(first_arg, "cd") == 0) {
      internal_cd(arguments_ptr);
    } else if (strcmp(first_arg, "help") == 0) {
      internal_help(arguments_ptr);
    } else if (strcmp(first_arg, "history") == 0) {
      print_history();
    } else if (first_arg[0] == '!') {
      internal_history(arguments_ptr);
    }
    // if first command was not an internal command and
    // was recgonized execute that command using fork
    else {
      execute_cmd(arguments_ptr, *background_flag);
    }
  }
}

void execute_cmd(char **arguments_ptr, int background_flag) {
  // fork and check for errors
  pid_t pid = fork();
  if (pid < 0) {
    // fork returned an error
    char fork_err[MAX_SIZE];
    snprintf(fork_err, MAX_SIZE, "shell: %s\n", FORK_ERROR_MSG);
    write(STDERR_FILENO, fork_err, strlen(fork_err));
    return;
  }
  // if child process
  if (pid == 0) {
    if (execvp(arguments_ptr[0], arguments_ptr) == -1) {
      // exec failed
      char exec_err[MAX_SIZE];
      snprintf(exec_err, MAX_SIZE, "shell: %s\n", EXEC_ERROR_MSG);
      write(STDERR_FILENO, exec_err, strlen(exec_err));
      return;
    }
  }
  // if parent process
  else {
    // if command is a background process do not wait on it
    // if foreground process must wait on child
    if (background_flag != 1) {
      int wstatus = 0;
      pid_t wpid = waitpid(pid, &wstatus, 0);
      if (wpid == -1) {
        // waitpid failed
        char wait_err[MAX_SIZE];
        snprintf(wait_err, MAX_SIZE, "shell: %s\n", WAIT_ERROR_MSG);
        write(STDERR_FILENO, wait_err, strlen(wait_err));
        return;
      }
    }
  }
}

void zombie_terminate() {
  // wait on any child process (-1), return immediately if no
  // child has existed
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  // continously call waitpid() until no more zombies
  // retuns 0 when there are no more zombies need to be reaped
}

void internal_exit(char **arguments_ptr) {
  // if args are provided, print error msg
  if (arguments_ptr[1]) {
    char exit_tma[MAX_SIZE];
    snprintf(exit_tma, MAX_SIZE, "exit: %s\n", TMA_MSG);
    write(STDERR_FILENO, exit_tma, strlen(exit_tma));
    return;
  } else if (strcmp(arguments_ptr[0], "exit") == 0) {
    // printf("first arg: %s\n", arguments_ptr[0]);
    // just 'exit' was entered, exit on success
    exit(0);
  }
}

void internal_pwd(char **arguments_ptr) {
  // if args are provided print error
  if (arguments_ptr[1] != NULL) {
    char pwd_tma[MAX_SIZE];
    snprintf(pwd_tma, MAX_SIZE, "pwd: %s\n", TMA_MSG);
    write(STDERR_FILENO, pwd_tma, strlen(pwd_tma));
    return;
  }
  // get working directory
  char cwd[MAX_SIZE];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    // if call to getcwd fails print error
    char cwd_err[MAX_SIZE];
    snprintf(cwd_err, MAX_SIZE, "pwd: %s\n", GETCWD_ERROR_MSG);
    write(STDERR_FILENO, cwd_err, strlen(cwd_err));
    return;
  }
  // getcwd worked so display the cwd
  char print_cwd[MAX_SIZE];
  snprintf(print_cwd, MAX_SIZE, "%s\n", cwd);
  write(STDOUT_FILENO, print_cwd, strlen(print_cwd));
}

// global variable to hold directory we want to move back into
char last_directory[MAX_SIZE];
void internal_cd(char **arguments_ptr) {
  // if more than one arg exists print error
  if (arguments_ptr[1] && arguments_ptr[2]) {
    char cd_tma[MAX_SIZE];
    snprintf(cd_tma, MAX_SIZE, "cd: %s\n", TMA_MSG);
    write(STDERR_FILENO, cd_tma, strlen(cd_tma));
    return;
  }
  // first argument is the directory we want to cd into
  char *directory = arguments_ptr[1];
  // get current directory so we can use initially when changing back
  char cwd[MAX_SIZE];
  getcwd(cwd, sizeof(cwd));

  // if no arg provided or '~' is the arg, change to home dir
  if (!directory || (strcmp(directory, "~")) == 0) {
    // get home dir from user id and assign it to the directory
    struct passwd *pw = getpwuid(getuid());
    // store the home directory as the directory we want to cd into
    directory = pw->pw_dir;
  }
  // '-' used, change back to previous dir
  else if (strcmp(directory, "-") == 0) {
    // store the last directory we were in as what we want to cd into
    directory = last_directory;
  }
  // if arg is in this format: '~/dir'
  else if (directory[0] == '~') {
    // replace '~' with the home directory
    // get home dir from user id
    struct passwd *pw = getpwuid(getuid());
    // holds the '~/dir' with '~' fully expanded
    char full_directory[MAX_SIZE];
    // pw->pw_dir is the home dir, directory+1 starts from
    // the slash after the '~', allows us to concatenate them
    // together to form a full directory path from home
    snprintf(full_directory, MAX_SIZE, "%s%s", pw->pw_dir, (directory + 1));
    // store the full directory as the directory we want to cd into now
    directory = full_directory;
  }
  // change into whatever directory we selected
  if (chdir(directory) == -1) {
    // if error occured, print error message
    char chdir_err[MAX_SIZE];
    snprintf(chdir_err, MAX_SIZE, "cd: %s\n", CHDIR_ERROR_MSG);
    write(STDERR_FILENO, chdir_err, strlen(chdir_err));
    return;
  }
  // copy directory we got at start into last directory
  // so it can be used for '-' functionality next time
  strncpy(last_directory, cwd, MAX_SIZE);
}

void internal_help(char **arguments_ptr) {
  // if more than 1 arg, print err
  if (arguments_ptr[1] && arguments_ptr[2]) {
    char help_tma[MAX_SIZE];
    snprintf(help_tma, MAX_SIZE, "help: %s\n", TMA_MSG);
    write(STDERR_FILENO, help_tma, strlen(help_tma));
    return;
  }
  // if an argument exists
  if (arguments_ptr[1] != NULL) {
    // initialize a help message
    char help_msg[MAX_SIZE];
    // if that arg is exit, print exit help
    if (strcmp(arguments_ptr[1], "exit") == 0) {
      snprintf(help_msg, MAX_SIZE, "exit: %s\n", EXIT_HELP_MSG);
      write(STDOUT_FILENO, help_msg, strlen(help_msg));
    }
    // if that arg is pwd, print pwd help
    else if (strcmp(arguments_ptr[1], "pwd") == 0) {
      snprintf(help_msg, MAX_SIZE, "pwd: %s\n", PWD_HELP_MSG);
      write(STDOUT_FILENO, help_msg, strlen(help_msg));
    }
    // if that arg is cd, print cd help
    else if (strcmp(arguments_ptr[1], "cd") == 0) {
      snprintf(help_msg, MAX_SIZE, "cd: %s\n", CD_HELP_MSG);
      write(STDOUT_FILENO, help_msg, strlen(help_msg));
    }
    // if that arg is help, print help help
    else if (strcmp(arguments_ptr[1], "help") == 0) {
      snprintf(help_msg, MAX_SIZE, "help: %s\n", HELP_HELP_MSG);
      write(STDOUT_FILENO, help_msg, strlen(help_msg));
    }
    // if that arg is history
    else if (strcmp(arguments_ptr[1], "history") == 0) {
      snprintf(help_msg, MAX_SIZE, "history: %s\n", HISTORY_HELP_MSG);
      write(STDOUT_FILENO, help_msg, strlen(help_msg));
    }
    // if arg is not an internal cmd
    else {
      char external_msg[MAX_SIZE];
      snprintf(external_msg, MAX_SIZE, "%s: %s\n", arguments_ptr[1],
               EXTERN_HELP_MSG);
      write(STDOUT_FILENO, external_msg, strlen(external_msg));
    }
  }
  // if there is no argument provided
  // list all help messages for interal commands
  if (arguments_ptr[1] == NULL) {
    print_help();
  }
}

void internal_history(char **arguments_ptr) {
  // store first word of arguments_ptr in arg
  char *arg = arguments_ptr[0];

  // handle "!!" functionality
  if (strcmp(arg, "!!") == 0) {
    // if there are no commands print error message
    if (total_cmds == 0) {
      char no_cmds[MAX_SIZE];
      snprintf(no_cmds, MAX_SIZE, "history: %s\n", HISTORY_NO_LAST_MSG);
      write(STDERR_FILENO, no_cmds, strlen(no_cmds));
      return;
    }
    // if arg is !! and there are cmds stored in history

    // copy the last cmd in history to last_cmd and print it
    char last_cmd[MAX_SIZE];
    strcpy(last_cmd, history[history_count - 1]);
    write(STDOUT_FILENO, last_cmd, strlen(last_cmd));
    // add that command to history and handle the command like usual
    add_to_history(last_cmd);
    int bflag;
    handle_cmd(last_cmd, arguments_ptr, &bflag);
  }

  // handle "!n" functionality
  else {
    // skip over '!'
    char *num_str = arg + 1;
    // set up error message if 'n' is invalid
    char inv_cmd[MAX_SIZE];
    snprintf(inv_cmd, MAX_SIZE, "history: %s\n", HISTORY_INVALID_MSG);

    // loop through each character after the '!'
    int i = 0;
    while (num_str[i] != '\0') {
      // if that character is not a digit print error
      if (isdigit((char)num_str[i]) == 0) {
        write(STDERR_FILENO, inv_cmd, strlen(inv_cmd));
        return;
      }
      i++;
    }
    // convert string to int
    int n = atoi(num_str);
    // calculate start of history if history is greater than 10
    // ex) 30 total cmds => start of the last 10 is at index 20 (21st cmd)
    int start = (total_cmds > MAX_LEN) ? total_cmds - MAX_LEN : 0;
    // the idx of the cmd we want to specify has to be between 0-9
    int cmd_idx = n - start;
    // if this is less than 0 or more than the last idx of history
    // print an error
    if (cmd_idx < 0 || cmd_idx >= history_count) {
      write(STDERR_FILENO, inv_cmd, strlen(inv_cmd));
      return;
    }
    // copy the specified command into n_cmd and print that
    char n_cmd[MAX_SIZE];
    strcpy(n_cmd, history[cmd_idx]);
    write(STDOUT_FILENO, n_cmd, strlen(n_cmd));
    // add the n_cmd to history as we are gonna execute it again
    add_to_history(n_cmd);
    // handle that command appropriately
    int bflag;
    handle_cmd(n_cmd, arguments_ptr, &bflag);
  }
}

void print_help() {
  // print all help messages
  char exit_help[MAX_SIZE];
  char pwd_help[MAX_SIZE];
  char cd_help[MAX_SIZE];
  char help_help[MAX_SIZE];
  char hist_help[MAX_SIZE];
  snprintf(exit_help, MAX_SIZE, "exit: %s\n", EXIT_HELP_MSG);
  snprintf(pwd_help, MAX_SIZE, "pwd: %s\n", PWD_HELP_MSG);
  snprintf(cd_help, MAX_SIZE, "cd: %s\n", CD_HELP_MSG);
  snprintf(help_help, MAX_SIZE, "help: %s\n", HELP_HELP_MSG);
  snprintf(hist_help, MAX_SIZE, "history: %s\n", HISTORY_HELP_MSG);
  write(STDOUT_FILENO, exit_help, strlen(exit_help));
  write(STDOUT_FILENO, pwd_help, strlen(pwd_help));
  write(STDOUT_FILENO, cd_help, strlen(cd_help));
  write(STDOUT_FILENO, help_help, strlen(help_help));
  write(STDOUT_FILENO, hist_help, strlen(hist_help));
}

void run_shell() {
  // initialize variables for user input arguments
  // and a flag for foreground/background
  char input[MAX_SIZE];
  char *args[MAX_SIZE];
  int flag;

  // check for SIGINT at any point
  registerSignalHandler();
  // continuouslly loop until exit is used
  while (true) {
    // prompt, read, tokenize and execute
    prompt_user();
    read_input(input);
    //    add_to_history(input);
    handle_cmd(input, args, &flag);
    // cleanup zombie processes
    zombie_terminate();
  }
}

int main() {
  // run shell program
  run_shell();
  free_history();
  return 0;
}
