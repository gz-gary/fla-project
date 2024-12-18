#include "utils.hpp"
#include "parseargs.hpp"
#include <vector>

ParseArgsResult parseArgs(int argc, char *argv[]) {
    ParseArgsResult result{};
    std::vector<std::string> arg;
    arg.resize(argc);
    for (int i = 0; i < argc; ++i) arg[i] = std::string(argv[i]);
    if (argc == 2) {
        if (arg[1] == "-h" || arg[1] == "--help") result.help = true;
        else result.invalid = true;
    } else if (argc == 3) {
        if (suffix_of(".tm", arg[1])) {
            result.tm = true;
            result.path = arg[1];
        } else if (suffix_of(".pda", arg[1])) {
            result.pda = true;
            result.path = arg[1];
        } else result.invalid = true;
        result.input = arg[2];
    } else if (argc == 4) {
        if (arg[1] == "-v" || arg[1] == "--verbose") result.verbose = true;
        if (suffix_of(".tm", arg[2])) {
            result.tm = true;
            result.path = arg[2];
        } else result.invalid = true;
        result.input = arg[3];
    } else result.invalid = true;
    return result;
}

ParseArgsResult::ParseArgsResult() :
    help(false),
    verbose(false),
    path(""),
    input(""),
    tm(false),
    pda(false),
    invalid(false)
{
}
