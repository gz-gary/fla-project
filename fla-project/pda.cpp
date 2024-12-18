#include "pda.hpp"
#include "utils.hpp"

#include <stack>
#include <string>
#include <sstream>
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
        } else if (prefix_of("#F = ", line)) {
            initializeTerminatingStates(line);
        } else if (prefix_of("#q0 = ", line)) {
            initializeStartingState(line);
        } else if (prefix_of("#z0 = ", line)) {
            initializeStackBottom(line);
        } else if (line == "") {
            continue;
        } else { // these lines are all transition definitions
            initializeTransition(line);
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
                throw std::runtime_error("syntax error");
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
                throw std::runtime_error("syntax error");
            stack_alphabet.insert(state[0]);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeTerminatingStates(std::string def) {
    remove_prefix("#F = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            terminating_states.insert(state);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeStartingState(std::string def) {
    remove_prefix("#q0 = ", def);
    if (state_alphabet.find(def) == state_alphabet.end())
        throw std::runtime_error("syntax error");
    starting_state = def;
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeStackBottom(std::string def) {
    remove_prefix("#z0 = ", def);
    if (def.length() > 1)
        throw std::runtime_error("syntax error");
    if (stack_alphabet.find(def[0]) == stack_alphabet.end())
        throw std::runtime_error("syntax error");
    stack_bottom = def[0];
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::initializeTransition(std::string def) {
    std::istringstream stream{def};
    std::string 
        tr_current_state,
        tr_input,
        tr_stack_pop,
        tr_next_state,
        tr_stack_push;
    stream >> tr_current_state >> tr_input >> tr_stack_pop >> tr_next_state >> tr_stack_push;

    if (state_alphabet.find(tr_current_state) == state_alphabet.end())
        throw std::runtime_error("syntax error");
    if (
        tr_input.length() > 1 ||
        (
        input_alphabet.find(tr_input[0]) == input_alphabet.end()
        && tr_input[0] != '_'
        )
    )
        throw std::runtime_error("syntax error");
    if (
        tr_stack_pop.length() > 1 ||
        stack_alphabet.find(tr_stack_pop[0]) == stack_alphabet.end()
    )
        throw std::runtime_error("syntax error");
    if (state_alphabet.find(tr_next_state) == state_alphabet.end())
        throw std::runtime_error("syntax error");

    std::tuple<StateSymbol, InputSymbol, StackSymbol> key{tr_current_state, tr_input[0], tr_stack_pop[0]};
    std::vector<StackSymbol> symb_push;
    if (tr_stack_push != "_") {
        for (int i = tr_stack_push.length() - 1; i >= 0; --i) {
            if (stack_alphabet.find(tr_stack_push[i]) == stack_alphabet.end())
                throw std::runtime_error("syntax error");
            symb_push.push_back(tr_stack_push[i]);
        }
    }
    std::tuple<StateSymbol, std::vector<StackSymbol>> value{tr_next_state, symb_push};

    addTransition(key, value);
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::addTransition(std::tuple<StateSymbol, InputSymbol, StackSymbol> key, std::tuple<StateSymbol, std::vector<StackSymbol>> value) {
    transitions[key] = value;
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
PDA<InputSymbol, StateSymbol, StackSymbol>::PDA() {
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
PDA<InputSymbol, StateSymbol, StackSymbol>::PDA(const std::string &path) {
    initializeFromFile(path);
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
bool PDA<InputSymbol, StateSymbol, StackSymbol>::accept(const std::string &str) {
    StateSymbol current_state = starting_state;
    std::stack<StackSymbol> stack{};
    stack.push(stack_bottom);

    for (int i = 0; i <= str.length(); ++i) {
        StackSymbol stk_top = stack.top();
        InputSymbol symb_input = i == str.length() ? '_' : str[i];
        std::tuple<StateSymbol, InputSymbol, StackSymbol> key{current_state, symb_input, stk_top};
        auto iter = transitions.find(key);
        if (iter != transitions.end()) {
            auto value = iter->second;
            auto next_state = std::get<0>(value);
            auto stk_push = std::get<1>(value);

            current_state = next_state;
            stack.pop();
            for (auto symb : stk_push) stack.push(symb);
        } else return false;
    }

    if (terminating_states.find(current_state) != terminating_states.end())
        return true;
    else
        return false;
}

template <typename InputSymbol, typename StateSymbol, typename StackSymbol>
void PDA<InputSymbol, StateSymbol, StackSymbol>::dumpDefinition() {
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

    std::cout << "Terminating states: {";
    for (auto state : terminating_states) {
        std::cout << state << ",";
    }
    std::cout << "}" << std::endl;

    std::cout << "Starting from: " << starting_state << std::endl;
    std::cout << "Stack bottom: " << stack_bottom << std::endl;

    std::cout << "Transitions: " << std::endl;
    for (auto iter = transitions.begin(); iter != transitions.end(); ++iter) {
        auto key = iter->first;
        auto value = iter->second;
        std::cout << "("
                  << std::get<0>(key) << ", "
                  << std::get<1>(key) << ", "
                  << std::get<2>(key) << ") -> ";
        std::cout << "("
                  << std::get<0>(value) << ", ";
        for (auto c : std::get<1>(value)) {
            std::cout << c;
        }
        std::cout << ")" << std::endl;
    }
}

template class PDA<char, std::string, char>;