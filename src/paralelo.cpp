#include "ejecutador.h"
#include <iostream>
#include <pthread.h>
#include <string>
using namespace std;

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
