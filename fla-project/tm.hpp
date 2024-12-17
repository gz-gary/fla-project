#ifndef __TM_HPP__
#define __TM_HPP__

#include <set>
#include <map>
#include <deque>
#include <tuple>
#include <vector>
#include <string>

class TMDirection {
public:
    enum {
        LEFT,
        RIGHT,
        IDLE
    } dir;

    static bool isValid(char d);
    TMDirection(char d);
};

template<typename InputSymbol, typename StateSymbol, typename TapeSymbol>
class TM {

public:

    TM();
    TM(const std::string &path);

    bool accept(const std::string &str);
    void dumpDefinition();
    void dumpCurrentState();

private:

    std::vector<std::deque<TapeSymbol>> tapes;
    int steps;
    std::vector<int> pointers;
    StateSymbol current_state;

    StateSymbol starting_state;
    std::set<StateSymbol> terminating_states;
    TapeSymbol blank_symbol;
    int cnt_tapes;

    std::set<InputSymbol> input_alphabet;
    std::set<StateSymbol> state_alphabet;
    std::set<TapeSymbol> tape_alphabet;

    std::map<
        std::tuple<StateSymbol, std::vector<TapeSymbol>>,
        std::tuple<StateSymbol, std::vector<std::tuple<TapeSymbol, TMDirection>>>
    > transitions;

    void initializeFromFile(const std::string &path);
    void initializeStateAlphabet(std::string def);
    void initializeInputAlphabet(std::string def);
    void initializeTapeAlphabet(std::string def);
    void initializeTerminatingStates(std::string def);
    void initializeStartingState(std::string def);
    void initializeBlankSymbol(std::string def);
    void initializeCntTapes(std::string def);
    void initializeTransition(std::string def);
    void addTransition(
        std::tuple<StateSymbol, std::vector<TapeSymbol>> key,
        std::tuple<StateSymbol, std::vector<std::tuple<TapeSymbol, TMDirection>>> value);

    TapeSymbol readTape(int i) const;
    void writeTape(int i, TapeSymbol symb_write);
    void movePointer(int i, const TMDirection &dir);

};

#endif

