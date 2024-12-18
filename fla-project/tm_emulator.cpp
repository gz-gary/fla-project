#include "tm.hpp"
#include "utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

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

template class TM<char, std::string, char>;