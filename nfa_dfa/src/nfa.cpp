#include "nfa.h"

#include <iostream>
#include <stack>


void NFA::addState(State state)
{
    states.insert(state);
}


void NFA::addAcceptState(State state)
{
    states.insert(state);
    acceptStates.insert(state);
}


void NFA::addTransition(
    State from,
    char symbol,
    State to
)
{
    states.insert(from);
    states.insert(to);

    alphabet.insert(symbol);

    transitions[from][symbol].insert(to);
}


void NFA::addEpsilonTransition(
    State from,
    State to
)
{
    states.insert(from);
    states.insert(to);

    epsilonTransitions[from].insert(to);
}


StateSet move(
    const NFA& nfa,
    const StateSet& states,
    char symbol
)
{
    StateSet result;

    for (State state : states)
    {
        auto stateIt = nfa.transitions.find(state);

        if (stateIt == nfa.transitions.end())
        {
            continue;
        }

        auto symbolIt = stateIt->second.find(symbol);

        if (symbolIt == stateIt->second.end())
        {
            continue;
        }

        result.insert(
            symbolIt->second.begin(),
            symbolIt->second.end()
        );
    }

    return result;
}


StateSet epsilonClosure(
    const NFA& nfa,
    const StateSet& states
)
{
    StateSet closure = states;

    std::stack<State> pending;

    for (State state : states)
    {
        pending.push(state);
    }

    while (!pending.empty())
    {
        State current = pending.top();
        pending.pop();

        auto it = nfa.epsilonTransitions.find(current);

        if (it == nfa.epsilonTransitions.end())
        {
            continue;
        }

        for (State next : it->second)
        {
            // insert devuelve false si el estado ya existia
            if (closure.insert(next).second)
            {
                pending.push(next);
            }
        }
    }

    return closure;
}


void printNFA(const NFA& nfa)
{
    std::cout << "===== NFA =====\n";

    std::cout << "Estado inicial: "
              << nfa.startState
              << "\n";

    std::cout << "Estados de aceptacion: ";

    for (State state : nfa.acceptStates)
    {
        std::cout << state << " ";
    }

    std::cout << "\n\nTransiciones:\n";

    for (const auto& [from, symbolMap] : nfa.transitions)
    {
        for (const auto& [symbol, destinations] : symbolMap)
        {
            for (State to : destinations)
            {
                std::cout
                    << from
                    << " --"
                    << symbol
                    << "--> "
                    << to
                    << "\n";
            }
        }
    }

    for (const auto& [from, destinations] : nfa.epsilonTransitions)
    {
        for (State to : destinations)
        {
            std::cout
                << from
                << " --epsilon--> "
                << to
                << "\n";
        }
    }

    std::cout << "\n";
}