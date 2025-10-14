#include "ejecutador.h"
#include <iostream>
#include <string>

using namespace std;

int main(){
    string comando;
    string celeste = "\033[36m"; 
    string reset = "\033[0m";
    system("clear");
    string spaces = "\t\t";
    cout<<celeste;
    cout << spaces<<" ▄▄▄  ▄▄▄     ██                  ██               ▄▄                  ▄▄▄▄      ▄▄▄▄     \n";
    cout << spaces<<" ███  ███     ▀▀                  ▀▀               ██                  ▀▀██      ▀▀██     \n";
    cout << spaces<<" ████████   ████     ██▄████▄   ████     ▄▄█████▄  ██▄████▄   ▄████▄     ██        ██     \n";
    cout << spaces<<" ██ ██ ██     ██     ██▀   ██     ██     ██▄▄▄▄ ▀  ██▀   ██  ██▄▄▄▄██    ██        ██     \n";
    cout << spaces<<" ██ ▀▀ ██     ██     ██    ██     ██      ▀▀▀▀██▄  ██    ██  ██▀▀▀▀▀▀    ██        ██     \n";
    cout << spaces<<" ██    ██  ▄▄▄██▄▄▄  ██    ██  ▄▄▄██▄▄▄  █▄▄▄▄▄██  ██    ██  ▀██▄▄▄▄█    ██▄▄▄     ██▄▄▄  \n";
    cout << spaces<<" ▀▀    ▀▀  ▀▀▀▀▀▀▀▀  ▀▀    ▀▀  ▀▀▀▀▀▀▀▀   ▀▀▀▀▀▀   ▀▀    ▀▀    ▀▀▀▀▀      ▀▀▀▀      ▀▀▀▀  \n";
    cout << spaces<<"----------------------------------Bienvenido a minishell---------------------------------\n"; 
    cout << endl;
    cout << endl;
    cout<<reset;
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
