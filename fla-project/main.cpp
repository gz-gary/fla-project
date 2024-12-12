#include <string>
#include <iostream>
#include "pda.hpp"
#include "tm.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    
    if (argc == 3) {
        // PDA<char, std::string, char> pda{std::string{argv[1]}};
        // pda.dumpDefinition();
        // std::string input{argv[2]};
        // std::cout << (pda.accept(input) ? "true" : "false");
        TM<char, std::string, char> tm{std::string{argv[1]}};
        tm.dumpDefinition();
        return 0;
    }
    cout<<"This is for testing"<<endl;
    return 0;
}