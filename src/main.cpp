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
        cout<<"Ejecutando comando: "<<comando<<endl;
    }
    return 0;
}