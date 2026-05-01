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
#include <ncurses.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
    #include <termios.h>
#endif

#if defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
char get_char() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON; // Disable line buffering
    old.c_lflag &= ~ECHO;   // Disable character echoing
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return buf;
}
#endif

std::string input() {
    std::string buffer;
    while(1) {
#if defined(_WIN32) || defined(_WIN64)
        char c = _getch();
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
        char c = get_char();
#endif

        if (c == '\n' || c == '\r' || c == 10) { // Enter Key
            std::cout << std::endl;
            return buffer;
        } 
        else if (c == 127 || c == 8) { // Backspace
            if (!buffer.empty()) {
                buffer.pop_back();
                // Move cursor back, print space to "erase", move cursor back again
                std::cout << "\b \b" << std::flush;
            }
        } 
        else {
            buffer += c;
            std::cout << c << std::flush; // Manual echo
        }
    }
}

int main() {
    std::cout << "Orbital Programming Language v1.0.0-alpha" << std::endl << std::endl;
    init();

    while(1) {
        std::cout << ">> ";
        Parser parser(input());
        parser.Interpret();
    }
}