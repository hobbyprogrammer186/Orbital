/*
 * Parser  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdbool>
#include <cstdint>
#include <optional>
#include <variant>
#include <utility>
#include <Lexer.h>

typedef enum {
	DATA_TYPE_STRING,
	DATA_TYPE_INT,
	DATA_TYPE_FLOAT,
	DATA_TYPE_DOUBLE,
	DATA_TYPE_BOOLEAN
} DATA_TYPE;

struct VariableInfo {
    std::string value;
	DATA_TYPE dtype;
    bool isConst;
};

struct AST {
    virtual ~AST() = default;
};

struct NumberNode : AST {
	int value;
	NumberNode(int val) : value(val) {}
};

struct BinOpNode : AST {
	AST* left;
	TOKEN_TYPE opr;
	AST* right;

	BinOpNode(AST* l, TOKEN_TYPE otkn, AST* r) : left(l), opr(otkn), right(r) {}
};

struct AssignNode : AST {
    std::string name;
    AST* value;

    AssignNode(std::string n, AST* v)
        : name(std::move(n)), value(v) {}
};

struct VariableNode : AST {
    std::string name;
    VariableNode(std::string n) : name(std::move(n)) {}
};

struct StringNode : AST {
    std::string value;
    StringNode(std::string v) : value(std::move(v)) {}
};

struct FunctionNode : AST {
    std::string name;
	std::vector<std::string> args;
	std::vector<AST*> body;
	FunctionNode(std::string n) : name(std::move(n)) {}
    FunctionNode(std::string n, std::vector<AST*> b) : name(std::move(n)), body(std::move(b)) {}
	FunctionNode(std::string n, std::vector<std::string> a, std::vector<AST*> b) : name(std::move(n)), args(std::move(a)), body(std::move(b)) {}
	FunctionNode(std::string n, std::vector<std::string> a) : name(std::move(n)), args(std::move(a)) {}
};

struct CallNode : AST {
    std::string name;
    std::vector<AST*> args;

    CallNode(std::string n) : name(std::move(n)) {}
	CallNode(std::string n, std::vector<AST*> a) : name(std::move(n)), args(std::move(a)) {}
};

struct BooleanNode : AST {
	AST* left;
	AST* right;
	TOKEN_TYPE op;

	BooleanNode(AST* l, TOKEN_TYPE o, AST* r) : left(l), op(o), right(r) {}
};

class Parser {
private:
	std::vector<TOKEN> tokens;
	Lexer lex;
	size_t idx;

	TOKEN& current();
	bool match(TOKEN_TYPE type);
	void expect(TOKEN_TYPE type, std::optional<std::string> msg);
	AST* factor();
	AST* term();
	AST* expr();
	AST* comparison();
	AST* statement();
	std::vector<AST*> parse();
	std::vector<AST*> parseBlocks();
	AST* parseFunction();
public:
	const std::vector<TOKEN>& getTokens() const { return tokens; }
	Parser() = default;
	Parser(std::string buffer) : Parser(std::move(buffer), std::nullopt) {}
	Parser(std::string buffer, std::optional<std::string> filename) {
		tokens = lex.Tokenize(std::move(buffer), filename.value_or(""));
		idx = 0;
	}

	void Interpret();
	std::variant<double, long, int, std::string> Evalulate(AST* node);
};