#include "pda.hpp"
#include "utils.hpp"

#include <string>
#include <fstream>
#include <iostream>

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeFromFile(const std::string &path) {
    std::ifstream file{path};

    if (!file.is_open())
        throw std::runtime_error("Fail to open file when initializing PDA");

    std::string line;
    while (std::getline(file, line)) {
        if (prefix_of(";", line)) {
            // ignore notation
            continue;
        } else if (prefix_of("#Q = ", line)) {
            initializeStateAlphabet(line);
        } else if (prefix_of("#S = ", line)) {
            initializeInputAlphabet(line);
        } else if (prefix_of("#G = ", line)) {
            initializeStackAlphabet(line);
        } else {
            continue;
        }
    }

    file.close();
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeStateAlphabet(std::string def) {
    remove_prefix("#Q = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            state_alphabet.insert(state);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeInputAlphabet(std::string def) {
    remove_prefix("#S = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            if (state.length() > 1) 
                throw std::runtime_error("Syntax error on PDA set definition");
            input_alphabet.insert(state[0]);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeStackAlphabet(std::string def) {
    remove_prefix("#G = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            if (state.length() > 1) 
                throw std::runtime_error("Syntax error on PDA set definition");
            stack_alphabet.insert(state[0]);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::parseSetDef(std::string def, std::function<void(const std::string &)> callback) {
    if (!
    ( 
      def.length() >= 2
      && def[0] == '{'
      && def[def.length() - 1] == '}'
    )
    ) throw std::runtime_error("Syntax error on PDA set definition");

    def = def.substr(1, def.length() - 2);
    size_t pos = 0;
    while (pos < def.length()) {
        size_t nxt_pos = def.find(",", pos);
        if (nxt_pos == std::string::npos)
            nxt_pos = def.length();

        std::string state{def.substr(pos, nxt_pos - pos)};
        callback(state);

        pos = nxt_pos + 1;
    }
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
PDA<InputSymbol, StateSymbol, StackSymbol>::PDA() {
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
PDA<InputSymbol, StateSymbol, StackSymbol>::PDA(const std::string &path) {
    initializeFromFile(path);
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::feed(const InputSymbol &input_symbol) {
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::dump() {
    std::cout << "States: {";
    for (auto state : state_alphabet) {
        std::cout << state << ",";
    }
    std::cout << "}" << std::endl;

    std::cout << "Input alphabet: {";
    for (auto input : input_alphabet) {
        std::cout << input << ",";
    }
    std::cout << "}" << std::endl;

    std::cout << "Stack alphabet: {";
    for (auto alpha : stack_alphabet) {
        std::cout << alpha << ",";
    }
    std::cout << "}" << std::endl;
}

template class PDA<char, std::string, char>;