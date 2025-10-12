#include "ejecutador.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h> 
#include <cerrno>
#include <fcntl.h>
#include <pthread.h>
using namespace std;

void ejecutar_comando(const string &comando) {
    if (comando.empty()){
        return;
    }
    char buffer[256];
    strncpy(buffer, comando.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    const int MAX= 10;
    char *args[MAX];
    int i = 0;
    char *token = strtok(buffer, " ");

    while (token != nullptr && i < MAX - 1) {
        args[i++] = token;
        token = strtok(nullptr, " ");
    }

    args[i] = nullptr;
    if (args[0] == nullptr) return;
    pid_t pid = fork();

    if (pid == 0) {
        if(manejar_tuberias(args)){
            _exit(0);
        }
        manejar_redireccion(args);
        ejecutar_ruta(args);
    } 
    else if (pid > 0) {
        waitpid(pid, nullptr, 0);
        
    } 
    else {
        perror("Error al crear proceso fork");
    }
}


