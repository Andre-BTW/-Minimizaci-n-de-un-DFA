#include "regex.h"

#include <stdexcept>
#include <vector>


namespace
{

int precedence(RegexType type)
{
    if (type == RegexType::Concat)
    {
        return 2;
    }

    if (type == RegexType::Or)
    {
        return 1;
    }

    return 0;
}


bool canEndExpression(char c)
{
    return c != '|'
        && c != '(';
}


bool canStartExpression(char c)
{
    return c != '|'
        && c != ')'
        && c != '*'
        && c != '+'
        && c != '?';
}


/*
    Inserta concatenaciones explicitas.

    Ejemplo:

        ab

    se convierte en:

        a.b
*/
std::string addConcatenation(
    const std::string& input
)
{
    std::string result;

    result.reserve(
        input.size() * 2
    );


    for (std::size_t i = 0;
         i < input.size();
         ++i)
    {
        char current =
            input[i];

        result.push_back(current);


        if (i + 1 < input.size())
        {
            char next =
                input[i + 1];


            if (canEndExpression(current)
                && canStartExpression(next))
            {
                result.push_back('.');
            }
        }
    }


    return result;
}


struct Fragment
{
    State start;
    State accept;
};

}


/*
    Convierte la expresion regular
    de notacion infija a postfija
    usando Shunting-Yard.
*/
std::vector<RegexItem> parseRegex(
    const std::string& regex
)
{
    if (regex.empty())
    {
        throw std::runtime_error(
            "La expresion regular esta vacia."
        );
    }


    std::string explicitRegex =
        addConcatenation(regex);


    std::vector<RegexItem> output;

    std::vector<char> operators;


    for (char c : explicitRegex)
    {
        // Caracter normal
        if (c != '('
            && c != ')'
            && c != '|'
            && c != '*'
            && c != '+'
            && c != '?'
            && c != '.')
        {
            output.push_back({
                c,
                RegexType::Char
            });

            continue;
        }


        // Estrella de Kleene
        if (c == '*')
        {
            output.push_back({
                '*',
                RegexType::Star
            });

            continue;
        }


        // Una o mas repeticiones
        if (c == '+')
        {
            output.push_back({
                '+',
                RegexType::Plus
            });

            continue;
        }


        // Cero o una repeticion
        if (c == '?')
        {
            output.push_back({
                '?',
                RegexType::Question
            });

            continue;
        }


        // Parentesis izquierdo
        if (c == '(')
        {
            operators.push_back(c);

            continue;
        }


        // Parentesis derecho
        if (c == ')')
        {
            bool foundOpen = false;


            while (!operators.empty())
            {
                char op =
                    operators.back();

                operators.pop_back();


                if (op == '(')
                {
                    foundOpen = true;

                    break;
                }


                output.push_back({
                    op,
                    op == '.'
                        ? RegexType::Concat
                        : RegexType::Or
                });
            }


            if (!foundOpen)
            {
                throw std::runtime_error(
                    "Parentesis desbalanceados."
                );
            }


            continue;
        }


        // Concatenacion u OR
        RegexType currentType =
            (c == '.')
                ? RegexType::Concat
                : RegexType::Or;


        while (!operators.empty()
               && operators.back() != '(')
        {
            char top =
                operators.back();


            RegexType topType =
                (top == '.')
                    ? RegexType::Concat
                    : RegexType::Or;


            if (precedence(topType)
                < precedence(currentType))
            {
                break;
            }


            operators.pop_back();


            output.push_back({
                top,
                topType
            });
        }


        operators.push_back(c);
    }


    // Vaciamos operadores restantes
    while (!operators.empty())
    {
        char op =
            operators.back();

        operators.pop_back();


        if (op == '(')
        {
            throw std::runtime_error(
                "Parentesis desbalanceados."
            );
        }


        output.push_back({
            op,
            op == '.'
                ? RegexType::Concat
                : RegexType::Or
        });
    }


    return output;
}


