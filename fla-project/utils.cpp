#include "utils.hpp"
#include <stdexcept>

bool prefix_of(const std::string &prefix, const std::string &another) {
    return (
        another.length() >= prefix.length()
        && another.substr(0, prefix.length()) == prefix
    );
}

void remove_prefix(const std::string &prefix, std::string &another) {
    if (prefix_of(prefix, another)) {
        int len_prefix = prefix.length();
        another = another.substr(len_prefix, another.length() - len_prefix);
    }
}

void parseSetDef(std::string def, std::function<void(const std::string &)> callback) {
    if (!
    ( 
      def.length() >= 2
      && def[0] == '{'
      && def[def.length() - 1] == '}'
    )
    ) throw std::runtime_error("Syntax error on PDA set definition");

    def = def.substr(1, def.length() - 2);
    size_t pos = 0;
    while (pos < def.length()) {
        size_t nxt_pos = def.find(",", pos);
        if (nxt_pos == std::string::npos)
            nxt_pos = def.length();

        std::string state{def.substr(pos, nxt_pos - pos)};
        callback(state);

        pos = nxt_pos + 1;
    }
}
