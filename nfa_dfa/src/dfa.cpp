#include "dfa.h"

#include <iostream>
#include <queue>
#include <vector>


static bool containsAcceptState(
    const StateSet& states,
    const std::set<State>& acceptStates
)
{
    for (State state : states)
    {
        if (acceptStates.count(state))
        {
            return true;
        }
    }

    return false;
}


DFA subsetConstruction(const NFA& nfa)
{
    DFA dfa;

    dfa.alphabet = nfa.alphabet;

    // s0 = epsilon-closure({q0})
    StateSet initialSet = { nfa.startState };

    dfa.startState =
        epsilonClosure(nfa, initialSet);

    dfa.states.insert(dfa.startState);

    std::queue<StateSet> pending;
    pending.push(dfa.startState);


    while (!pending.empty())
    {
        StateSet current = pending.front();
        pending.pop();

        for (char symbol : nfa.alphabet)
        {
            // Move(U, a)
            StateSet moved =
                move(nfa, current, symbol);

            // epsilon-Closure(Move(U, a))
            StateSet next =
                epsilonClosure(nfa, moved);

            // El pseudocodigo ignora conjuntos vacios
            if (next.empty())
            {
                continue;
            }

            // Si es un estado nuevo del DFA
            if (dfa.states.insert(next).second)
            {
                pending.push(next);
            }

            // deltaD(U, a) = V
            dfa.transitions[current][symbol] = next;
        }
    }


    // Determinar estados de aceptacion
    for (const StateSet& state : dfa.states)
    {
        if (containsAcceptState(
                state,
                nfa.acceptStates
            ))
        {
            dfa.acceptStates.insert(state);
        }
    }

    return dfa;
}

DFA removeUnreachableStates(const DFA& dfa)
{
    DFA result;

    result.startState = dfa.startState;
    result.alphabet = dfa.alphabet;

    std::set<StateSet> reachable;
    std::queue<StateSet> pending;

    reachable.insert(dfa.startState);
    pending.push(dfa.startState);

    while (!pending.empty())
    {
        StateSet current = pending.front();
        pending.pop();

        auto fromIt = dfa.transitions.find(current);

        if (fromIt == dfa.transitions.end())
        {
            continue;
        }

        for (const auto& [symbol, next] : fromIt->second)
        {
            (void)symbol;

            if (reachable.insert(next).second)
            {
                pending.push(next);
            }
        }
    }

    result.states = reachable;

    for (const StateSet& state : dfa.acceptStates)
    {
        if (reachable.count(state))
        {
            result.acceptStates.insert(state);
        }
    }

    for (const StateSet& state : reachable)
    {
        auto fromIt = dfa.transitions.find(state);

        if (fromIt == dfa.transitions.end())
        {
            continue;
        }

        for (const auto& [symbol, next] : fromIt->second)
        {
            if (reachable.count(next))
            {
                result.transitions[state][symbol] = next;
            }
        }
    }

    return result;
}

DFA completeDFA(const DFA& dfa)
{
    DFA result = dfa;

    StateSet sinkState;
    bool needsSink = false;

    for (const StateSet& state : dfa.states)
    {
        for (char symbol : dfa.alphabet)
        {
            auto stateIt = dfa.transitions.find(state);

            if (stateIt == dfa.transitions.end() ||
                !stateIt->second.count(symbol))
            {
                result.transitions[state][symbol] = sinkState;
                needsSink = true;
            }
        }
    }

    if (needsSink)
    {
        result.states.insert(sinkState);

        for (char symbol : dfa.alphabet)
        {
            result.transitions[sinkState][symbol] = sinkState;
        }
    }

    return result;
}



void printStateSet(const StateSet& states)
{
    std::cout << "(";

    bool first = true;

    for (State state : states)
    {
        if (!first)
        {
            std::cout << ",";
        }

        std::cout << state;

        first = false;
    }

    std::cout << ")";
}

static std::set<StateSet> setIntersection(
    const std::set<StateSet>& a,
    const std::set<StateSet>& b
)
{
    std::set<StateSet> result;

    for (const StateSet& state : a)
    {
        if (b.count(state))
        {
            result.insert(state);
        }
    }

    return result;
}


static std::set<StateSet> setDifference(
    const std::set<StateSet>& a,
    const std::set<StateSet>& b
)
{
    std::set<StateSet> result;

    for (const StateSet& state : a)
    {
        if (!b.count(state))
        {
            result.insert(state);
        }
    }

    return result;
}


