#include <string>
#include <iostream>
#include "pda.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    
    if (argc == 3) {
        PDA<char, std::string, char> pda{std::string{argv[1]}};
        pda.dumpDefinition();
        std::string input{argv[2]};
        std::cout << (pda.accept(input) ? "true" : "false");
        return 0;
    }
    cout<<"This is for testing"<<endl;
    return 0;
}