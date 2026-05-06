/*
 * Lexer  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>

// Definations
#define PREVIOUS_LINE_BUFFER 		5

typedef enum {
	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_DECIMAL,
	TOKEN_ASIGNMENT,
	TOKEN_EQUAL,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
	TOKEN_XOR,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_GREATER,
	TOKEN_SHORTER,
	TOKEN_GREATER_EQUAL,
	TOKEN_SHORTER_EQUAL,
	TOKEN_COMMA,
	TOKEN_STRING_IGNORE,
	TOKEN_SEMICOLON,
    TOKEN_FULL_STOP,
	TOKEN_EOL,
	TOKEN_EOF,

    // Extended Tokens For Functions Defination
    TOKEN_FUNCTION,

	// Extended Tokens For Keywords
	TOKEN_IMPORT,
    TOKEN_CLASS,
    TOKEN_IS,
    TOKEN_IN,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ELIF,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_THROW,
    TOKEN_STRUCT,
    TOKEN_VOLATILE,
    TOKEN_VIEW_OBJECT
} TOKEN_TYPE;

static std::unordered_map<std::string, TOKEN_TYPE> keywords = {
    {"import", TOKEN_IMPORT},
    {"use", TOKEN_IMPORT},

    {"class", TOKEN_CLASS},

    {"is", TOKEN_IS},
    {"in", TOKEN_IN},

    {"and", TOKEN_AND},
    {"or", TOKEN_OR},
    {"not", TOKEN_NOT},

    {"True", TOKEN_TRUE},
    {"False", TOKEN_FALSE},
    {"None", TOKEN_NULL},

    {"const", TOKEN_CONST},

    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"elif", TOKEN_ELIF},

    {"Greater", TOKEN_GREATER},
    {"Shorter", TOKEN_SHORTER},

    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},

    {"for", TOKEN_FOR},
    {"while", TOKEN_WHILE},

    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"finish", TOKEN_CONTINUE},

    {"return", TOKEN_RETURN},
    {"throw", TOKEN_THROW},

    {"struct", TOKEN_STRUCT},
    {"nocache", TOKEN_VOLATILE},

    {"symbol", TOKEN_VIEW_OBJECT}
};

typedef struct TOKEN {
	TOKEN_TYPE token;
	int row = 1, col = 1;
	std::string value;
} TOKEN;

class Lexer {
private:
	int row = 1 , col = 1, idx = 0;
	char current;
	std::string fname;

	void Next(char cur);
	char Advance();
	char Peek();
	bool isNumber(char cur);
	bool isAlpha(char cur);
	bool isAtEnd();
	bool isalnum(char cur);

public:
	std::string buffer;
	std::vector<TOKEN> Tokenize(std::string input, std::string filename);
	void error(TOKEN tkn, std::string msg);
    void warn(TOKEN tkn, std::string msg);
};