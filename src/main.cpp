#include "ejecutador.h"
#include <iostream>
#include <string>

using namespace std;

int main(){
    string comando;

    while(true){
        cout<<"minishell> ";
        getline(cin, comando);
        if(comando == "salir"){
            break;
        }
        if(comando.empty()){
            continue;
        }  
        ejecutar_comando(comando);
    }
    return 0;
}