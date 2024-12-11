#include <string>

inline bool prefix_of(const std::string &prefix, const std::string &another) {
    return (
        another.length() >= prefix.length()
        && another.substr(0, prefix.length()) == prefix
    );
}

inline void remove_prefix(const std::string &prefix, std::string &another) {
    if (prefix_of(prefix, another)) {
        int len_prefix = prefix.length();
        another = another.substr(len_prefix, another.length() - len_prefix);
    }
}