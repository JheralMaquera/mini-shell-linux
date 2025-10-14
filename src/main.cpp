#include "ejecutador.h"
#include <iostream>
#include <string>

using namespace std;

int main(){
    string comando;

    while(true){
        cout<<"minishell> ";
        if (!getline(cin, comando)) {
            cout << "\n";
            break;
        }
        if(comando == "salir"){
            break;
        }
        if(comando.empty()){
            continue;
        }
        
        if(comando.rfind("parallel", 0) == 0){
            string resto = comando.substr(8);
            ejecutar_paralelo(resto);
        }else{
            ejecutar_comando(comando);
        }
    }
    return 0;
}
