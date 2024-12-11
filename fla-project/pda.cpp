#include "pda.hpp"
#include <fstream>
#include <iostream>

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeFromFile(const std::string &path) {
    std::ifstream file{path};

    if (!file.is_open())
        throw std::runtime_error("Fail to open file when initializing PDA");

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    file.close();
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
PDA<InputSymbol, StateSymbol, StackSymbol>::PDA()
{
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
PDA<InputSymbol, StateSymbol, StackSymbol>::PDA(const std::string &path) {
    initializeFromFile(path);
}
