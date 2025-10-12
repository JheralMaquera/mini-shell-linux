#include "ejecutador.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>
using namespace std;

bool manejar_tuberias(char *args[]){

    int pipe_index = -1;
    for (int i = 0; args[i] != nullptr; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }
    if (pipe_index == -1) {
        return false;
    }

    args[pipe_index] = nullptr; 
    char *cmd1[15];
    char *cmd2[15];
    int i1 = 0, i2 = 0;
    for (int i = 0; args[i] != nullptr; i++) {
        cmd1[i1] = args[i];
        i1++;
    }
    cmd1[i1] = nullptr;
    for (int i = pipe_index + 1; args[i] != nullptr; i++) {
        cmd2[i2] = args[i];
        i2++;
    }
    cmd2[i2] = nullptr;

    int fd[2];
    if (pipe(fd) == -1) {
        perror("Error al crear la tubería");
        exit(1);
    }
    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(cmd1[0], cmd1);
        perror("Error al ejecutar el primer comando");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(cmd2[0], cmd2);
        perror("Error al ejecutar el segundo comando");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    return true;
}