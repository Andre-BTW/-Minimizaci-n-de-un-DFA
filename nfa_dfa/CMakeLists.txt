cmake_minimum_required(VERSION 3.16)

project(nfa_to_dfa)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(
    automata
    src/nfa.cpp
    src/dfa.cpp
    src/regex.cpp
)

target_include_directories(
    automata
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


add_executable(
    nfa_to_dfa
    src/main.cpp
)

target_link_libraries(
    nfa_to_dfa
    automata
)


enable_testing()

add_executable(
    tests
    tests/test.cpp
)

target_link_libraries(
    tests
    automata
)

add_test(
    NAME automata_tests
    COMMAND tests
)