static int findBlock(
    const std::vector<std::set<StateSet>>& partitions,
    const StateSet& state
)
{
    for (std::size_t i = 0;
         i < partitions.size();
         ++i)
    {
        if (partitions[i].count(state))
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}


MinDFA minimizeDFA(const DFA& input)
{
    // Primero eliminamos estados inalcanzables
    // y completamos las transiciones.
    DFA dfa = completeDFA(
        removeUnreachableStates(input)
    );

    using Block = std::set<StateSet>;

    std::vector<Block> partitions;
    std::vector<Block> worklist;


    // -------------------------
    // Particion inicial
    // P = {F, Q - F}
    // -------------------------

    Block accepting = dfa.acceptStates;
    Block nonAccepting;

    for (const StateSet& state : dfa.states)
    {
        if (!accepting.count(state))
        {
            nonAccepting.insert(state);
        }
    }

    if (!accepting.empty())
    {
        partitions.push_back(accepting);
        worklist.push_back(accepting);
    }

    if (!nonAccepting.empty())
    {
        partitions.push_back(nonAccepting);
        worklist.push_back(nonAccepting);
    }


    // -------------------------
    // Refinamiento de Hopcroft
    // -------------------------

    while (!worklist.empty())
    {
        Block A = worklist.back();
        worklist.pop_back();

        for (char symbol : dfa.alphabet)
        {
            Block X;

            /*
                X = {
                    q en Q |
                    delta(q, symbol) pertenece a A
                }
            */

            for (const StateSet& state : dfa.states)
            {
                const StateSet& destination =
                    dfa.transitions
                        .at(state)
                        .at(symbol);

                if (A.count(destination))
                {
                    X.insert(state);
                }
            }


            std::vector<Block> newPartitions;

            for (const Block& Y : partitions)
            {
                Block Y1 =
                    setIntersection(Y, X);

                Block Y2 =
                    setDifference(Y, X);


                // Si no hay division,
                // conservamos Y.
                if (Y1.empty() || Y2.empty())
                {
                    newPartitions.push_back(Y);
                    continue;
                }


                // Y se divide en Y1 y Y2.
                newPartitions.push_back(Y1);
                newPartitions.push_back(Y2);


                bool wasInWorklist = false;

                for (auto it = worklist.begin();
                     it != worklist.end();
                     ++it)
                {
                    if (*it == Y)
                    {
                        worklist.erase(it);

                        wasInWorklist = true;

                        break;
                    }
                }


                /*
                    Si Y ya estaba en W,
                    se sustituye por Y1 y Y2.

                    Si no estaba, agregamos
                    el bloque mas pequeno.
                */

                if (wasInWorklist)
                {
                    worklist.push_back(Y1);
                    worklist.push_back(Y2);
                }
                else
                {
                    if (Y1.size() <= Y2.size())
                    {
                        worklist.push_back(Y1);
                    }
                    else
                    {
                        worklist.push_back(Y2);
                    }
                }
            }

            partitions = newPartitions;
        }
    }


    // -------------------------
    // Construir DFA minimizado
    // -------------------------

    MinDFA result;

    result.alphabet = dfa.alphabet;
    result.startState = -1;


    for (std::size_t i = 0;
         i < partitions.size();
         ++i)
    {
        int newState =
            static_cast<int>(i);

        const Block& block =
            partitions[i];


        result.states.insert(newState);

        result.partitions[newState] =
            block;


        // ¿Este bloque contiene
        // al estado inicial?
        if (block.count(dfa.startState))
        {
            result.startState =
                newState;
        }


        // ¿Este bloque contiene
        // algun estado de aceptacion?
        for (const StateSet& state : block)
        {
            if (dfa.acceptStates.count(state))
            {
                result.acceptStates.insert(
                    newState
                );

                break;
            }
        }
    }


    // -------------------------
    // Nuevas transiciones
    // -------------------------

    for (std::size_t i = 0;
         i < partitions.size();
         ++i)
    {
        const Block& block =
            partitions[i];

        if (block.empty())
        {
            continue;
        }


        /*
            Todos los estados del bloque
            son equivalentes, por lo que
            basta usar uno como representante.
        */

        const StateSet& representative =
            *block.begin();

        int from =
            static_cast<int>(i);


        for (char symbol : dfa.alphabet)
        {
            const StateSet& destination =
                dfa.transitions
                    .at(representative)
                    .at(symbol);

            int to =
                findBlock(
                    partitions,
                    destination
                );

            if (to != -1)
            {
                result.transitions[from][symbol] =
                    to;
            }
        }
    }


    return result;
}

bool testString(
    const MinDFA& dfa,
    const std::string& input
)
{
    int currentState =
        dfa.startState;

    for (char symbol : input)
    {
        // El simbolo debe pertenecer
        // al alfabeto del DFA.
        if (!dfa.alphabet.count(symbol))
        {
            return false;
        }

        auto stateIt =
            dfa.transitions.find(currentState);

        if (stateIt ==
            dfa.transitions.end())
        {
            return false;
        }

        auto transitionIt =
            stateIt->second.find(symbol);

        if (transitionIt ==
            stateIt->second.end())
        {
            return false;
        }

        currentState =
            transitionIt->second;
    }

    return
        dfa.acceptStates.count(currentState)
        > 0;
}

void runTestSuite(
    const MinDFA& dfa,
    const std::vector<std::string>& acceptTests,
    const std::vector<std::string>& rejectTests
)
{
    int passed = 0;

    int total =
        static_cast<int>(
            acceptTests.size()
            + rejectTests.size()
        );


    std::cout
        << "\n[Casos de Aceptacion]\n";


    for (const std::string& input :
         acceptTests)
    {
        bool result =
            testString(dfa, input);

        std::cout
            << "Cadena \""
            << input
            << "\": "
            << (result ? "PASS" : "FAIL")
            << "\n";

        if (result)
        {
            passed++;
        }
    }


    std::cout
        << "\n[Casos de Rechazo]\n";


    for (const std::string& input :
         rejectTests)
    {
        bool result =
            !testString(dfa, input);

        std::cout
            << "Cadena \""
            << input
            << "\": "
            << (result ? "PASS" : "FAIL")
            << "\n";

        if (result)
        {
            passed++;
        }
    }


    std::cout
        << "\nResultado: "
        << passed
        << "/"
        << total
        << " pruebas superadas.\n";
}

void printDFA(const DFA& dfa)
{
    std::cout << "===== DFA =====\n";

    std::cout << "Estado inicial: ";
    printStateSet(dfa.startState);
    std::cout << "\n";

    std::cout << "Estados de aceptacion:\n";

    for (const StateSet& state : dfa.acceptStates)
    {
        std::cout << "  ";
        printStateSet(state);
        std::cout << "\n";
    }


    std::cout << "\nEstados del DFA:\n";

    for (const StateSet& state : dfa.states)
    {
        std::cout << "  ";
        printStateSet(state);

        if (dfa.acceptStates.count(state))
        {
            std::cout << " [aceptacion]";
        }

        std::cout << "\n";
    }


    std::cout << "\nTransiciones:\n";

    for (const auto& [from, symbolMap] : dfa.transitions)
    {
        for (const auto& [symbol, to] : symbolMap)
        {
            std::cout << "  ";

            printStateSet(from);

            std::cout
                << " --"
                << symbol
                << "--> ";

            printStateSet(to);

            std::cout << "\n";
        }
    }

    std::cout << "\n";
}

void printMinDFA(const MinDFA& dfa)
{
    std::cout
        << "===== DFA MINIMIZADO =====\n";

    std::cout
        << "Estado inicial: "
        << dfa.startState
        << "\n";


    std::cout
        << "Estados de aceptacion:";

    for (int state : dfa.acceptStates)
    {
        std::cout
            << " "
            << state;
    }

    std::cout << "\n";


    std::cout
        << "\nEstados:\n";


    for (int state : dfa.states)
    {
        std::cout
            << "  "
            << state
            << " = {";


        bool first = true;

        for (const StateSet& original :
             dfa.partitions.at(state))
        {
            if (!first)
            {
                std::cout << ", ";
            }

            printStateSet(original);

            first = false;
        }

        std::cout << "}";


        if (dfa.acceptStates.count(state))
        {
            std::cout
                << " [aceptacion]";
        }

        std::cout << "\n";
    }


    std::cout
        << "\nTransiciones:\n";


    for (const auto& [from, symbolMap] :
         dfa.transitions)
    {
        for (const auto& [symbol, to] :
             symbolMap)
        {
            std::cout
                << "  "
                << from
                << " --"
                << symbol
                << "--> "
                << to
                << "\n";
        }
    }

    std::cout << "\n";
}