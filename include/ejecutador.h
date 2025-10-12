#ifndef EJECUTADOR_H
#define EJECUTADOR_H
#include <string>
using namespace std;

void ejecutar_comando(const string &comando);
void ejecutar_paralelo(const string &entrada_o);
void manejar_redireccion(char *args[]);
void ejecutar_ruta(char *args[]);
bool manejar_tuberias(char *args[]);

#endif