#ifndef REGEX_H
#define REGEX_H

#include "nfa.h"

#include <string>
#include <vector>


enum class RegexType
{
    Char,
    Concat,
    Or,
    Star,
    Plus,
    Question
};


struct RegexItem
{
    char value;
    RegexType type;
};


std::vector<RegexItem> parseRegex(
    const std::string& regex
);


NFA regexToNFA(
    const std::string& regex
);


#endif