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

// Tokenizar una linea simple por espacios en un argv estilo exec
// args_out debe ser char*[] con espacio para max_args, se termina con nullptr
static void tokenize_to_argv(const string &linea, char *args_out[], int max_args) {
    char buf[512];
    strncpy(buf, linea.c_str(), sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    int i = 0;
    char *t = strtok(buf, " ");
    while (t != nullptr && i < max_args-1) {
        args_out[i++] = t;
        t = strtok(nullptr, " ");
    }
    args_out[i] = nullptr;
}

// En el hijo: procesar redireccion '>' y ejecutar argv (no hace fork)
// args es modificado para que execv no reciba tokens especiales
static void ejecutar_args_simple(char *args[]) {
    // Manejar redireccion '>' simple
    int idx = -1;
    for (int j = 0; args[j] != nullptr; ++j) {
        if (strcmp(args[j], ">") == 0) { idx = j; break; }
    }
    if (idx != -1) {
        if (args[idx+1] == nullptr) {
            errno = EINVAL; perror("Error: falta el nombre del archivo para redireccionar la salida");
            cerr << "(errno " << errno << ": " << strerror(errno) << ")" << endl;
            _exit(2);
        }
        char *file = args[idx+1];
        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            int err = errno;
            perror((string("Error al abrir o crear el archivo '") + file + "'").c_str());
            cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
            _exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); _exit(1); }
        close(fd);
        // quitar token '>' para exec (poner nullptr en su lugar)
        args[idx] = nullptr;
    }

    // Resolver ruta y ejecutar
    if (args[0] == nullptr) _exit(0);
    if (args[0][0] == '/') {
        // ruta absoluta
        struct stat st;
        if (stat(args[0], &st) != 0) { int err = errno; perror((string("Error: la ruta '") + args[0] + "' no existe").c_str()); cerr << "(errno " << err << ": " << strerror(err) << ")" << endl; _exit(1); }
        if (!(st.st_mode & S_IXUSR)) { int err = EACCES; errno = err; perror((string("Error: la ruta '") + args[0] + "' no tiene permisos de ejecución.").c_str()); cerr << "(errno " << err << ": " << strerror(err) << ")" << endl; _exit(1); }
        execv(args[0], args);
    } else {
        char ruta[512]; snprintf(ruta, sizeof(ruta), "/bin/%s", args[0]);
        struct stat st2;
        if (stat(ruta, &st2) != 0) { int err = errno; perror((string("Error: no se encontró el comando '") + args[0] + "'").c_str()); cerr << "(errno " << err << ": " << strerror(err) << ")" << endl; _exit(1); }
        if (!(st2.st_mode & S_IXUSR)) { int err = EACCES; errno = err; perror((string("Error: el archivo '") + args[0] + "' no tiene permisos de ejecución.").c_str()); cerr << "(errno " << err << ": " << strerror(err) << ")" << endl; _exit(1); }
        execv(ruta, args);
    }
    perror("Error al ejecutar el comando");
    _exit(1);
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
