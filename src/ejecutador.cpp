#include "ejecutador.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h> 
#include <cerrno>
#include <fcntl.h>
using namespace std;

// Recortar espacios 
static string recortar(const string &s){
    size_t b = 0;
    while (b < s.size() && isspace((unsigned char)s[b])) ++b;
    size_t e = s.size();
    while (e > b && isspace((unsigned char)s[e-1])) --e;
    return s.substr(b, e-b);
}




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

        int cen=-1;
        for(int j=0; args[j] != nullptr; j++){
            if(strcmp(args[j], ">") == 0){
                cen=j;
                break;
            }
        }

        if(cen != (-1)){
            if (args[cen + 1] == nullptr) {
                errno = EINVAL;
                perror("Error: falta el nombre del archivo para redireccionar la salida");
                cerr << "(errno " << errno << ": " << strerror(errno) << ")" << endl;
                exit(2);
        }

            char *nombreArchivo = args[cen+1];

            int fd = open(nombreArchivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(fd <0){
                int err = errno;
                perror(("Error al abrir o crear el archivo '" + string(nombreArchivo) + "'").c_str());
                cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
                exit(1);
            }       

            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[cen] = nullptr;
            
        }

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
