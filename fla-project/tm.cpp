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

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
TM<InputSymbol, StateSymbol, TapeSymbol>::TM() : verbose(false) {
}

template <typename InputSymbol, typename StateSymbol, typename TapeSymbol>
TM<InputSymbol, StateSymbol, TapeSymbol>::TM(const std::string &path) : verbose(false) {
    initializeFromFile(path);
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

template class TM<char, std::string, char>;