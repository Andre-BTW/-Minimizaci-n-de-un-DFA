#include "nfa.h"
#include "dfa.h"
#include "regex.h"

#include <cassert>
#include <iostream>


void testEpsilonClosure()
{
    NFA nfa;

    /*
        0 --e--> 1
        1 --e--> 2
        2 --e--> 1

        Ciclo entre 1 y 2.
    */

    nfa.addEpsilonTransition(0, 1);
    nfa.addEpsilonTransition(1, 2);
    nfa.addEpsilonTransition(2, 1);

    StateSet input = {0};

    StateSet result =
        epsilonClosure(nfa, input);

    StateSet expected = {0, 1, 2};

    assert(result == expected);

    std::cout
        << "epsilonClosure (CORRECTO)\n";
}


void testMove()
{
    NFA nfa;

    /*
        0 --a--> 1
        0 --a--> 2
        0 --b--> 3
    */

    nfa.addTransition(0, 'a', 1);
    nfa.addTransition(0, 'a', 2);
    nfa.addTransition(0, 'b', 3);

    StateSet input = {0};

    StateSet result =
        move(nfa, input, 'a');

    StateSet expected = {1, 2};

    assert(result == expected);

    std::cout
        << "move (CORRECTO)\n";
}


void testSubsetConstruction()
{
    NFA nfa;

    nfa.startState = 0;

    nfa.addState(0);
    nfa.addState(1);
    nfa.addAcceptState(2);

    nfa.addEpsilonTransition(0, 1);

    nfa.addTransition(1, 'a', 1);
    nfa.addTransition(1, 'b', 2);

    nfa.addTransition(2, 'b', 2);


    DFA dfa =
        subsetConstruction(nfa);


    StateSet state01 = {0, 1};
    StateSet state1 = {1};
    StateSet state2 = {2};


    // Estado inicial
    assert(dfa.startState == state01);


    // Estados encontrados
    assert(dfa.states.count(state01));
    assert(dfa.states.count(state1));
    assert(dfa.states.count(state2));


    // Debe haber exactamente 3
    assert(dfa.states.size() == 3);


    // Transiciones desde 0,1
    assert(
        dfa.transitions[state01]['a']
        == state1
    );

    assert(
        dfa.transitions[state01]['b']
        == state2
    );


    // Transiciones desde 1
    assert(
        dfa.transitions[state1]['a']
        == state1
    );

    assert(
        dfa.transitions[state1]['b']
        == state2
    );


    // 2 debe ser aceptacion
    assert(
        dfa.acceptStates.count(state2)
    );


    std::cout
        << "subsetConstruction (CORRECTO)\n";
}

void testRemoveUnreachableStates()
{
    DFA dfa;

    StateSet state0 = {0};
    StateSet state1 = {1};
    StateSet state2 = {2};

    dfa.startState = state0;
    dfa.states = {state0, state1, state2};
    dfa.acceptStates = {state1, state2};
    dfa.alphabet = {'a', 'b'};

    dfa.transitions[state0]['a'] = state1;
    dfa.transitions[state0]['b'] = state0;

    dfa.transitions[state1]['a'] = state1;
    dfa.transitions[state1]['b'] = state0;

    // state2 existe, pero no se puede alcanzar
    // desde el estado inicial.
    dfa.transitions[state2]['a'] = state2;
    dfa.transitions[state2]['b'] = state2;

    DFA result =
        removeUnreachableStates(dfa);

    assert(result.states.size() == 2);

    assert(result.states.count(state0));
    assert(result.states.count(state1));

    assert(!result.states.count(state2));

    assert(result.acceptStates.count(state1));

    assert(!result.acceptStates.count(state2));

    std::cout
        << "removeUnreachableStates (CORRECTO)\n";
}


void testCompleteDFA()
{
    DFA dfa;

    StateSet state0 = {0};
    StateSet state1 = {1};

    // El conjunto vacio sera el estado sumidero.
    StateSet sinkState;

    dfa.startState = state0;

    dfa.states = {
        state0,
        state1
    };

    dfa.acceptStates = {
        state1
    };

    dfa.alphabet = {
        'a',
        'b'
    };

    // Intencionalmente faltan las
    // transiciones con b.
    dfa.transitions[state0]['a'] = state1;
    dfa.transitions[state1]['a'] = state1;

    DFA result =
        completeDFA(dfa);

    assert(
        result.states.count(sinkState)
    );

    assert(
        result.transitions[state0]['a']
        == state1
    );

    assert(
        result.transitions[state0]['b']
        == sinkState
    );

    assert(
        result.transitions[state1]['a']
        == state1
    );

    assert(
        result.transitions[state1]['b']
        == sinkState
    );

    assert(
        result.transitions[sinkState]['a']
        == sinkState
    );

    assert(
        result.transitions[sinkState]['b']
        == sinkState
    );

    std::cout
        << "completeDFA (CORRECTO)\n";
}

void testMinimizeDFA()
{
    DFA dfa;

    StateSet A = {0};
    StateSet B = {1};
    StateSet C = {2};
    StateSet D = {3};
    StateSet E = {4};


    dfa.startState = A;

    dfa.states = {
        A, B, C, D, E
    };

    dfa.acceptStates = {
        E
    };

    dfa.alphabet = {
        '0', '1'
    };


    /*
        DFA del ejemplo de la practica.

        A y C deben resultar equivalentes.
    */

    dfa.transitions[A]['0'] = B;
    dfa.transitions[A]['1'] = C;

    dfa.transitions[B]['0'] = B;
    dfa.transitions[B]['1'] = D;

    dfa.transitions[C]['0'] = B;
    dfa.transitions[C]['1'] = C;

    dfa.transitions[D]['0'] = B;
    dfa.transitions[D]['1'] = E;

    dfa.transitions[E]['0'] = B;
    dfa.transitions[E]['1'] = C;


    MinDFA result =
        minimizeDFA(dfa);


    // El original tiene 5 estados.
    // El minimo debe tener 4.
    assert(
        result.states.size() == 4
    );


    int blockA = -1;
    int blockC = -1;


    // Buscamos en que bloque
    // quedaron A y C.
    for (const auto& [id, block] :
         result.partitions)
    {
        if (block.count(A))
        {
            blockA = id;
        }

        if (block.count(C))
        {
            blockC = id;
        }
    }


    assert(blockA != -1);
    assert(blockC != -1);

    // A y C deben haberse fusionado.
    assert(blockA == blockC);


    std::cout
        << "minimizeDFA (CORRECTO)\n";
}

void testRegexToNFA()
{
    NFA nfa =
        regexToNFA("(a|b)*abb");


    DFA dfa =
        subsetConstruction(nfa);


    MinDFA dfaMin =
        minimizeDFA(dfa);


    // Deben aceptarse porque terminan en abb.
    assert(
        testString(dfaMin, "abb")
    );

    assert(
        testString(dfaMin, "aabb")
    );

    assert(
        testString(dfaMin, "babb")
    );


    // No terminan en abb.
    assert(
        !testString(dfaMin, "ab")
    );

    assert(
        !testString(dfaMin, "abba")
    );


    std::cout
        << "regexToNFA (CORRECTO)\n";
}

int main()
{
    testEpsilonClosure();

    testMove();

    testSubsetConstruction();

    testRemoveUnreachableStates();

    testCompleteDFA();

    testMinimizeDFA();

    testRegexToNFA();

    std::cout
        << "\nTests exitosos.\n";

    return 0;
}