/*
    Construccion de Thompson.

    Convierte directamente la expresion
    regular a nuestra estructura NFA de C++.
*/
NFA regexToNFA(
    const std::string& regex
)
{
    std::vector<RegexItem> postfix =
        parseRegex(regex);


    NFA result;

    std::vector<Fragment> stack;


    State nextState = 0;


    auto newState =
        [&nextState]()
        {
            return nextState++;
        };


    for (const RegexItem& item :
         postfix)
    {

        // ==========================================
        // CARACTER
        // ==========================================

        if (item.type == RegexType::Char)
        {
            Fragment fragment = {
                newState(),
                newState()
            };


            result.addTransition(
                fragment.start,
                item.value,
                fragment.accept
            );


            stack.push_back(fragment);

            continue;
        }


        // ==========================================
        // CONCATENACION
        // ==========================================

        if (item.type == RegexType::Concat)
        {
            if (stack.size() < 2)
            {
                throw std::runtime_error(
                    "Concatenacion invalida."
                );
            }


            Fragment right =
                stack.back();

            stack.pop_back();


            Fragment left =
                stack.back();

            stack.pop_back();


            result.addEpsilonTransition(
                left.accept,
                right.start
            );


            stack.push_back({
                left.start,
                right.accept
            });


            continue;
        }


        // ==========================================
        // ALTERNACION |
        // ==========================================

        if (item.type == RegexType::Or)
        {
            if (stack.size() < 2)
            {
                throw std::runtime_error(
                    "Alternacion invalida."
                );
            }


            Fragment right =
                stack.back();

            stack.pop_back();


            Fragment left =
                stack.back();

            stack.pop_back();


            State start =
                newState();

            State accept =
                newState();


            result.addEpsilonTransition(
                start,
                left.start
            );

            result.addEpsilonTransition(
                start,
                right.start
            );

            result.addEpsilonTransition(
                left.accept,
                accept
            );

            result.addEpsilonTransition(
                right.accept,
                accept
            );


            stack.push_back({
                start,
                accept
            });


            continue;
        }


        // ==========================================
        // ESTRELLA DE KLEENE *
        // ==========================================

        if (item.type == RegexType::Star)
        {
            if (stack.empty())
            {
                throw std::runtime_error(
                    "Estrella de Kleene invalida."
                );
            }


            Fragment inner =
                stack.back();

            stack.pop_back();


            State start =
                newState();

            State accept =
                newState();


            result.addEpsilonTransition(
                start,
                accept
            );

            result.addEpsilonTransition(
                start,
                inner.start
            );

            result.addEpsilonTransition(
                inner.accept,
                inner.start
            );

            result.addEpsilonTransition(
                inner.accept,
                accept
            );


            stack.push_back({
                start,
                accept
            });


            continue;
        }


        // ==========================================
        // UNA O MAS REPETICIONES +
        // ==========================================

        if (item.type == RegexType::Plus)
        {
            if (stack.empty())
            {
                throw std::runtime_error(
                    "Operador + invalido."
                );
            }


            Fragment inner =
                stack.back();

            stack.pop_back();


            State start =
                newState();

            State accept =
                newState();


            result.addEpsilonTransition(
                start,
                inner.start
            );

            result.addEpsilonTransition(
                inner.accept,
                inner.start
            );

            result.addEpsilonTransition(
                inner.accept,
                accept
            );


            stack.push_back({
                start,
                accept
            });


            continue;
        }


        // ==========================================
        // CERO O UNA REPETICION ?
        // ==========================================

        if (item.type == RegexType::Question)
        {
            if (stack.empty())
            {
                throw std::runtime_error(
                    "Operador ? invalido."
                );
            }


            Fragment inner =
                stack.back();

            stack.pop_back();


            State start =
                newState();

            State accept =
                newState();


            result.addEpsilonTransition(
                start,
                accept
            );

            result.addEpsilonTransition(
                start,
                inner.start
            );

            result.addEpsilonTransition(
                inner.accept,
                accept
            );


            stack.push_back({
                start,
                accept
            });
        }
    }


    if (stack.size() != 1)
    {
        throw std::runtime_error(
            "Expresion regular invalida."
        );
    }


    result.startState =
        stack.back().start;


    result.addAcceptState(
        stack.back().accept
    );


    return result;
}