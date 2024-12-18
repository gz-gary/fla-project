#ifndef __PARSEARGS_HPP__
#define __PARSEARGS_HPP__

#include <string>

class ParseArgsResult {
public:
    bool help;
    bool verbose;
    std::string path;
    std::string input;
    bool tm;
    bool pda;
    bool invalid;

    ParseArgsResult();
};

ParseArgsResult parseArgs(int argc, char* argv[]);

#endif