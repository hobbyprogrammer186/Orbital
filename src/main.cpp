/*
 * Orbital Shell  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <Lexer.h>
#include <Parser.h>
#include <orbtlio.h>
#if defined(_WIN32) || defined(_WIN64)
    #include <curses.h>
    #include <conio.h>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
    #include <ncurses.h>
    #include <unistd.h>
    #include <termios.h>
#endif

extern std::map<std::string, VariableInfo> variableTable;
extern std::map<std::string, FunctionNode*> functionTable;

static const std::vector<std::string> KEYWORDS = {
    "import", "use", "class", "is", "in", "and", "or", "not",
    "True", "False", "None", "const", "if", "else", "elif",
    "Greater", "Shorter", "switch", "case", "default",
    "for", "while", "break", "continue", "finish",
    "return", "throw", "struct", "nocache", "symbol",
    "print"
};

static std::vector<std::string> get_all_suggestions() {
    std::vector<std::string> all = KEYWORDS;
    for (const auto& pair : variableTable) {
        all.push_back(pair.first);
    }
    for (const auto& pair : functionTable) {
        all.push_back(pair.first);
    }
    std::sort(all.begin(), all.end());
    all.erase(std::unique(all.begin(), all.end()), all.end());
    return all;
}

static std::string find_suggestion(const std::string& buffer) {
    std::string current_word;
    for (int i = (int)buffer.size() - 1; i >= 0; i--) {
        char c = buffer[i];
        if (c == ' ' || c == '\t') break;
        current_word = c + current_word;
    }
    if (current_word.empty()) return "";

    std::vector<std::string> all = get_all_suggestions();

    for (const auto& kw : all) {
        if (kw.size() > current_word.size() &&
            kw.substr(0, current_word.size()) == current_word) {
            return kw.substr(current_word.size());
        }
    }
    return "";
}

static void clear_line() {
    std::cout << "\r\033[K" << std::flush;
}

#if defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
char get_char() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
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
        std::string suggestion = find_suggestion(buffer);
        clear_line();
        std::cout << ">> " << buffer;
        if (!suggestion.empty()) {
            std::cout << "\033[90m> " << suggestion << "\033[0m";
        }
        std::cout << "\r" << std::string(3 + (int)buffer.size(), ' ') << "\r>> " << buffer << std::flush;

#if defined(_WIN32) || defined(_WIN64)
        char c = _getch();
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
        char c = get_char();
#endif

        if (c == '\n' || c == '\r' || c == 10) {
            clear_line();
            std::cout << ">> " << buffer << std::endl;
            return buffer;
        }
        else if (c == 127 || c == 8) {
            if (!buffer.empty()) {
                buffer.pop_back();
            }
        }
        else if (c == '\t') {
            if (!suggestion.empty()) {
                buffer += suggestion;
            }
        }
        else if (c >= 32 && c <= 126) {
            buffer += c;
        }
    }
}

int main() {
    std::cout << "Orbital Programming Language v1.0.0-alpha" << std::endl << std::endl;
    init();

    while(1) {
        Parser parser(input());
        parser.Interpret();
    }
}