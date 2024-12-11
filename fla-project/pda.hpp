#ifndef __PDA_HPP__
#define __PDA_HPP__

#include <set>
#include <tuple>
#include <stack>
#include <vector>
#include <string>
#include <unordered_map>

template<typename InputSymbol, typename StateSymbol, typename StackSymbol>
class PDA {
private:
    std::stack<StackSymbol> stack;
    StateSymbol current_state;

    StateSymbol starting_state;
    std::set<StateSymbol> terminating_states;
    StackSymbol stack_bottom;

    std::set<InputSymbol> input_alphabet;
    std::set<StateSymbol> state_alphabet;
    std::set<StackSymbol> stack_alphabet;

    std::unordered_map<
        std::tuple<StateSymbol, InputSymbol, StackSymbol>,
        std::tuple<StackSymbol, std::vector<StackSymbol>>
    > transitions;

    void initializeFromFile(const std::string &path);

public:
    PDA();
    PDA(const std::string &path);

    void feed(const InputSymbol &input_symbol);
    void simulate();

};

#endif