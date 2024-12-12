#ifndef __PDA_HPP__
#define __PDA_HPP__

#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <string>

template<typename InputSymbol, typename StateSymbol, typename StackSymbol>
class PDA {
private:

    StateSymbol starting_state;
    std::set<StateSymbol> terminating_states;
    StackSymbol stack_bottom;

    std::set<InputSymbol> input_alphabet;
    std::set<StateSymbol> state_alphabet;
    std::set<StackSymbol> stack_alphabet;

    std::map<
        std::tuple<StateSymbol, InputSymbol, StackSymbol>,
        std::tuple<StateSymbol, std::vector<StackSymbol>>
    > transitions;

    void initializeFromFile(const std::string &path);
    void initializeStateAlphabet(std::string def);
    void initializeInputAlphabet(std::string def);
    void initializeStackAlphabet(std::string def);
    void initializeTerminatingStates(std::string def);
    void initializeStartingState(std::string def);
    void initializeStackBottom(std::string def);
    void initializeTransition(std::string def);
    void addTransition(
        std::tuple<StateSymbol, InputSymbol, StackSymbol> key,
        std::tuple<StateSymbol, std::vector<StackSymbol>> value);

public:
    PDA();
    PDA(const std::string &path);

    bool accept(const std::string &str);
    void dumpDefinition();
    void dumpCurrentState();

};

#endif