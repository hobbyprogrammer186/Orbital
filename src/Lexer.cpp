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

std::vector<ORB_TOKEN> Lexer::Tokenize(std::string value, std::string filename) {
	std::vector<ORB_TOKEN> tkns;
	ORB_TOKEN tkn;
	fname = filename;
	buffer = value;
	idx = 0;
	row = 1;
	col = 1;

	while (idx < value.length()) {
		char cur = value[idx];
		int lastIDX = idx;

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
			while (idx < value.length() && ((value[idx] >= 'a' && value[idx] <= 'z')
				|| (value[idx] >= 'A' && value[idx] <= 'Z') || (value[idx] >= '0' && value[idx] <= '9')
				|| value[idx] == '_'))
			{
				word += value[idx++];
			}
			
			auto it = keywords.find(word);
			if(it != keywords.end())
				tkn.token = (ORB_TOKEN_TYPE)it->second;
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
			if (idx < value.length() && value[idx] == '"') idx++;
			tkn.token = TOKEN_STRING;
			tkn.value = str;
		}
		else if (cur == '=') {
			if (idx + 1 < value.length() && value[idx + 1] == '=') {
				tkn.token = TOKEN_EQUAL;
				tkn.value = "==";
				idx += 2;
			} else {
				tkn.token = TOKEN_ASIGNMENT;
				tkn.value = "=";
				idx++;
			}
		}
		else if (cur == '>') {
			if (idx + 1 < value.length() && value[idx + 1] == '=') {
				tkn.token = TOKEN_GREATER_EQUAL;
				tkn.value = ">=";
				idx += 2;
			} else {
				tkn.token = TOKEN_GREATER;
				tkn.value = ">";
				idx++;
			}
		}
		else if (cur == '<') {
			if (idx + 1 < value.length() && value[idx + 1] == '=') {
				tkn.token = TOKEN_SHORTER_EQUAL;
				tkn.value = "<=";
				idx += 2;
			} else {
				tkn.token = TOKEN_SHORTER;
				tkn.value = "<";
				idx++;
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
			int consumed = idx - lastIDX;
			col += consumed;
			continue;
		}
		else if (cur == '#') {
			while (idx < value.length() && value[idx] != '\n')
				idx++; // Ignore all contents until end of line
			int consumed = idx - lastIDX;
			col += consumed;
			continue;
		}
		else if (cur == '.') {
			tkn.token = TOKEN_FULL_STOP;
			tkn.value = ".";
			idx++;
		}
		else {
			idx++;
			continue;
		}

		if (idx == lastIDX)
			idx++;

		int consumed = idx - lastIDX;
		if (tkn.token != TOKEN_EOL) {
			col += consumed;
		}

		tkns.push_back(tkn);
	}

	ORB_TOKEN eof;
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

void Lexer::error(ORB_TOKEN tkn, std::string msg) {
	int err_row = std::max(1, tkn.row);
	int err_col = std::max(1, tkn.col);

	if (!fname.empty())
		std::cout << fname << ":" << err_row << ":" << err_col << "\n";
	else
		std::cout << err_row << ":" << err_col << "\n";

	int start = std::max(1, err_row - PREVIOUS_LINE_BUFFER);

	for (int i = start; i <= err_row; ++i) {
		std::string line = getLine(buffer, i);
		if (!line.empty()) {
			std::cout << i << " | " << line << "\n";
		}
	}

	int prefix_len = (int)std::to_string(err_row).size() + 3; // e.g. "N | "
	int spaces_before_caret = prefix_len + (err_col - 1);
	for (int s = 0; s < spaces_before_caret; ++s) std::cout << ' ';
	std::cout << "^\n";

	for (int s = 0; s < spaces_before_caret; ++s) std::cout << ' ';
	std::cout << msg << "\n";

	exit(1);
}

void Lexer::warn(ORB_TOKEN tkn, std::string msg) {
	int wrn_row = std::max(1, tkn.row);
	int wrn_col = std::max(1, tkn.col);

	if (!fname.empty())
		std::cout << fname << ":" << wrn_row << ":" << wrn_col << "\n";
	else
		std::cout << wrn_row << ":" << wrn_col << "\n";

	int start = std::max(1, wrn_row - PREVIOUS_LINE_BUFFER);

	for (int i = start; i <= wrn_row; ++i) {
		std::string line = getLine(buffer, i);
		if (!line.empty()) {
			std::cout << i << " | " << line << "\n";
		}
	}

	int prefix_len = (int)std::to_string(wrn_row).size() + 3; // e.g. "N | "
	int spaces_before_caret = prefix_len + (wrn_col - 1);
	for (int s = 0; s < spaces_before_caret; ++s) std::cout << ' ';
	std::cout << "^\n";

	for (int s = 0; s < spaces_before_caret; ++s) std::cout << ' ';
	std::cout << "Warning: " << msg << "\n";
}