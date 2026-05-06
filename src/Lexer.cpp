/*
 * Lexer  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <Lexer.h>

std::string getLine(const std::string& input, int targetLine) {
	std::istringstream stream(input);
	std::string line;
	int currentLine = 1;

	while (std::getline(stream, line)) {
		if (currentLine == targetLine)
			return line;
		currentLine++;
	}

	return "";
}

std::vector<TOKEN> Lexer::Tokenize(std::string value, std::string filename) {
	std::vector<TOKEN> tkns;
	TOKEN tkn;
	fname = filename;
	buffer = value;
	idx = 0;
	row = 1;
	col = 1;

	while (idx < value.length()) {
		char cur = value[idx];

		tkn.row = row;
		tkn.col = col;
		tkn.value = "";

		if (cur >= '0' && cur <= '9') {
			std::string num;
			while (idx < value.length() && value[idx] >= '0' && value[idx] <= '9') {
				num += value[idx++];
			}
			tkn.token = TOKEN_NUMBER;
			tkn.value = num;
		}
		else if ((cur >= 'a' && cur <= 'z') || (cur >= 'A' && cur <= 'Z') || cur == '_') {
			std::string word;
			while (idx < value.length() && ((value[idx] >= 'a' && value[idx] <= 'z') || (value[idx] >= 'A' && value[idx] <= 'Z') || (value[idx] >= '0' && value[idx] <= '9') || value[idx] == '_')) {
				word += value[idx++];
			}
			
			auto it = keywords.find(word);
			if(it != keywords.end())
				tkn.token = (TOKEN_TYPE)it->second;
			else
				tkn.token = TOKEN_IDENTIFIER;

			tkn.value = word;
		}
		else if (cur == '"') {
			std::string str;
			idx++;
			while (idx < value.length() && value[idx] != '"') {
				str += value[idx++];
			}
			idx++;
			tkn.token = TOKEN_STRING;
			tkn.value = str;
		}
		else if (cur == '=') {
			if (idx + 1 < value.length() && value[idx + 1] == '=') {
				tkn.token = TOKEN_EQUAL;
				tkn.value = "==";
				idx += 2;
				col += 2;
			} else {
				tkn.token = TOKEN_ASIGNMENT;
				tkn.value = "=";
				idx++;
				col++;
			}
		}
		else if (cur == '>') {
			if (idx + 1 < value.length() && value[idx + 1] == '=') {
				tkn.token = TOKEN_GREATER_EQUAL;
				tkn.value = ">=";
				idx += 2;
				col += 2;
			} else {
				tkn.token = TOKEN_GREATER;
				tkn.value = ">";
				idx++;
				col++;
			}
		}
		else if (cur == '<') {
			if (idx + 1 < value.length() && value[idx + 1] == '=') {
				tkn.token = TOKEN_SHORTER_EQUAL;
				tkn.value = "<=";
				idx += 2;
				col += 2;
			} else {
				tkn.token = TOKEN_SHORTER;
				tkn.value = "<";
				idx++;
				col++;
			}
		}
		else if (cur == '+') {
			tkn.token = TOKEN_PLUS;
			tkn.value = "+";
			idx++;
		}
		else if (cur == '-') {
			tkn.token = TOKEN_MINUS;
			tkn.value = "-";
			idx++;
		}
		else if (cur == '*') {
			tkn.token = TOKEN_MULTIPLY;
			tkn.value = "*";
			idx++;
		}
		else if (cur == '/') {
			tkn.token = TOKEN_DIVIDE;
			tkn.value = "/";
			idx++;
		}
		else if (cur == '(') {
			tkn.token = TOKEN_LPAREN;
			tkn.value = "(";
			idx++;
		}
		else if (cur == ')') {
			tkn.token = TOKEN_RPAREN;
			tkn.value = ")";
			idx++;
		}
		else if (cur == ';') {
			tkn.token = TOKEN_SEMICOLON;
			tkn.value = ";";
			idx++;
		}
		else if (cur == ':') {
			tkn.token = TOKEN_ASIGNMENT;
			tkn.value = ":";
			idx++;
		}
		else if (cur == '\n') {
			tkn.token = TOKEN_EOL;
			tkn.value = "\n";
			idx++;
			row++;
			col = 1;
		}
		else if (cur == ',') {
			tkn.token = TOKEN_COMMA;
			tkn.value = ",";
			idx++;
		}
		else if (cur == ' ' || cur == '\t' || cur == '\r') {
			idx++;
			col++;
			continue;
		}
		else if (cur == '#') {
			while(value[idx] != '\n')
				idx++; // Ignore All Contents While Not End The Line
			
			continue;
		}
		else if (cur == '.') {
			tkn.token = TOKEN_FULL_STOP;
			tkn.value = ".";
		}
		else {
			idx++;
			continue;
		}
		tkns.push_back(tkn);
	}

	TOKEN eof;
	eof.token = TOKEN_EOF;
	eof.value = "";
	eof.row = row;
	eof.col = col;
	tkns.push_back(eof);

	return tkns;
}

char Lexer::Peek() {
	if (isAtEnd()) return '\0';
	return buffer[idx];
}

bool Lexer::isAtEnd() {
	return idx >= buffer.length();
}

void Lexer::error(TOKEN tkn, std::string msg) {
    // Print location header
    if (!fname.empty())
        std::cout << fname << ":" << tkn.row << ":" << tkn.col << "\n";
    else
        std::cout << tkn.row << ":" << tkn.col << "\n";

    int start = std::max(1, tkn.row - PREVIOUS_LINE_BUFFER);

    // Margin Problem Is Here (Overmargin)
    for (int i = start; i <= tkn.row; i++) {
        std::string line = getLine(buffer, i);
        if (!line.empty()) {
            std::cout << i << " | " << line << "\n";
        }
    }
	
	// Margin Problem Is Here (Undermargin)
    for (int i = 1; i < tkn.col; i++) {
        std::cout << " ";
    }

    std::cout << "^\n";
    for (int i = 1; i < tkn.col; i++) {
        std::cout << " ";
    }
    std::cout << msg << "\n";

    exit(1);
}

void Lexer::warn(TOKEN tkn, std::string msg) {
    // Print location header
    if (!fname.empty())
        std::cout << fname << ":" << tkn.row << ":" << tkn.col << "\n";
    else
        std::cout << tkn.row << ":" << tkn.col << "\n";

    int start = std::max(1, tkn.row - PREVIOUS_LINE_BUFFER);

    // Margin Problem Is Here (Overmargin)
    for (int i = start; i <= tkn.row; i++) {
        std::string line = getLine(buffer, i);
        if (!line.empty()) {
            std::cout << i << " | " << line << "\n";
        }
    }
	
	// Margin Problem Is Here (Undermargin)
    for (int i = 1; i < tkn.col; i++) {
        std::cout << " ";
    }

    std::cout << "^\n";
    for (int i = 1; i < tkn.col; i++) {
        std::cout << " ";
    }
	std::cout << "Warning: ";
    std::cout << msg << "\n";
}