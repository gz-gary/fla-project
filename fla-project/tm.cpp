#include "tm.hpp"
#include "utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

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
TM<InputSymbol, StateSymbol, TapeSymbol>::TM() : verbose(false) {
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
TM<InputSymbol, StateSymbol, TapeSymbol>::TM(const std::string &path) : verbose(false) {
    initializeFromFile(path);
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
bool TM<InputSymbol, StateSymbol, TapeSymbol>::accept(const std::string &str) {
    tapes.clear();
    tapes.resize(cnt_tapes);
    for (auto &tape : tapes) tape = std::deque<TapeSymbol>();

    pointers.clear();
    pointers.resize(cnt_tapes);
    for (auto &pointer : pointers) pointer = 0;

    leftmostIdx.clear();
    leftmostIdx.resize(cnt_tapes);
    for (auto &idx : leftmostIdx) idx = 0;

    steps = 0;
    current_state = starting_state;
    for (int i = 0; i < str.length(); ++i) {
        if (input_alphabet.find(str[i]) == input_alphabet.end()) {
            if (!verbose) throw std::runtime_error("illegal input");
            else {
                std::ostringstream ss;
                ss << "Input: " << str << std::endl;
                ss << "==================== ERR ====================" << std::endl;
                ss << "error: '" << str[i] << "' was not declared in the set of input symbols" << std::endl;
                ss << "Input: " << str << std::endl;
                ss << "       ";
                for (int j = 0; j < i; ++j) ss << " ";
                ss << "^" << std::endl;
                ss << "==================== END ====================";
                throw std::runtime_error(ss.str());
            }
        }
        tapes[0].push_back(str[i]);
    }
    if (verbose) {
        std::cout << "Input: " << str << std::endl;
        std::cout << "==================== RUN ====================" << std::endl;
    }

    dumpCurrentState();
    while (true) {
        if (terminating_states.find(current_state) != terminating_states.end())
            return true;
        std::vector<TapeSymbol> symb_read;
        for (int i = 0; i < cnt_tapes; ++i) {
            symb_read.push_back(readTape(i));
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
                writeTape(i, symb_write);
                movePointer(i, dir);
            }
            current_state = next_state;
            ++steps;
        } else {
            bool if_found = false;
            for (const auto& kvpair : transitions) {
                auto key = kvpair.first;
                auto value = kvpair.second;
                auto this_state = std::get<0>(key);
                if (this_state != current_state) continue;
                auto symb_read = std::get<1>(key);
                auto next_state = std::get<0>(value);
                auto symb_write_dir = std::get<1>(value);
                bool if_match = true;
                for (int i = 0; i < cnt_tapes; ++i) {
                    TapeSymbol symb = readTape(i);
                    if (!(
                        (symb_read[i] == '*' && symb != blank_symbol) ||
                        (symb_read[i] != '*' && symb == symb_read[i])
                    )) {
                        if_match = false;
                        break;
                    }
                }
                if (if_match) {
                    if_found = true;
                    for (int i = 0; i < cnt_tapes; ++i) {
                        TapeSymbol symb = readTape(i);
                        TapeSymbol symb_write;
                        TapeSymbol tr_symb_write = std::get<0>(symb_write_dir[i]);
                        TMDirection dir = std::get<1>(symb_write_dir[i]);
                        if (symb_read[i] != '*') symb_write = tr_symb_write;
                        else {
                            if (tr_symb_write == '*') symb_write = symb;
                            else symb_write = tr_symb_write;
                        }
                        writeTape(i, symb_write);
                        movePointer(i, dir);
                    }
                    current_state = next_state;
                    ++steps;
                    break;
                }
            }
            if (!if_found) return false;
        }
        dumpCurrentState();
    }

}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::dumpDefinition()
{
    if (!verbose) return;
    std::cerr << "States: {";
    for (auto state : state_alphabet) {
        std::cerr << state << ",";
    }
    std::cerr << "}" << std::endl;

    std::cerr << "Input alphabet: {";
    for (auto input : input_alphabet) {
        std::cerr << input << ",";
    }
    std::cerr << "}" << std::endl;

    std::cerr << "Tape alphabet: {";
    for (auto alpha : tape_alphabet) {
        std::cerr << alpha << ",";
    }
    std::cerr << "}" << std::endl;

    std::cerr << "Terminating states: {";
    for (auto state : terminating_states) {
        std::cerr << state << ",";
    }
    std::cerr << "}" << std::endl;

    std::cerr << "Starting from: " << starting_state << std::endl;
    std::cerr << "Blank symbol: " << blank_symbol << std::endl;
    std::cerr << "Count of tapes: " << cnt_tapes << std::endl;

    std::cerr << "Transitions: " << std::endl;
    for (auto iter = transitions.begin(); iter != transitions.end(); ++iter) {
        auto key = iter->first;
        auto value = iter->second;
        std::cerr << "("
                  << std::get<0>(key) << ", {";
        for (auto v : std::get<1>(key)) {
            std::cerr << v;
        }
        std::cerr << "}) ->";
        std::cerr << "("
                  << std::get<0>(value) << ", {";
        for (auto v : std::get<1>(value)) {
            std::cerr << "(" << std::get<0>(v) << ",";
            std::cerr << std::get<1>(v) << "), ";
        }
        std::cerr << "})" << std::endl;
    }
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::dumpCurrentState() {
    if (!verbose) return;
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
        if (tapes[i].empty()) {
            std::cout << gen_info_pref(idx) << iabs(pointers[i]) << std::endl;
            std::cout << gen_info_pref(tpe) << "_" << std::endl;
            std::cout << gen_info_pref(hd) << "^" << std::endl;
            continue;
        }
        if (pointers[i] < leftmostIdx[i]) {
            int cnt_space = leftmostIdx[i] - pointers[i];
            int idx_l = pointers[i], idx_r = leftmostIdx[i] + tapes[i].size();
            auto gen_idx_space = fillUpSpaces(
                std::max(
                    digitsLength(iabs(idx_r - 1)),
                    digitsLength(iabs(idx_l))
                ) + 1
            );
            std::cout << gen_info_pref(idx);
            for (int j = idx_l; j < idx_r; ++j)
                std::cout << gen_idx_space(std::to_string(iabs(j)));
            std::cout << std::endl;
            
            std::cout << gen_info_pref(tpe);
            for (int j = 0; j < cnt_space; ++j) 
                std::cout << gen_idx_space(std::string(1, blank_symbol));
            for (int j = 0; j < tapes[i].size(); ++j)
                std::cout << gen_idx_space(std::string(1, tapes[i][j]));
            std::cout << std::endl;

            std::cout << gen_info_pref(hd);
            std::cout << "^" << std::endl;
        } else if (pointers[i] - leftmostIdx[i] >= tapes[i].size()) {
            int cnt_space = (pointers[i] - leftmostIdx[i]) - tapes[i].size() + 1;
            int idx_l = leftmostIdx[i], idx_r = pointers[i] + 1;
            auto gen_idx_space = fillUpSpaces(
                std::max(
                    digitsLength(iabs(idx_r - 1)),
                    digitsLength(iabs(idx_l))
                ) + 1
            );
            std::cout << gen_info_pref(idx);
            for (int j = idx_l; j < idx_r; ++j)
                std::cout << gen_idx_space(std::to_string(iabs(j)));
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
            int idx_l = leftmostIdx[i], idx_r = leftmostIdx[i] + tapes[i].size();
            auto gen_idx_space = fillUpSpaces(
                std::max(
                    digitsLength(iabs(idx_r - 1)),
                    digitsLength(iabs(idx_l))
                ) + 1
            );
            std::cout << gen_info_pref(idx);
            for (int j = idx_l; j < idx_r; ++j)
                std::cout << gen_idx_space(std::to_string(iabs(j)));
            std::cout << std::endl;
            
            std::cout << gen_info_pref(tpe);
            for (int j = 0; j < tapes[i].size(); ++j)
                std::cout << gen_idx_space(std::string(1, tapes[i][j]));
            std::cout << std::endl;

            std::cout << gen_info_pref(hd);
            for (int j = 0; j < pointers[i] - leftmostIdx[i]; ++j)
                std::cout << gen_idx_space(" ");
            std::cout << "^" << std::endl;
        }
    }
    std::cout << "-------------------------------" << std::endl;
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
std::string TM<InputSymbol, StateSymbol, TapeSymbol>::getResult() const {
    std::string result{};
    for (int i = 0; i < tapes[0].size(); ++i) {
        if (tapes[0][i] == blank_symbol) result += " ";
        else result += tapes[0][i];
    }
    if (verbose) {
        std::ostringstream ss;
        ss << "Result: " << result << std::endl << "" << "==================== END ====================";
        result = ss.str();
    }
    return result;
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

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
TapeSymbol TM<InputSymbol, StateSymbol, TapeSymbol>::readTape(int i) const {
    if (tapes[i].empty()) return blank_symbol;
    else if (pointers[i] < leftmostIdx[i]) return blank_symbol;
    else if (pointers[i] - leftmostIdx[i] >= tapes[i].size()) return blank_symbol;
    else return tapes[i][pointers[i] - leftmostIdx[i]];
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::writeTape(int i, TapeSymbol symb_write) {
    if (tapes[i].empty()) {
        if (symb_write != blank_symbol) {
            tapes[i].push_back(symb_write);
            leftmostIdx[i] = pointers[i];
        }
        return;
    }
    if (pointers[i] < leftmostIdx[i]) {
        if (symb_write != blank_symbol) { // write non-blank symbol to the left
            int cnt_space = leftmostIdx[i] - pointers[i];
            for (int j = 0; j < cnt_space; ++j) tapes[i].push_front(blank_symbol);
            leftmostIdx[i] = pointers[i];
            tapes[i][0] = symb_write;
        }
    } else if ((pointers[i] - leftmostIdx[i]) >= tapes[i].size()) {
        if (symb_write != blank_symbol) { // write non-blank symbol to the right
            int cnt_space = (pointers[i] - leftmostIdx[i]) - tapes[i].size() + 1;
            for (int j = 0; j < cnt_space; ++j) tapes[i].push_back(blank_symbol);
            tapes[i][pointers[i] - leftmostIdx[i]] = symb_write;
        }
    } else {
        tapes[i][pointers[i] - leftmostIdx[i]] = symb_write;
        if (symb_write == blank_symbol) {
            if (pointers[i] == leftmostIdx[i]) {
                while (!tapes[i].empty() && tapes[i].front() == blank_symbol) {
                    tapes[i].pop_front();
                    ++leftmostIdx[i];
                }
            } else if (pointers[i] - leftmostIdx[i] == tapes[i].size() - 1) {
                while (!tapes[i].empty() && tapes[i].back() == blank_symbol)
                    tapes[i].pop_back();
            }
        }
    }
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
void TM<InputSymbol, StateSymbol, TapeSymbol>::movePointer(int i, const TMDirection &dir) {
    pointers[i] += dir;
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