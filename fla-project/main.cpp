#include <string>
#include <iostream>
#include "pda.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    
    if (argc == 2) {
        PDA<char, std::string, char> pda{std::string{argv[1]}};
        pda.dump();
        return 0;
    }
    if(argc == 1){
        return 1;
    }
    cout<<"This is for testing"<<endl;
    return 0;
}