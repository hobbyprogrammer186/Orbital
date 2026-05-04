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

namespace fs = std::filesystem;
#if defined(_WIN32) || defined(_WIN64)
    #include <curses.h>
    #include <conio.h>
#elif defined(__APPLE__) || defined(__MACH__) || defined(__linux__)
    #include <ncurses.h>
    #include <unistd.h>
    #include <termios.h>
#endif



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

static const int MAX_VISIBLE_SUGGESTIONS = 8;

static void clear_line();

static std::vector<std::string> get_matching_suggestions(const std::string& prefix) {
    std::vector<std::string> matches;
    if (prefix.empty()) return matches;

    std::vector<std::string> all = get_all_suggestions();
    for (const auto& kw : all) {
        if (kw.size() > prefix.size() &&
            kw.substr(0, prefix.size()) == prefix) {
            matches.push_back(kw);
        }
    }
    return matches;
}

static void render_suggestion_box(const std::string& prompt, const std::string& buffer,
                                   const std::vector<std::string>& suggestions, int selected_idx) {
    clear_line();
    printw("%s%s\n", prompt.c_str(), buffer.c_str());

    if (suggestions.empty()) {
        refresh();
        return;
    }

    int start = 0;
    int end = (int)suggestions.size();
    if (end > MAX_VISIBLE_SUGGESTIONS) {
        if (selected_idx < MAX_VISIBLE_SUGGESTIONS / 2) {
            end = MAX_VISIBLE_SUGGESTIONS;
        } else if (selected_idx >= end - MAX_VISIBLE_SUGGESTIONS / 2) {
            start = end - MAX_VISIBLE_SUGGESTIONS;
        } else {
            start = selected_idx - MAX_VISIBLE_SUGGESTIONS / 2;
            end = start + MAX_VISIBLE_SUGGESTIONS;
        }
    }

    attron(COLOR_PAIR(8));
    addstr("+--------------------------------------+\n");
    attroff(COLOR_PAIR(8));

    for (int i = start; i < end; i++) {
        attron(COLOR_PAIR(8));
        addstr("| ");
        attroff(COLOR_PAIR(8));

        if (i == selected_idx) {
            attron(A_REVERSE);
            printw(" %-36s", suggestions[i].c_str());
            attroff(A_REVERSE);
        } else {
            printw("  %-36s", suggestions[i].c_str());
        }

        attron(COLOR_PAIR(8));
        addstr("|\n");
        attroff(COLOR_PAIR(8));
    }

    if ((int)suggestions.size() > MAX_VISIBLE_SUGGESTIONS) {
        std::string scroll_info = std::to_string(start + 1) + "-" + std::to_string(end) + "/" + std::to_string((int)suggestions.size());
        attron(COLOR_PAIR(8));
        attron(COLOR_PAIR(4));
        printw("  %-36s|\n", scroll_info.c_str());
        attroff(COLOR_PAIR(4));
        attroff(COLOR_PAIR(8));
    }

    attron(COLOR_PAIR(8));
    addstr("+--------------------------------------+\n");
    addstr("[Tab] accept  [UP/DOWN] navigate  [Esc] dismiss\n");
    attroff(COLOR_PAIR(8));

    int rows = (end - start) + 6;
    while (rows-- > 0) move(getcury(stdscr) - 1, 0);
    refresh();
    printw("%s%s", prompt.c_str(), buffer.c_str());
    refresh();
}

static void clear_line() {
    move(getcury(stdscr), 0);
    clrtoeol();
    refresh();
}

static void clear_suggestion_display() {
    int max_lines = 12;
    while (max_lines-- > 0) {
        move(getcury(stdscr) - 1, 0);
        clrtoeol();
    }
    refresh();
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
    std::string current_word;
    std::vector<std::string> suggestions;
    int selected_idx = -1;
    bool showing_suggestions = false;
    std::string prompt = ">> ";

    auto extract_current_word = [&]() {
        std::string word;
        for (int i = (int)buffer.size() - 1; i >= 0; i--) {
            char c = buffer[i];
            if (c == ' ' || c == '\t') break;
            word = c + word;
        }
        return word;
    };

    auto refresh_display = [&]() {
        current_word = extract_current_word();
        suggestions = get_matching_suggestions(current_word);
        if (!suggestions.empty() && !current_word.empty()) {
            showing_suggestions = true;
            if (selected_idx < 0) selected_idx = 0;
            if (selected_idx >= (int)suggestions.size()) selected_idx = (int)suggestions.size() - 1;
            render_suggestion_box(prompt, buffer, suggestions, selected_idx);
        } else {
            if (showing_suggestions) {
                clear_suggestion_display();
                showing_suggestions = false;
            }
            clear_line();
            printw("%s%s", prompt.c_str(), buffer.c_str());
            refresh();
            selected_idx = -1;
        }
    };

    refresh_display();

    while(1) {
        int c = getch();

        if (c == KEY_UP && showing_suggestions) {
            selected_idx--;
            if (selected_idx < 0) selected_idx = (int)suggestions.size() - 1;
            render_suggestion_box(prompt, buffer, suggestions, selected_idx);
            continue;
        }
        else if (c == KEY_DOWN && showing_suggestions) {
            selected_idx++;
            if (selected_idx >= (int)suggestions.size()) selected_idx = 0;
            render_suggestion_box(prompt, buffer, suggestions, selected_idx);
            continue;
        }
        else if (c == 27) {
            if (showing_suggestions) {
                clear_suggestion_display();
                showing_suggestions = false;
                selected_idx = -1;
                clear_line();
                printw("%s%s", prompt.c_str(), buffer.c_str());
                refresh();
            }
        }
        else if (c == '\n' || c == '\r' || c == KEY_ENTER) {
            if (showing_suggestions) {
                clear_suggestion_display();
            }
            clear_line();
            printw("%s%s\n", prompt.c_str(), buffer.c_str());
            refresh();
            return buffer;
        }
        else if (c == KEY_BACKSPACE || c == 127 || c == 8) {
            if (!buffer.empty()) {
                buffer.pop_back();
                refresh_display();
            }
        }
        else if (c == '\t' || c == KEY_BTAB) {
            if (showing_suggestions && selected_idx >= 0 && selected_idx < (int)suggestions.size()) {
                buffer = buffer.substr(0, buffer.size() - current_word.size());
                buffer += suggestions[selected_idx];
                refresh_display();
            }
        }
        else if (c >= 32 && c <= 126) {
            buffer += c;
            refresh_display();
        }
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

        Parser parser(buffer);
        parser.Interpret();
        file.close();
    }
    else {
        auto now = std::chrono::system_clock::now();
        auto nyear = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(now)).year();
        int yearInt = static_cast<int>(nyear);
        std::string copyrightYear = yearInt > 2026 ? std::to_string(yearInt) : "2026";

        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        if (has_colors()) {
            start_color();
            init_pair(4, COLOR_CYAN, COLOR_BLACK);
            init_pair(8, COLOR_BLACK, COLOR_BLACK);
        }
        printw((std::string(NAME_STRING) + " Programming Language " + VERSION_STRING + "\n").c_str());
        printw(("Copyright (c) " + copyrightYear + " First Person\n\n").c_str());
        refresh();

        while(1) {
            Parser parser(input());
            parser.Interpret();
        }
        std::atexit([](){ endwin(); }); // Register Exit To Destory The Canvas
    }
}