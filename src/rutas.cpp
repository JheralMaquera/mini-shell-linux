#include "ejecutador.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>

using namespace std;

void ejecutar_ruta(char *args[]) {

    if (args[0][0] == '/') {
        // Comprobar si el archivo existe y tiene permisos de ejecucion
        struct stat errores;
        if (stat(args[0], &errores) != 0) {
            int err = errno;
            perror((string("Error: la ruta '") + string(args[0]) + "' no existe").c_str());
            cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
            _exit(1);
        }
        if (!(errores.st_mode & S_IXUSR)) {
            int err = EACCES; // permiso denegado
            errno = err; // para que perror muestre el texto correcto
            perror((string("Error: la ruta '") + string(args[0]) + "' no tiene permisos de ejecución.").c_str());
            cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
            _exit(1);
        }
        execv(args[0], args);

    } else {
        char ruta[256];
        snprintf(ruta, sizeof(ruta), "/bin/%s", args[0]);

        // Comprobar si existe y tiene permisos
        struct stat info;
        if (stat(ruta, &info) != 0) {
            int err = errno;
            perror((string("Error: no se encontró el comando '") + string(args[0]) + "'").c_str());
            cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
            _exit(1);
        }
        if (!(info.st_mode & S_IXUSR)) {
            int err = EACCES;
            errno = err;
            perror((string("Error: el archivo '") + string(args[0]) + "' no tiene permisos de ejecución.").c_str());
            cerr << "(errno " << err << ": " << strerror(err) << ")" << endl;
            _exit(1);
        }
        execv(ruta, args);

    }
    perror("Error al ejecutar comando");
    _exit(127);
}