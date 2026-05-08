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
#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif
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

static std::vector<std::string> get_matching_suggestions(const std::string& prefix) {
    std::vector<std::string> matches;
    if (prefix.empty()) return matches;

    std::vector<std::string> all = get_all_suggestions();
    for (const auto &kw : all) {
        if (kw.size() >= prefix.size() && kw.compare(0, prefix.size(), prefix) == 0) {
            matches.push_back(kw);
        }
    }
    return matches;
}

// Inline suggestion input using termios and ANSI escapes. Tab accepts suggestion.
std::string input() {
    std::string buffer;

    auto extract_current_word = [](const std::string &buf) {
        std::string w;
        for (int i = (int)buf.size() - 1; i >= 0; --i) {
            char c = buf[i];
            if (std::isspace(static_cast<unsigned char>(c))) break;
            w.insert(w.begin(), c);
        }
        return w;
    };

    const int MAX_VISIBLE = 6;
    const int BOX_MAX_WIDTH = 60;

    std::vector<std::string> suggestions;
    int selected_idx = -1;
    bool showing = false;

    auto update_suggestions = [&]() {
        std::string prefix = extract_current_word(buffer);
        suggestions = get_matching_suggestions(prefix);
        if (!prefix.empty() && !suggestions.empty()) {
            showing = true;
            selected_idx = 0;
        } else {
            showing = false;
            selected_idx = -1;
        }
    };

    // initial prompt
    std::cout << ">> " << buffer << std::flush;

    int last_box_lines = 0;

    update_suggestions();

    auto render_box = [&]() {
        // reprint prompt and clear below
        std::cout << "\r\x1b[K>> " << buffer;
        std::cout << "\x1b[J"; // clear below

        int printed_lines = 0;
        if (showing && !suggestions.empty()) {
            int total = (int)suggestions.size();
            int start = 0;
            if (selected_idx >= MAX_VISIBLE / 2) start = selected_idx - MAX_VISIBLE / 2;
            if (start + MAX_VISIBLE > total) start = std::max(0, total - MAX_VISIBLE);
            int end = std::min(total, start + MAX_VISIBLE);

            size_t maxlen = 0;
            for (int i = start; i < end; ++i) if (suggestions[i].size() > maxlen) maxlen = suggestions[i].size();
            size_t content_width = std::min<size_t>(BOX_MAX_WIDTH, std::max<size_t>(10, maxlen + 2));
            std::string horiz(content_width, '-');

            // top border
            std::cout << "\n+" << horiz << "+\n";
            printed_lines += 2;

            for (int i = start; i < end; ++i) {
                std::string item = suggestions[i];
                size_t pad = (content_width - 2 > item.size()) ? (content_width - 2 - item.size()) : 0;
                std::string padded = item + std::string(pad, ' ');

                if (i == selected_idx) {
                    std::cout << "| " << "\x1b[7m" << padded << "\x1b[0m" << " |";
                } else {
                    std::cout << "| " << "\x1b[90m" << padded << "\x1b[0m" << " |";
                }
                std::cout << "\n";
                printed_lines++;
            }

            // bottom border
            std::cout << "+" << horiz << "+\n";
            printed_lines++;

            // footer
#if defined(_WIN32) || defined(_WIN64)
            // Some Windows consoles (especially older versions) do not render
            // UTF-8 arrow characters correctly. Use an ASCII fallback.
            std::cout << "[" << (start + 1) << "-" << end << "/" << total << "] Use Tab to accept, Up/Down to navigate\n";
#else
            std::cout << "[" << (start + 1) << "-" << end << "/" << total << "] Use Tab to accept, ↑/↓ to navigate\n";
#endif
            printed_lines++;

            // move cursor back up to prompt line
            std::cout << "\x1b[" << printed_lines << "A";

            // reprint prompt to position cursor at end
            std::cout << "\r\x1b[K>> " << buffer;
        }

        std::cout << std::flush;
        last_box_lines = printed_lines;
    };

    render_box();

#if defined(_WIN32) || defined(_WIN64)
    // Enable ANSI processing on Windows consoles if possible and try to
    // switch the output code page to UTF-8. Older Windows (e.g., 7) may not
    // fully support UTF-8 in the console; we still attempt this but keep an
    // ASCII fallback elsewhere.
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    // Try to set console output to UTF-8. This may fail on older systems,
    // but it's safe to attempt.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    while (true) {
        int ch = _getch();

        if (ch == 13) { // Enter
            std::cout << std::endl;
            return buffer;
        }
        if (ch == 3) { // Ctrl-C
            std::raise(SIGINT);
            return std::string();
        }
        if (ch == 4) { // Ctrl-D (not typical on Windows)
            return std::string();
        }

        if (ch == 8) { // Backspace
            if (!buffer.empty()) buffer.pop_back();
            update_suggestions();
            render_box();
            continue;
        }

        if (ch == 9) { // Tab
            if (showing && selected_idx >= 0 && selected_idx < (int)suggestions.size()) {
                std::string prefix = extract_current_word(buffer);
                buffer = buffer.substr(0, buffer.size() - prefix.size());
                buffer += suggestions[selected_idx];
                update_suggestions();
                render_box();
            }
            continue;
        }

        if (ch == 0 || ch == 224) {
            int code = _getch();
            if (code == 72) { // up
                if (showing && !suggestions.empty()) {
                    selected_idx = (selected_idx - 1 + (int)suggestions.size()) % (int)suggestions.size();
                    render_box();
                }
                continue;
            } else if (code == 80) { // down
                if (showing && !suggestions.empty()) {
                    selected_idx = (selected_idx + 1) % (int)suggestions.size();
                    render_box();
                }
                continue;
            }
            // ignore other special keys
            continue;
        }

        if (ch == 27) { // ESC
            if (showing) {
                showing = false;
                suggestions.clear();
                selected_idx = -1;
                std::cout << "\r\x1b[K\x1b[J";
                std::cout << "\r\x1b[K>> " << buffer << std::flush;
            }
            continue;
        }

        if (std::isprint(static_cast<unsigned char>(ch))) {
            buffer.push_back((char)ch);
            update_suggestions();
            render_box();
            continue;
        }
    }
