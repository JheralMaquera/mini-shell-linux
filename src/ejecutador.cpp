#include "ejecutador.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h> 
#include <cerrno>
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
        if (args[0][0] == '/') {
            // Comprobar si el archivo existe y tiene permisos de ejecucion
            struct stat errores;
            if (stat(args[0], &errores) != 0) {
                int err = errno;
                perror(("Error: la ruta '" + string(args[0]) + "' no existe").c_str());
                cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
                exit(1);
            }
            if (!(errores.st_mode & S_IXUSR)) {
                int err = EACCES; // permiso denegado
                errno = err; // para que perror muestre el texto correcto
                perror(("Error: la ruta '" + string(args[0]) + "' no tiene permisos de ejecución.").c_str());
                cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
                exit(1);
            }
            execv(args[0], args);
        } else {
            char ruta[256];
            snprintf(ruta, sizeof(ruta), "/bin/%s", args[0]);

            // Comprobar si existe y tiene permisos
            struct stat info;
            if (stat(ruta, &info) != 0) {
                int err = errno;
                perror(("Error: no se encontró el comando '" + string(args[0]) + "'").c_str());
                cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
                exit(1);
            }
            if (!(info.st_mode & S_IXUSR)) {
                int err = EACCES;
                errno = err;
                perror(("Error: el archivo '" + string(args[0]) + "' no tiene permisos de ejecución.").c_str());
                cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
                exit(1);
            }

            execv(ruta, args);
        }
        perror("Error al ejecutar el comando");
        exit(1);
    } 
    else if (pid > 0) {
        waitpid(pid, nullptr, 0);
        
    } 
    else {
        perror("Error al crear proceso fork");
    }
}
