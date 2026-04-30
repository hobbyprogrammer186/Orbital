/*
 * Orbital Shell  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <string>
#include <cstdbool>
#include <vector>
#include <stdexcept>
#include <Lexer.h>
#include <Parser.h>
#include <orbtlio.h>

int main() {
    std::string code = R"(
print:
    SayHello

SayHelloWorld:
    print "Hello World!"

SayHello(nickname):
    print("Hello " + nickname)

SayHello "Earth"
SayHelloWorld
)";
    init();

    try {
        Parser parser(code);
        parser.Interpret();
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}