#else
    termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        // fallback
        if (!std::getline(std::cin, buffer)) return std::string();
        return buffer;
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 1; // allow short inter-byte timeout for escape sequences
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

        if (ch == 127 || ch == 8) { // backspace
            if (!buffer.empty()) buffer.pop_back();
            update_suggestions();
            render_box();
            continue;
        }

        if (ch == '\t') { // Tab: accept selected suggestion
            if (showing && selected_idx >= 0 && selected_idx < (int)suggestions.size()) {
                std::string prefix = extract_current_word(buffer);
                buffer = buffer.substr(0, buffer.size() - prefix.size());
                buffer += suggestions[selected_idx];
                update_suggestions();
                render_box();
            }
            continue;
        }

        if (ch == '\x1b') {
            // possible arrow key sequence; read up to two more bytes with short timeout
            termios tmp = newt;
            tmp.c_cc[VMIN] = 0;
            tmp.c_cc[VTIME] = 1;
            tcsetattr(STDIN_FILENO, TCSANOW, &tmp);
            char seq1 = 0, seq2 = 0;
            ssize_t r1 = read(STDIN_FILENO, &seq1, 1);
            ssize_t r2 = read(STDIN_FILENO, &seq2, 1);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);

            if (r1 > 0 && seq1 == '[' && r2 > 0) {
                if (seq2 == 'A') { // up
                    if (showing && !suggestions.empty()) {
                        selected_idx = (selected_idx - 1 + (int)suggestions.size()) % (int)suggestions.size();
                        render_box();
                    }
                    continue;
                } else if (seq2 == 'B') { // down
                    if (showing && !suggestions.empty()) {
                        selected_idx = (selected_idx + 1) % (int)suggestions.size();
                        render_box();
                    }
                    continue;
                }
            }

            // ESC alone -> dismiss suggestions
            if (showing) {
                showing = false;
                suggestions.clear();
                selected_idx = -1;
                std::cout << "\r\x1b[K\x1b[J";
                std::cout << "\r\x1b[K>> " << buffer << std::flush;
            }
            continue;
        }

        if (std::isprint(static_cast<unsigned char>(ch))) {
            buffer.push_back(ch);
            update_suggestions();
            render_box();
            continue;
        }

        // otherwise ignore
    }

    restore();
    return buffer;
#endif
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