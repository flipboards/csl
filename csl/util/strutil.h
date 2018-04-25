#pragma once

#ifndef CSL_UTIL_STRUTIL_H
#define CSL_UTIL_STRUTIL_H

#include <string>
#include <vector>

std::vector<std::string> split(const std::string&, char);
std::string join(const std::vector<std::string>&, char);
std::string join(const std::vector<std::string>&, const std::string&);

std::string format(const std::string& base, ...);


#endif // !CSL_UTIL_STRUTIL_H
