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

// Tokenizar y duplicar tokens en heap. Retorna un array recién allocado (char**) con
// máximo max_args entradas (última es nullptr). El llamador debe liberar con free_tokenized_argv.
static char **tokenize_dup_to_argv(const string &linea, int max_args) {
    char buf[512];
    strncpy(buf, linea.c_str(), sizeof(buf)); buf[sizeof(buf)-1] = '\0';
    char **out = (char **)malloc(max_args * sizeof(char*));
    if (!out) return nullptr;
    int i = 0;
    char *t = strtok(buf, " ");
    while (t != nullptr && i < max_args - 1) {
        out[i++] = strdup(t);
        t = strtok(nullptr, " ");
    }
    out[i] = nullptr;
    return out;
}

static void free_tokenized_argv(char **arr) {
    if (!arr) return;
    for (int i = 0; arr[i] != nullptr; ++i) free(arr[i]);
    free(arr);
}


// Ejecutar pipeline simple: left | right
static void ejecutar_pipeline_cmd(const string &left, const string &right) {
    const int MAX = 20;
    char **argsL = tokenize_dup_to_argv(left, MAX);
    char **argsR = tokenize_dup_to_argv(right, MAX);
    if (argsL == nullptr || argsR == nullptr || argsL[0] == nullptr || argsR[0] == nullptr) {
        cerr << "Pipe: sintaxis invalida" << endl;
        free_tokenized_argv(argsL); free_tokenized_argv(argsR);
        return;
    }

    int tub[2]; if (pipe(tub) < 0) { perror("pipe"); return; }
    pid_t p1 = fork();
    if (p1 < 0) { perror("fork"); close(tub[0]); close(tub[1]); return; }
    if (p1 == 0) {
        // hijo izquierdo: stdout -> tub[1]
        if (dup2(tub[1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(1); }
        close(tub[0]); close(tub[1]);
        // ejecutar left
        if (argsL[0][0] == '/') execv(argsL[0], argsL);
        else { char ruta[512]; snprintf(ruta, sizeof(ruta), "/bin/%s", argsL[0]); execv(ruta, argsL); }
        perror("Error al ejecutar comando (lado izquierdo)"); _exit(127);
    }

    pid_t p2 = fork();
    if (p2 < 0) { perror("fork"); close(tub[0]); close(tub[1]); return; }
    if (p2 == 0) {
        // hijo derecho: stdin <- tub[0]
        if (dup2(tub[0], STDIN_FILENO) < 0) { perror("dup2"); _exit(1); }
        close(tub[0]); close(tub[1]);
        if (argsR[0][0] == '/') execv(argsR[0], argsR);
        else { char ruta[512]; snprintf(ruta, sizeof(ruta), "/bin/%s", argsR[0]); execv(ruta, argsR); }
        perror("Error al ejecutar comando (lado derecho)"); _exit(127);
    }

    // padre
    close(tub[0]); close(tub[1]);
    int st; waitpid(p1, &st, 0); waitpid(p2, &st, 0);
    // liberar memoria usada para argv
    free_tokenized_argv(argsL);
    free_tokenized_argv(argsR);
}


void ejecutar_comando(const string &comando) {
    if (comando.empty()){
        return;
    }

    // Si la línea contiene una tubería simple 'left | right', manejarla aquí
    size_t pos_pipe = comando.find('|');
    if (pos_pipe != string::npos) {
        string left = recortar(comando.substr(0, pos_pipe));
        string right = recortar(comando.substr(pos_pipe + 1));
        if (left.empty() || right.empty()) {
            cerr << "Pipe: sintaxis invalida" << endl;
            return;
        }
        ejecutar_pipeline_cmd(left, right);
        return;
    }

    // eliminar posibles saltos de linea y recortar espacios alrededor
    string cmd_trim = recortar(comando);

    char buffer[256];
    strncpy(buffer, cmd_trim.c_str(), sizeof(buffer));
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
