#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <functional>

bool prefix_of(const std::string &prefix, const std::string &another);
bool suffix_of(const std::string &suffix, const std::string &another);
void remove_surrounding_spaces(std::string &str);
void remove_comment(std::string &str);
int iabs(int x);
void remove_prefix(const std::string &prefix, std::string &another);
void parseSetDef(std::string def, std::function<void(const std::string &)> callback);
int digitsLength(int x);
std::function<std::string(std::string)> fillUpSpaces(int cnt_total);

#endif