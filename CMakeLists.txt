cmake_minimum_required(VERSION 2.6)
project(rgr)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Lexer.cpp Lexer.h Parser.cpp Parser.h)
add_executable(rgr ${SOURCE_FILES} main.cpp)
add_executable(rgr_test ${SOURCE_FILES} tests.cpp LexerTest.cpp ParserTest.cpp)
