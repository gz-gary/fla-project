#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <functional>

bool prefix_of(const std::string &prefix, const std::string &another);
void remove_prefix(const std::string &prefix, std::string &another);
void parseSetDef(std::string def, std::function<void(const std::string &)> callback);

#endif