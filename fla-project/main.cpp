#include <string>
#include <iostream>
#include <cassert>
#include "parseargs.hpp"
#include "pda.hpp"
#include "tm.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    std::cout << std::unitbuf;

    std::string help_information = R"(usage:   fla [-h|--help]
         fla [-v|--verbose] <tm> <input>
         fla [tm|pda] <input>)";
    std::string invalid_information = R"(Invalid arguments! Please use fla -h to see usage.)";

    ParseArgsResult result = parseArgs(argc, argv);
    if (result.invalid) {
        std::cerr << invalid_information << std::endl;
        return 1;
    }
    if (result.help) {
        std::cout << help_information << std::endl;
        return 0;
    }
    try {
        if (result.tm) {
            TM<char, std::string, char> tm{result.path};
            if (result.verbose) tm.verbose = true;
            tm.accept(result.input);
            std::cout << tm.getResult() << std::endl;
            return 0;
        } else if (result.pda) {
            PDA<char, std::string, char> pda{result.path};
            std::cout << (pda.accept(result.input) ? "true" : "false") << std::endl;
            return 0;
        }
    } catch (std::runtime_error re) {
        std::cerr << re.what() << std::endl;
        return 1;
    }
    assert(0);
    return 2;
}