
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

struct parametrs {
    int inum;
    char *name;
    char size_znak;
    int size;
    int nlinks_num;
    char *exec_path;
};

void execute_command(char * * arguments);

void dfsDir(char *dirName, struct parametrs* p)
{
   DIR* dir;
   struct dirent *dirEntry;
   struct stat inode;
   char name[1000];
   dir = opendir(dirName);

if (dir == 0) {
    perror(dirName);
    closedir(dir);
    return;
  }

while ((dirEntry=readdir(dir)) != 0) {
      sprintf(name,"%s/%s",dirName,dirEntry->d_name);
      lstat (name, &inode);

       // test the type of file
      if (S_ISDIR(inode.st_mode)) {
        if (strcmp(".", dirEntry->d_name) != 0 && strcmp("..", dirEntry->d_name) != 0){
          char pth[1024];
          snprintf(pth, sizeof(pth), "%s/%s", dirName, dirEntry->d_name);
          dfsDir(pth, p);
        }
      }
      else if (S_ISREG(inode.st_mode)){
        if (p->inum != -1){
          if (inode.st_ino != p->inum){
            continue;
          }
        }
        if (p->name != NULL){
          if (strcmp(p->name, dirEntry->d_name) != 0){
            continue;
          }
        }
        if (p->size != -1){
          if (p->size_znak == '-' && inode.st_size >= p->size){
              continue;
          } else if (p->size_znak == '=' && inode.st_size != p->size){
            continue;
          } else if (p->size_znak == '+' && inode.st_size <= p->size){
            continue;
          }
        }
        if (p->nlinks_num != -1){
          if (p->nlinks_num != inode.st_nlink){
            continue;
          }
        }
        if (p->exec_path != NULL){
          char * arguments[3] = {p->exec_path, name, NULL};
          execute_command(arguments);
        }

        char pth[1024];
        snprintf(pth, sizeof(pth), "%s/%s", dirName, dirEntry->d_name);
        printf("%s\n", pth);
      }
      //else if (S_ISLNK(inode.st_mode))
    }
    closedir(dir);
}

void init_params(struct parametrs *p) {
    p->inum = -1;
    p->name = NULL;
    p->size_znak = '@';
    p->size = -1;
    p->nlinks_num = -1;
    p->exec_path = NULL;
}

int get_number(char * s, int * answer) {
   int res = 0;
   s++;
   while(isdigit(*s)) {
      res = res * 10 + (*s++ - '0');
   }
   if (*s) {
       return 0;
   }
   (*answer) = res;
   return 1;
}

void execute_command(char * * arguments) {

    pid_t pid = fork();
    int status;
    if (pid == 0) {
        char * envp[] = {NULL};

        if (execve(arguments[0], &arguments[0],envp) == -1) {

            perror("Error with execve");
        }
        exit(0);
    } else if (pid < 0) {

        perror("Error with fork");

    } else {
        do {
            pid_t wpid = waitpid(pid, &status, WUNTRACED);

            if (wpid == 0) {
                perror("Error with waiting process");
            } else
              if (wpid == -1) {
                perror("Error while waiting for process");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void parse_args(int argc, char**argv, struct parametrs *p){
    int i = 2;
    while(i < argc-1){
      //printf("! %s\n",argv[i]);
      if (strcmp(argv[i],"-inum") == 0){
            p->inum = atoi(argv[++i]);
            //printf("*** %i\n", p->inum);
        } else if (strcmp(argv[i],"-name") == 0)
            p->name = argv[++i];
        else if (strcmp(argv[i],"-size") == 0){
          p->size_znak = argv[++i][0];
          int size_num;
          if(!get_number(argv[i], &size_num)) {
              printf("Invalid number in the size parametr");
              exit(0);
          }
          p->size = size_num;
          //printf("*- %i\n", p->size);
        } else if (strcmp(argv[i],"-nlinks") == 0)
            p->nlinks_num = atoi(argv[++i]);
        else if (strcmp(argv[i],"-exec") == 0){
              p->exec_path = argv[++i];
            } else {
            printf("Wrong arguments. Usage: find /path_to_folder -inum [inum] -name [name] -size [-+=][size] -nlinks [numOfHardLinks] -exec [path_to_executable_for_file_output]\n");
            exit(0);
          }
      i++;
    }
}

int main(int argc, char **argv)
{
    struct parametrs params;

    init_params(&params);
    parse_args(argc, argv, &params);

    if (argc < 2) {
      printf("Wrong arguments. Usage: find /path_to_folder -inum [inum] -name [name] -size [-+=][size] -nlinks [numOfHardLinks] -exec [path_to_executable_for_file_output]\n");
      exit(0);
    }

    dfsDir(argv[1], &params);
}
