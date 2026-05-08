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
#include <map>
#include <Lexer.h>
#include <Parser.h>
#include <orbtlio.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <termios.h>
#include <cctype>
#include <csignal>

namespace fs = std::filesystem;
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
    for (const auto& pair : variableTable) all.push_back(pair.first);
    for (const auto& pair : functionTable) all.push_back(pair.first);
    std::sort(all.begin(), all.end());
    all.erase(std::unique(all.begin(), all.end()), all.end());
    return all;
}

static std::string find_suggestion(const std::string& buffer) {
    std::string current_word;
    for (int i = (int)buffer.size() - 1; i >= 0; --i) {
        char c = buffer[i];
        if (std::isspace(static_cast<unsigned char>(c))) break;
        current_word.insert(current_word.begin(), c);
    }
    if (current_word.empty()) return std::string();

    std::vector<std::string> all = get_all_suggestions();
    for (const auto& kw : all) {
        if (kw.size() > current_word.size() &&
            kw.compare(0, current_word.size(), current_word) == 0) {
            return kw.substr(current_word.size());
        }
    }
    return std::string();
}

// Inline suggestion input using termios and ANSI escapes. Tab accepts suggestion.
std::string input() {
    std::string buffer;
    std::cout << ">> " << std::flush;

    termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        // Fallback to basic getline if termios unsupported
        if (!std::getline(std::cin, buffer)) return std::string();
        return buffer;
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    auto restore = [&]() { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); };

    while (true) {
        char ch = 0;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n <= 0) { restore(); return buffer; }

        if (ch == '\n' || ch == '\r') {
            std::cout << std::endl;
            restore();
            return buffer;
        }
        if (ch == 3) { // Ctrl-C
            restore();
            std::raise(SIGINT);
            return std::string();
        }
        if (ch == 4) { // Ctrl-D
            restore();
            return std::string();
        }
        if (ch == 127 || ch == 8) { // Backspace
            if (!buffer.empty()) buffer.pop_back();
        } else if (ch == '\t') { // Tab: accept suggestion
            std::string sug = find_suggestion(buffer);
            if (!sug.empty()) buffer += sug;
        } else if (ch >= 32 && ch <= 126) {
            buffer.push_back(ch);
        }

        std::string sug = find_suggestion(buffer);
        std::cout << "\r\x1b[K" << ">> " << buffer;
        if (!sug.empty()) {
            std::cout << "\x1b[90m" << "-> " << sug << "\x1b[0m";
        }
        std::cout << std::flush;
    }
}

int main(int argc, char **argv) {
    init();

    if(argc >= 2) {
        if(!fs::exists(argv[1])) {
            std::cout << "File Is Not Exist" << std::endl;
            exit(1);
        }
        
        std::ifstream file(argv[1]);
        if(!file.is_open()) {
            std::cout << "Access Is Deniend." << std::endl;
            exit(1);
        }

        std::string line;
        std::string buffer;
        while (std::getline(file, line))
            buffer += line + "\n";

        Parser parser(buffer, argv[1]);
        parser.Interpret();
        file.close();
    }
    else {
        auto now = std::chrono::system_clock::now();
        auto nyear = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(now)).year();
        int yearInt = static_cast<int>(nyear);
        std::string copyrightYear = yearInt > 2026 ? std::to_string(yearInt) : "2026";

        std::cout << std::string(NAME_STRING) + " Programming Language " + VERSION_STRING << std::endl;
        std::cout << "Copyright (c) " << copyrightYear << " First Person" << std::endl << std::endl;

        while (true) {
            std::string line = input();
            if (std::cin.eof()) break;
            Parser parser(line);
            parser.Interpret();
        }
    }
}