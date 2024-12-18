#include "tm.hpp"
#include "utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeFromFile(const std::string &path) {
    std::ifstream file{path};

    if (!file.is_open())
        throw std::runtime_error("Fail to open file when initializing TM");

    std::string line;
    while (std::getline(file, line)) {
        remove_surrounding_spaces(line);
        remove_comment(line);
        remove_surrounding_spaces(line);
        if (prefix_of(";", line)) {
            // ignore comment
            continue;
        } else if (prefix_of("#Q = ", line)) {
            initializeStateAlphabet(line);
        } else if (prefix_of("#S = ", line)) {
            initializeInputAlphabet(line);
        } else if (prefix_of("#G = ", line)) {
            initializeTapeAlphabet(line);
        } else if (prefix_of("#F = ", line)) {
            initializeTerminatingStates(line);
        } else if (prefix_of("#q0 = ", line)) {
            initializeStartingState(line);
        } else if (prefix_of("#B = ", line)) {
            initializeBlankSymbol(line);
        } else if (prefix_of("#N = ", line)) {
            initializeCntTapes(line);
        } else if (line == "") {
            continue;
        } else { // these lines are all transition definitions
            initializeTransition(line);
        }
    }

    file.close();
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeStateAlphabet(std::string def) {
    remove_prefix("#Q = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            state_alphabet.insert(state);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeInputAlphabet(std::string def) {
    remove_prefix("#S = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            if (state.length() > 1) 
                throw std::runtime_error("Input alphabet's length is more than 1");
            input_alphabet.insert(state[0]);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeTapeAlphabet(std::string def) {
    remove_prefix("#G = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            if (state.length() > 1) 
                throw std::runtime_error("Tape alphabet's length is more than 1");
            tape_alphabet.insert(state[0]);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeTerminatingStates(std::string def) {
    remove_prefix("#F = ", def);
    parseSetDef(
        def,
        [this](const std::string &state) {
            terminating_states.insert(state);
        }
    );
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeStartingState(std::string def) {
    remove_prefix("#q0 = ", def);
    if (state_alphabet.find(def) == state_alphabet.end())
        throw std::runtime_error("Starting state is not in state alphabet");
    starting_state = def;
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeBlankSymbol(std::string def) {
    remove_prefix("#B = ", def);
    if (
        def.length() > 1 || 
        tape_alphabet.find(def[0]) == tape_alphabet.end()
    ) throw std::runtime_error("Blank symbol is not in tape alphabet");
    blank_symbol = def[0];
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeCntTapes(std::string def) {
    remove_prefix("#N = ", def);
    try {
        cnt_tapes = std::stoi(def);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Count of tapes is not a valid integer");
    }
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeTransition(std::string def) {
    std::istringstream stream{def};
    std::string 
        tr_current_state,
        tr_tape_read,
        tr_tape_write,
        tr_direction,
        tr_next_state;
    stream >> tr_current_state >> tr_tape_read >> tr_tape_write >> tr_direction >> tr_next_state;

    if (state_alphabet.find(tr_current_state) == state_alphabet.end())
        throw std::runtime_error("Invalid transition with invalid current state");
    if (tr_tape_read.length() == cnt_tapes) {
        for (int i = 0; i < cnt_tapes; ++i)
            if (tr_tape_read[i] != '*' && tape_alphabet.find(tr_tape_read[i]) == tape_alphabet.end())
                throw std::runtime_error("Invalid transition with invalid tape read");
    } else throw std::runtime_error("Invalid transition with invalid tape read");
    if (tr_tape_write.length() == cnt_tapes) {
        for (int i = 0; i < cnt_tapes; ++i)
            if (tr_tape_write[i] != '*' && tape_alphabet.find(tr_tape_write[i]) == tape_alphabet.end())
                throw std::runtime_error("Invalid transition with invalid tape write");
    } else throw std::runtime_error("Invalid transition with invalid tape write");
    if (tr_direction.length() == cnt_tapes) {
        for (int i = 0; i < cnt_tapes; ++i)
            if (!TMDirection::isValid(tr_direction[i]))
                throw std::runtime_error("Invalid transition with invalid direction");
    } else throw std::runtime_error("Invalid transition with invalid direction");
    if (state_alphabet.find(tr_next_state) == state_alphabet.end())
        throw std::runtime_error("Invalid transition with invalid next state");

    std::vector<TapeSymbol> symb_read;
    for (int i = 0; i < cnt_tapes; ++i)
        symb_read.push_back(tr_tape_read[i]);
    
    std::vector<std::tuple<TapeSymbol, TMDirection>> symb_write_dir;
    for (int i = 0; i < cnt_tapes; ++i)
        symb_write_dir.push_back(
            {
            tr_tape_write[i],
            TMDirection(tr_direction[i])
            }
        );

    std::tuple<StateSymbol, std::vector<TapeSymbol>> key{tr_current_state, symb_read};
    std::tuple<StateSymbol, std::vector<std::tuple<TapeSymbol, TMDirection>>> value{tr_next_state, symb_write_dir};
    addTransition(key, value);
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::addTransition(std::tuple<StateSymbol, std::vector<TapeSymbol>> key, std::tuple<StateSymbol, std::vector<std::tuple<TapeSymbol, TMDirection>>> value) {
    transitions[key] = value;
}

bool TMDirection::isValid(char d) {
    return d == 'l' || d == 'r' || d == '*';
}

TMDirection::TMDirection(char d) {
    if (d == 'l') dir = LEFT;
    if (d == 'r') dir = RIGHT;
    if (d == '*') dir = IDLE;
}

template class TM<char, std::string, char>;