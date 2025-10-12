#include "ejecutador.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include <cstdlib>
using namespace std;

void manejar_redireccion(char *args[]) {
    int pos = -1;
    for (int i = 0; args[i] != nullptr; i++) {
        if (strcmp(args[i], ">") == 0) {
            pos = i;
            break;
        }
    }

    if (pos == -1){
        return;
    }

    if (args[pos + 1] == nullptr) {
        cerr << "Error: falta el nombre del archivo después de '>'" << endl;
        exit(2);
    }

    char *archivo = args[pos + 1];
    int fd = open(archivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror(("Error al abrir o crear archivo '" + string(archivo) + "'").c_str());
        exit(1);
    }

    dup2(fd, STDOUT_FILENO); // Redirige salida estándar al archivo
    close(fd);
    args[pos] = nullptr; // Corta los argumentos en '>'
}