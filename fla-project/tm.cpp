#include "tm.hpp"
#include "utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const TMDirection &dir) {
    std::string result;
    switch (dir.dir) {
    case TMDirection::LEFT:
        result = "LEFT";
        break;
    case TMDirection::RIGHT:
        result = "RIGHT";
        break;
    case TMDirection::IDLE:
        result = "IDLE";
        break;
    }
    os << result;
    return os;
}

int& operator+=(int& p, const TMDirection &dir) {
    switch (dir.dir) {
    case TMDirection::LEFT:
        --p;
        break;
    case TMDirection::RIGHT:
        ++p;
        break;
    case TMDirection::IDLE:
        break;
    }
    return p;
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
TM<InputSymbol, StateSymbol, TapeSymbol>::TM() {
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
TM<InputSymbol, StateSymbol, TapeSymbol>::TM(const std::string &path) {
    initializeFromFile(path);
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
bool TM<InputSymbol, StateSymbol, TapeSymbol>::accept(const std::string &str) {
    tapes.clear();
    tapes.resize(cnt_tapes);
    for (auto tape : tapes) tape = std::deque<TapeSymbol>();

    pointers.clear();
    pointers.resize(cnt_tapes);
    for (auto pointer : pointers) pointer = 0;

    steps = 0;
    current_state = starting_state;
    for (int i = 0; i < str.length(); ++i) {
        tapes[0].push_back(str[i]);
    }

    dumpCurrentState();
    while (true) {
        if (terminating_states.find(current_state) != terminating_states.end())
            return true;
        std::vector<TapeSymbol> symb_read;
        for (int i = 0; i < cnt_tapes; ++i) {
            TapeSymbol symb
                = pointers[i] >= 0 && pointers[i] < tapes[i].size() ? tapes[i][pointers[i]] : blank_symbol;
            symb_read.push_back(symb);
        }
        std::tuple<StateSymbol, std::vector<TapeSymbol>> key{current_state, symb_read};
        auto iter = transitions.find(key);
        if (iter != transitions.end()) {
            auto value = iter->second;
            auto next_state = std::get<0>(value);
            auto symb_write_dir = std::get<1>(value);
            for (int i = 0; i < cnt_tapes; ++i) {
                TapeSymbol symb_write = std::get<0>(symb_write_dir[i]);
                TMDirection dir = std::get<1>(symb_write_dir[i]);
                if (pointers[i] < 0) {
                    if (symb_write != blank_symbol) {
                        int cnt_space = 0 - pointers[i];
                        for (int j = 0; j < cnt_space; ++j) tapes[i].push_front(blank_symbol);
                        pointers[i] = 0;
                    }
                } else if (pointers[i] >= tapes[i].size()) {
                    if (symb_write != blank_symbol) {
                        int cnt_space = pointers[i] - tapes[i].size() + 1;
                        for (int j = 0; j < cnt_space; ++j) tapes[i].push_back(blank_symbol);
                        pointers[i] = tapes[i].size() - 1;
                    }
                } else {
                }
                if (symb_write != blank_symbol) {
                    tapes[i][pointers[i]] = symb_write;
                }
                pointers[i] += dir;
            }
            current_state = next_state;
            ++steps;
        } else return false;
        dumpCurrentState();
    }

}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::dumpDefinition()
{
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

    std::cout << "Tape alphabet: {";
    for (auto alpha : tape_alphabet) {
        std::cout << alpha << ",";
    }
    std::cout << "}" << std::endl;

    std::cout << "Terminating states: {";
    for (auto state : terminating_states) {
        std::cout << state << ",";
    }
    std::cout << "}" << std::endl;

    std::cout << "Starting from: " << starting_state << std::endl;
    std::cout << "Blank symbol: " << blank_symbol << std::endl;
    std::cout << "Count of tapes: " << cnt_tapes << std::endl;

    std::cout << "Transitions: " << std::endl;
    for (auto iter = transitions.begin(); iter != transitions.end(); ++iter) {
        auto key = iter->first;
        auto value = iter->second;
        std::cout << "("
                  << std::get<0>(key) << ", {";
        for (auto v : std::get<1>(key)) {
            std::cout << v;
        }
        std::cout << "}) ->";
        std::cout << "("
                  << std::get<0>(value) << ", {";
        for (auto v : std::get<1>(value)) {
            std::cout << "(" << std::get<0>(v) << ",";
            std::cout << std::get<1>(v) << "), ";
        }
        std::cout << "})" << std::endl;
    }
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::dumpCurrentState() {
    auto gen_info_pref_space = fillUpSpaces(10);
    auto gen_info_pref = [&gen_info_pref_space](std::string pref) {
        return gen_info_pref_space(pref) + ": ";
    };

    std::cout << gen_info_pref("Step") << steps << std::endl;
    std::cout << gen_info_pref("State") << current_state << std::endl;
    for (int i = 0; i < cnt_tapes; ++i) {
        std::string idx = "Index" + std::to_string(i);
        std::string tpe = "Tape" + std::to_string(i);
        std::string hd = "Head" + std::to_string(i);
        if (pointers[i] < 0) {
            int cnt_space = 0 - pointers[i];
            auto gen_idx_space = fillUpSpaces(digitsLength(tapes[i].size() + cnt_space - 1) + 1);
            std::cout << gen_info_pref(idx);
            for (int j = 0; j < tapes[i].size() + cnt_space; ++j)
                std::cout << gen_idx_space(std::to_string(j));
            std::cout << std::endl;
            
            std::cout << gen_info_pref(tpe);
            for (int j = 0; j < cnt_space; ++j) 
                std::cout << gen_idx_space(std::string(1, blank_symbol));
            for (int j = 0; j < tapes[i].size(); ++j)
                std::cout << gen_idx_space(std::string(1, tapes[i][j]));
            std::cout << std::endl;

            std::cout << gen_info_pref(hd);
            std::cout << "^" << std::endl;
        } else if (pointers[i] >= tapes[i].size()) {
            int cnt_space = pointers[i] - tapes[i].size() + 1;
            auto gen_idx_space = fillUpSpaces(digitsLength(tapes[i].size() + cnt_space - 1) + 1);
            std::cout << gen_info_pref(idx);
            for (int j = 0; j < tapes[i].size() + cnt_space; ++j)
                std::cout << gen_idx_space(std::to_string(j));
            std::cout << std::endl;
            
            std::cout << gen_info_pref(tpe);
            for (int j = 0; j < tapes[i].size(); ++j)
                std::cout << gen_idx_space(std::string(1, tapes[i][j]));
            for (int j = 0; j < cnt_space; ++j)
                std::cout << gen_idx_space(std::string(1, blank_symbol));
            std::cout << std::endl;

            std::cout << gen_info_pref(hd);
            for (int j = 0; j < tapes[i].size() + cnt_space - 1; ++j)
                std::cout << gen_idx_space(" ");
            std::cout << "^" << std::endl;
        } else {
            auto gen_idx_space = fillUpSpaces(digitsLength(tapes[i].size()) + 1);
            std::cout << gen_info_pref(idx);
            for (int j = 0; j < tapes[i].size(); ++j)
                std::cout << gen_idx_space(std::to_string(j));
            std::cout << std::endl;
            
            std::cout << gen_info_pref(tpe);
            for (int j = 0; j < tapes[i].size(); ++j)
                std::cout << gen_idx_space(std::string(1, tapes[i][j]));
            std::cout << std::endl;

            std::cout << gen_info_pref(hd);
            for (int j = 0; j < pointers[i]; ++j)
                std::cout << gen_idx_space(" ");
            std::cout << "^" << std::endl;
        }
    }
    std::cout << "-------------------------------" << std::endl;
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::initializeFromFile(const std::string &path) {
    std::ifstream file{path};

    if (!file.is_open())
        throw std::runtime_error("Fail to open file when initializing TM");

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