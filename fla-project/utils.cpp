#include "utils.hpp"
#include <stdexcept>

bool prefix_of(const std::string &prefix, const std::string &another) {
    return (
        another.length() >= prefix.length()
        && another.substr(0, prefix.length()) == prefix
    );
}

bool suffix_of(const std::string &suffix, const std::string &another) {
    return (
        another.length() >= suffix.length()
        && another.substr(another.length() - suffix.length(), suffix.length()) == suffix
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

int digitsLength(int x) {
    if (x <= 0) return 1;
    else if (x < 10) return 1;
    else return digitsLength(x / 10) + 1;
}

std::function<std::string(std::string)> fillUpSpaces(int cnt_total) {
    return [cnt_total](std::string pref) {
        int pref_len = pref.length();
        std::string result{pref};
        for (int i = 0; i < cnt_total - pref_len; ++i) {
            result += ' ';
        }
        return result;
    };
}
