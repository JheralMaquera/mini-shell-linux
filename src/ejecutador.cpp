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

void *ejecutar_hilo(void *arg){
    string comando=*(string*)arg;
    ejecutar_comando(comando); 
    return nullptr;
}

void ejecutar_paralelo(const string &entrada_o){
    string entrada = entrada_o;
    size_t start= entrada.find_first_not_of(" \t");
    if(start != string::npos){
        entrada = entrada.substr(start);
    }else{
        entrada="";
    }

    if (entrada.empty()){
        cout<<"Error: no se proporcionó ningún comando para ejecutar en paralelo."<<endl;
        return;
    }

    const int MAX_CMD=10;
    string comandos[MAX_CMD];
    int cantidad=0;

    int inicio=0;
    for(size_t i=0; i<=entrada.size(); i++){
        if(entrada[i]==';' || i==entrada.size()){
            string sub=entrada.substr(inicio, i-inicio);
            size_t start = sub.find_first_not_of(" \t");
            size_t end = sub.find_last_not_of(" \t");
            if(start != string::npos){
                sub = sub.substr(start, end - start + 1);
            }
            if(!sub.empty() && cantidad < MAX_CMD){
                comandos[cantidad++]=sub;
            }
            inicio=i+1;
        }
    }
    pthread_t hilos[MAX_CMD];
    for(int i=0; i<cantidad; i++){
        pthread_create(&hilos[i], nullptr, ejecutar_hilo, (void*)&comandos[i]);
    }
    for(int i=0; i<cantidad; i++){
        pthread_join(hilos[i], nullptr);
    }

    cout<<"Todos los comandos en paralelo han finalizado."<<endl;
}
