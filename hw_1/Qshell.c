#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

typedef int bool;
#define true 1
#define false 0

void printArrow(){
  char path_name[PATH_MAX];

  if (getcwd(path_name, sizeof(path_name)) == NULL) {
    printf("> ");
  } else {
    printf("%s> ", path_name);
  }

}

char *getSafeLine(){
  char *arguments_line = NULL;
  size_t length = 0;
  int number = getline(&arguments_line, &length, stdin);
   if (number == -1 && errno == 0) {
     printf("\n");
     exit(0);
  }
  return arguments_line;
}

char **extractArgs(char *arguments_line){
  int bufsize = 64;
  int position = 0;
  char **arguments = malloc(bufsize * sizeof(char*));
  char *argument = NULL;

  if (!arguments) {
    fprintf(stderr, "Ошибка выделения памяти\n");
    exit(1);
  }

  if (arguments_line == NULL){
    arguments[0] = "exit";
    return arguments;
  }

  argument = strtok(arguments_line, " \t\r\n\a");

  if (argument == NULL) {
    arguments[0] = "skip";
    return arguments;
  }

  while (argument != NULL) {
    arguments[position] = argument;
    position++;

    if (position >= bufsize) {
      bufsize += 64;
      argument = realloc(arguments, bufsize * sizeof(char*));
      if (!arguments) {
        free(arguments_line);
        free(arguments);
        free(argument);
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 0;
      }
    }
    argument = strtok(NULL, " \t\r\n\a");
  }
  arguments[position] = NULL;

  return arguments;
}

int executeCommand(char **arguments){
  if (!strcmp(arguments[0],"exit")) {
    return false;
  }
  if (!strcmp(arguments[0],"skip")) {
    return true;
  }

  pid_t pid,wpid;
  pid = fork();

  int status;
  if (pid == 0){

    char *envp[] = {NULL};

    if (execve(arguments[0], &arguments[0],envp) == -1) {
      printf("execve error %i\n", errno);
      perror("");
    }
    exit(1);
  } else if (pid < 0){
    printf("fork error %i\n", errno);
    perror("");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);

      if (wpid == 0) {
        printf("information process status is not available error\n");
        perror("");
      } else if (wpid == -1) {
        printf("waitpid error %i\n", errno);
        perror("");
      }

    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    printf("%i\n",status);
  }
  return true;
}


int main(int argc, char const *argv[]) {

char *arguments_line = NULL;
char **arguments = NULL;
bool next = true;

  if (argc > 1){
    printf("Too many argumnents, expected 0\n");
  } else {
    while(next){

      printArrow();
      arguments_line = getSafeLine();

      arguments = extractArgs(arguments_line);

      next = executeCommand(arguments);

      free(arguments_line);
      free(arguments);
    }
  }

  return 0;
}
