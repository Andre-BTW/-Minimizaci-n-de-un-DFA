#ifndef NFA_H
#define NFA_H

#include <map>
#include <set>

using State = int;
using StateSet = std::set<State>;

struct NFA
{
    State startState = 0;

    std::set<State> states;
    std::set<State> acceptStates;
    std::set<char> alphabet;

    // estado -> simbolo -> conjunto de estados destino
    std::map<State, std::map<char, StateSet>> transitions;

    // estado -> conjunto de estados alcanzables con epsilon
    std::map<State, StateSet> epsilonTransitions;

    void addState(State state);
    void addAcceptState(State state);

    void addTransition(
        State from,
        char symbol,
        State to
    );

    void addEpsilonTransition(
        State from,
        State to
    );
};

StateSet move(
    const NFA& nfa,
    const StateSet& states,
    char symbol
);

StateSet epsilonClosure(
    const NFA& nfa,
    const StateSet& states
);

void printNFA(const NFA& nfa);

#endif