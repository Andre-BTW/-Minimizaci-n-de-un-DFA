#ifndef DFA_H
#define DFA_H

#include "nfa.h"

#include <map>
#include <set>
#include <string>
#include <vector>


struct DFA
{
    StateSet startState;

    std::set<StateSet> states;
    std::set<StateSet> acceptStates;

    std::set<char> alphabet;

    // subconjunto -> simbolo -> subconjunto destino
    std::map<StateSet, std::map<char, StateSet>> transitions;
};


DFA subsetConstruction(const NFA& nfa);

DFA removeUnreachableStates(const DFA& dfa);

struct MinDFA
{
    int startState;

    std::set<int> states;
    std::set<int> acceptStates;

    std::set<char> alphabet;

    std::map<int, std::map<char, int>> transitions;

    // Cada estado del DFA minimizado representa
    // un bloque de estados del DFA original.
    std::map<int, std::set<StateSet>> partitions;
};

MinDFA minimizeDFA(const DFA& dfa);

bool testString(
    const MinDFA& dfa,
    const std::string& input
);

void runTestSuite(
    const MinDFA& dfa,
    const std::vector<std::string>& acceptTests,
    const std::vector<std::string>& rejectTests
);

void printMinDFA(const MinDFA& dfa);

DFA completeDFA(const DFA& dfa);

void printStateSet(const StateSet& states);

void printDFA(const DFA& dfa);

#endif