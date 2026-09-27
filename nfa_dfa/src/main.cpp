#include "nfa.h"
#include "dfa.h"
#include "regex.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>


int main()
{
    try
    {
        // ==================================================
        // ENTRADA DE LA EXPRESION REGULAR
        // ==================================================

        std::string regex;

        std::cout
            << "Ingrese una expresion regular: ";

        std::cin >> regex;


        // ==================================================
        // REGEX -> NFA
        // ==================================================

        NFA nfa =
            regexToNFA(regex);


        std::cout
            << "\n--------------------\n"
            << "       NFA ORIGINAL\n"
            << "--------------------\n\n";

        printNFA(nfa);


        // ==================================================
        // NFA -> DFA
        // ==================================================

        DFA dfa =
            subsetConstruction(nfa);


        // Eliminamos estados inalcanzables
        // y completamos transiciones.
        DFA dfaPrepared =
            completeDFA(
                removeUnreachableStates(dfa)
            );


        // ==================================================
        // DFA ORIGINAL
        // ==================================================

        std::cout
            << "--------------------\n"
            << "       DFA ORIGINAL\n"
            << "--------------------\n\n";

        printDFA(dfaPrepared);


        // ==================================================
        // MINIMIZACION CON HOPCROFT
        // ==================================================

        MinDFA dfaMin =
            minimizeDFA(dfaPrepared);


        // ==================================================
        // DFA MINIMIZADO
        // ==================================================

        std::cout
            << "--------------------\n"
            << "     DFA MINIMIZADO\n"
            << "--------------------\n\n";

        printMinDFA(dfaMin);


        // ==================================================
        // COMPROBACION
        // ==================================================

        std::cout
            << "--------------------\n"
            << "      COMPROBACION\n"
            << "--------------------\n\n";


        std::cout
            << "Expresion regular: "
            << regex
            << "\n\n";


        std::cout
            << "Estados del DFA original: "
            << dfaPrepared.states.size()
            << "\n";


        std::cout
            << "Estados del DFA minimizado: "
            << dfaMin.states.size()
            << "\n\n";


        if (dfaPrepared.states.size()
            >= dfaMin.states.size())
        {
            std::cout
                << dfaPrepared.states.size()
                << " >= "
                << dfaMin.states.size()
                << " (CORRECTO)\n";
        }
        else
        {
            std::cout
                << "ERROR: el DFA minimizado tiene "
                << "mas estados que el original.\n";

            return 1;
        }


        // ==================================================
        // CASOS DE PRUEBA
        // ==================================================

        std::vector<std::string> acceptTests;
        std::vector<std::string> rejectTests;


        // --------------------------------------------------
        // REGEX 1
        // (a|b)*abb
        // Cadenas que terminan en abb
        // --------------------------------------------------

        if (regex == "(a|b)*abb")
        {
            acceptTests = {
                "abb",
                "aabb",
                "babb",
                "aaabb",
                "bbabb",
                "ababb",
                "aababb",
                "bbaabb",
                "aaaabb",
                "bababb"
            };


            rejectTests = {
                "",
                "a",
                "b",
                "ab",
                "aba",
                "abba",
                "baa",
                "bbbb",
                "aaba",
                "bababa"
            };
        }


        // --------------------------------------------------
        // REGEX 2
        // (0|1)*01(0|1)*
        // Cadenas que contienen 01
        // --------------------------------------------------

        else if (regex == "(0|1)*01(0|1)*")
        {
            acceptTests = {
                "01",
                "001",
                "101",
                "010",
                "011",
                "0001",
                "1101",
                "0101",
                "10010",
                "111011"
            };


            rejectTests = {
                "",
                "0",
                "1",
                "00",
                "10",
                "11",
                "1000",
                "1111",
                "1100",
                "111000"
            };
        }


        // --------------------------------------------------
        // REGEX 6
        // (a|b)*ab(a|b)*
        // Cadenas que contienen ab
        // --------------------------------------------------

        else if (regex == "(a|b)*ab(a|b)*")
        {
            acceptTests = {
                "ab",
                "aab",
                "abb",
                "abab",
                "bab",
                "baab",
                "aaab",
                "abbb",
                "bbaba",
                "aabbab"
            };


            rejectTests = {
                "",
                "a",
                "b",
                "aa",
                "bb",
                "ba",
                "bba",
                "bbaa",
                "bbbbaaa",
                "aaa"
            };
        }


        // --------------------------------------------------
        // Regex valida, pero no pertenece a las 3 elegidas
        // para la suite de pruebas.
        // --------------------------------------------------

        else
        {
            std::cout
                << "\n--------------------\n"
                << "        PRUEBAS\n"
                << "--------------------\n\n";

            std::cout
                << "La expresion regular fue procesada "
                << "correctamente, pero no tiene una suite "
                << "de 20 pruebas configurada.\n";

            return 0;
        }


        // ==================================================
        // EJECUTAR LAS 20 PRUEBAS
        // ==================================================

        std::cout
            << "\n--------------------\n"
            << "        PRUEBAS\n"
            << "--------------------\n";


        runTestSuite(
            dfaMin,
            acceptTests,
            rejectTests
        );


        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "\nERROR: "
            << error.what()
            << "\n";

        return 1;
    }
}