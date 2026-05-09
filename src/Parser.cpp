/*
 * Parser  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <utility>
#include <variant>
#include <map>
#include <set>
#include <Parser.h>
#include <Lexer.h>
#include <orbtlio.h>
#include <definations.h>

std::map<std::string, VariableInfo> variableTable;
std::map<std::string, FunctionNode*> functionTable;
static std::set<std::string> importedFiles;

void Parser::Interpret() {
    std::vector<AST*> nodes = parse();
    for(AST* node : nodes) {
        Evalulate(node);
    }
}

ORB_TOKEN& Parser::current() {
    return tokens[idx];
}

bool Parser::match(ORB_TOKEN_TYPE type) {
    if(current().token == type) {
        idx++;
        return true;
    }
    return false;
}

void Parser::expect(ORB_TOKEN_TYPE type, std::optional<std::string> msg) {
    if(match(type))
        return;

    lex.error(current(), msg.value_or("Syntax Error."));
}

AST* Parser::factor() {
    ORB_TOKEN& tkn = current();

    if(match(TOKEN_NUMBER))
        return new NumberNode(std::stod(tkn.value));
    else if(match(TOKEN_STRING))
        return new StringNode(tkn.value);
    else if(match(TOKEN_TRUE))
        return new StringNode("True");
    else if(match(TOKEN_FALSE))
        return new StringNode("False");
    else if(match(TOKEN_NULL))
        return new StringNode("None");
    else if(match(TOKEN_LPAREN)) {
        AST* node = expr();
        expect(TOKEN_RPAREN, "Bracket Is Not Terminated.");
        return node;
    }
    else if(match(TOKEN_IDENTIFIER)) {
        if(functionTable.find(tkn.value) != functionTable.end() && idx < tokens.size() && tokens[idx].token == TOKEN_LPAREN) {
            // parse parenthesized function call: identifier '(' [expr (',' expr)*] ')'
            idx++; // skip '('

            std::vector<AST*> args;
            // If immediate closing paren, return empty-arg call
            if (current().token != TOKEN_RPAREN) {
                while (true) {
                    AST* a = comparison();
                    if (!a) return nullptr;
                    args.push_back(a);

                    if (current().token == TOKEN_COMMA) {
                        idx++; // consume comma and continue
                        continue;
                    }
                    break;
                }
            }

            expect(TOKEN_RPAREN, "Bracket Is Not Terminated.");
            if(!args.empty())
                return new CallNode(tkn.value, args);
            else
                return new CallNode(tkn.value);
        }
        else if(functionTable.find(tkn.value) != functionTable.end() && idx < tokens.size() && current().token != TOKEN_LPAREN) {
            while(current().value == tkn.value)
                idx++;
            
            std::vector<AST*> args;
            while(current().token != TOKEN_EOL && current().token != TOKEN_EOF) {
                args.push_back(comparison());

                if(functionTable.find(current().value) != functionTable.end()) {
                    std::string functionName = current().value;
                    std::vector<AST*> fargs;
                    idx++;
                    for(std::string arg : functionTable.find(functionName)->second->args) {
                        if(current().token == TOKEN_SEMICOLON)
                            break;
                        
                        if(current().token == TOKEN_COMMA)
                            idx++;
                        else if(current().token != TOKEN_EOL && current().token != TOKEN_FULL_STOP) {
                            expect(TOKEN_RPAREN, "Expected/Missing '.'");
                            return nullptr;
                        }
                        else
                            fargs.push_back(factor());
                    }
                    auto val = Evalulate(new CallNode(functionName, fargs));
                    std::string str_val;
                    std::visit([&str_val](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            str_val = arg;
                        } else {
                            str_val = std::to_string(arg);
                        }
                    }, val);
                    args.push_back(new StringNode(str_val));
                }

                if(current().token == TOKEN_COMMA)
                    idx++;
                else
                    break;
            }

            if(!args.empty())
                return new CallNode(tkn.value, args);
            else
                return new CallNode(tkn.value);
        }
        else
            return new VariableNode(tkn.value, tkn.row, tkn.col);
    }
    else {
        lex.error(current(), "Invalid Bracket.");
        return nullptr;
    }
}

AST* Parser::term() {
    AST* node = factor();

    while(current().token == TOKEN_MULTIPLY || current().token == TOKEN_DIVIDE) {
        ORB_TOKEN_TYPE op = current().token;
        idx++;
        node = new BinOpNode(node, op, factor());
    }

    return node;
}

AST* Parser::expr() {
    AST* node = term();
    while(current().token == TOKEN_PLUS || current().token == TOKEN_MINUS) {
        ORB_TOKEN_TYPE op = current().token;
        idx++;
        node = new BinOpNode(node, op, term());
    }
    return node;
}

AST* Parser::comparison() {
    AST* node = expr();
    while(current().token == TOKEN_GREATER || current().token == TOKEN_SHORTER
            || current().token == TOKEN_GREATER_EQUAL || current().token == TOKEN_SHORTER_EQUAL
            || current().token == TOKEN_EQUAL)
    {
        ORB_TOKEN_TYPE op = current().token;
        idx++;
        AST* right = expr();
        node = new BooleanNode(node, op, right);
    }
    return node;
}

AST* Parser::statement() {
    if (current().token == TOKEN_IMPORT) {
        // parse: import <path1>, <path2> .
        idx++; // consume 'import'
        std::vector<std::string> pths;
        while (idx < tokens.size() && current().token != TOKEN_FULL_STOP && current().token != TOKEN_EOL && current().token != TOKEN_EOF) {
            if (current().token == TOKEN_COMMA) {
                idx++;
                continue;
            }

            if (current().token == TOKEN_STRING) {
                pths.push_back(current().value);
                idx++;
                continue;
            } else if (current().token == TOKEN_IDENTIFIER) {
                std::string path = current().value;
                idx++;
                // assemble dotted identifiers like hello_world.obt
                while (idx < tokens.size() && current().token == TOKEN_FULL_STOP) {
                    idx++; // consume '.'
                    if (idx < tokens.size() && current().token == TOKEN_IDENTIFIER) {
                        path += "." + current().value;
                        idx++;
                    } else {
                        lex.error(current(), "Invalid import path.");
                        return nullptr;
                    }
                }
                pths.push_back(path);
                continue;
            }

            lex.error(current(), "Invalid import path.");
            return nullptr;
        }

        return new ImportNode(pths);
    }
    if(current().token == TOKEN_IDENTIFIER) {
        if(idx < tokens.size() - 1) {
            ORB_TOKEN& next = tokens[idx + 1];
            
            if(next.token == TOKEN_ASIGNMENT && next.value == ":")
                return parseFunction();
            else if(next.token == TOKEN_LPAREN && current().token == TOKEN_IDENTIFIER) {
                size_t parenIdx = idx + 1;
                while(parenIdx < tokens.size() && tokens[parenIdx].token != TOKEN_RPAREN) {
                    parenIdx++;
                }
                if(parenIdx < tokens.size() - 1 && tokens[parenIdx + 1].token == TOKEN_ASIGNMENT && 
                   tokens[parenIdx + 1].value == ":") {
                    return parseFunction();
                }
            }
            else if(next.token == TOKEN_ASIGNMENT) {
                std::string name = current().value;
                idx++;
                match(TOKEN_ASIGNMENT);
                
                AST* value = expr();
                return new AssignNode(name, value);
            }
            else if(next.token == TOKEN_GREATER || next.token == TOKEN_SHORTER
                    || next.token == TOKEN_GREATER_EQUAL || next.token == TOKEN_SHORTER_EQUAL
                    || next.token == TOKEN_EQUAL)
            {
                AST* left = expr();
                ORB_TOKEN_TYPE op = current().token;
                idx++;
                AST* right = expr();
                
                // If Failed To Parse Condition Othwise Returns BooleanNode
                if(left == nullptr || right == nullptr) {
                    lex.error(current(), "Syntax Error.");
                    return nullptr;
                }
                else
                    return new BooleanNode(left, op, right);
            }
            else if(current().token == TOKEN_IF) {
                idx++;
                bool useParens = match(TOKEN_LPAREN);
                
                AST* condition = comparison();
                if(useParens)
                    expect(TOKEN_RPAREN, "Expected ')' to close condition.");
                match(TOKEN_ASIGNMENT);

                if(current().token == TOKEN_EOL)
                    idx++;

                int functionIndent = current().col;
                BooleanNode* cond = dynamic_cast<BooleanNode*>(condition);
                if (!cond) {
                    lex.error(tokens[idx - 1], "Condition is not boolean.");
                    return nullptr;
                }

                std::vector<AST*> body;
                while(idx < tokens.size() && current().token != TOKEN_EOF) {
                    if(current().token == TOKEN_EOL) {
                        idx++;
                        continue;
                    }
					/*
					Chnaged this to the below one:
                    if(current().token == TOKEN_IDENTIFIER && current().col < functionIndent)
                        break;
                    */
                    if (current().col < functionIndent)
                        break;
                    body.push_back(statement());

                    if(current().token == TOKEN_EOL)
                        idx++;
                }

                if(!body.empty())
                    return new IfNode(cond, body);
                else
                    return new IfNode(cond);
            }
            else if(current().token == TOKEN_RETURN) {
                if(next.value == "")
                    return new ReturnNode();
                else
                    return new ReturnNode(next.value);
            }
            else if(current().token == TOKEN_IMPORT) {
                // parse: import <path1>, <path2> .
                idx++; // consume 'import'
                std::vector<std::string> pths;
                while(idx < tokens.size() && current().token != TOKEN_FULL_STOP && current().token != TOKEN_EOL && current().token != TOKEN_EOF) {
                    if(current().token == TOKEN_COMMA) {
                        idx++;
                        continue;
                    }

                    if (current().token == TOKEN_STRING) {
                        pths.push_back(current().value);
                        idx++;
                        continue;
                    } else if (current().token == TOKEN_IDENTIFIER) {
                        std::string path = current().value;
                        idx++;
                        // assemble dotted identifiers like hello_world.obt
                        while (idx < tokens.size() && current().token == TOKEN_FULL_STOP) {
                            idx++; // consume '.'
                            if (idx < tokens.size() && current().token == TOKEN_IDENTIFIER) {
                                path += "." + current().value;
                                idx++;
                            } else {
                                lex.error(current(), "Invalid import path.");
                                return nullptr;
                            }
                        }
                        pths.push_back(path);
                        continue;
                    }

                    lex.error(current(), "Invalid import path.");
                    return nullptr;
                }

                return new ImportNode(pths);
            }
        }
    }
    else if(current().token == TOKEN_IF) {
        idx++;
        bool useParens = match(TOKEN_LPAREN);
        
        AST* condition = comparison();
        if(useParens)
            expect(TOKEN_RPAREN, "Expected ')' to close condition.");
        match(TOKEN_ASIGNMENT);

        if(current().token == TOKEN_EOL)
            idx++;

        int functionIndent = current().col;
        BooleanNode* cond = dynamic_cast<BooleanNode*>(condition);
        if (!cond) {
            lex.error(tokens[idx - 1], "Condition is not boolean.");
            return nullptr;
        }

        std::vector<AST*> body;
        while(idx < tokens.size() && current().token != TOKEN_EOF) {
            if(current().token == TOKEN_EOL) {
                idx++;
                continue;
            }
			/* Changed this to the below one:
            if(current().token == TOKEN_IDENTIFIER && current().col < functionIndent)
                break;
            */    
            if (current().col < functionIndent)
            	break;
            
            body.push_back(statement());

            if(current().token == TOKEN_EOL)
                idx++;
        }

        if(!body.empty())
            return new IfNode(cond, body);
        else
            return new IfNode(cond);
    }
    else return expr();
    return expr();
}

AST* Parser::parseFunction() {
    std::string name = current().value;
    std::vector<std::string> args;
    int functionIndent = current().col;
    FunctionNode* fn = nullptr;
    idx++;

    if(current().token == TOKEN_ASIGNMENT && current().value == ":") {
        if(isBuiltin(name)) {
            lex.error(current(), "Builtin Funciton Cannot Be Overrided/Overwritten.");
            return nullptr;
        }
        else if(functionTable.find(name) != functionTable.end()) {
            functionTable.erase(name);
        }
        
        idx++;
        if(current().token == TOKEN_EOL)
            idx++;
        
        std::vector<AST*> body;
        while(idx < tokens.size() && current().token != TOKEN_EOF) {
            if(current().token == TOKEN_EOL) {
                idx++;
                continue;
            }
            
            if(current().token == TOKEN_IDENTIFIER && current().col <= functionIndent)
                break;
                
            body.push_back(statement());
            
            if(current().token == TOKEN_EOL)
                idx++;
        }
        
        // std::move can be expect in some cases if variable value is empty
        if(!body.empty()) {
            if(!args.empty())
                fn = new FunctionNode(name, args, body);
            else
                fn = new FunctionNode(name, body);
        }
        else {
            if(!args.empty())
                fn = new FunctionNode(name, args);
            else
                fn = new FunctionNode(name);
        }
        functionTable[name] = fn;
        return fn;
    }
    else if(current().token == TOKEN_LPAREN && current().value == "(") {
        idx++;
        current().token == TOKEN_EOL;
        
        idx++;
        if(current().token == TOKEN_ASIGNMENT && current().value == ":") {
            if(isBuiltin(name)) {
                lex.error(current(), "Builtin Funciton Cannot Be Overrided/Overwritten.");
                return nullptr;
            }

            idx++;
            if(current().token == TOKEN_EOL)
                idx++;
            
            std::vector<AST*> body;
            while(idx < tokens.size() && current().token != TOKEN_EOF) {
                if(current().token == TOKEN_EOL) {
                    idx++;
                    continue;
                }
                
                if(current().token == TOKEN_IDENTIFIER && current().col <= functionIndent)
                    break;
                    
                body.push_back(statement());
                
                if(current().token == TOKEN_EOL)
                    idx++;
            }
            
            // std::move can be expect in some cases if variable value is empty
            if(!body.empty()) {
                if(!args.empty())
                    fn = new FunctionNode(name, args, body);
                else
                    fn = new FunctionNode(name, body);
            }
            else {
                if(!args.empty())
                    fn = new FunctionNode(name, args);
                else
                    fn = new FunctionNode(name);
            }
            functionTable[name] = fn;
            return fn;
        }
    }
    
    lex.error(current(), "Expected/Missing ':' After Function Declearation.");
    return nullptr;
}

std::vector<AST*> Parser::parse() {
    std::vector<AST*> nodes;
    while(idx < tokens.size() && current().token != TOKEN_EOF) {
        if(current().token == TOKEN_EOL || current().token == TOKEN_FULL_STOP || current().token == TOKEN_SEMICOLON) {
            idx++;
            continue;
        }
        
        nodes.push_back(statement());
        
        if(current().token == TOKEN_EOL || current().token == TOKEN_FULL_STOP || current().token == TOKEN_SEMICOLON)
            idx++;
    }
    return nodes;
}

std::vector<AST*> Parser::parseBlocks() {
    std::string name = current().value;
    std::vector<AST*> nodes;
    idx++;

    if(current().token == TOKEN_EOL)
        idx++;

    while(idx < tokens.size() && current().token != TOKEN_EOF) {
        if(current().token == TOKEN_EOL || current().token == TOKEN_SEMICOLON) {
            idx++;
            continue;
        }

        // Stop When Top Level Statement Sart
        if(current().token == TOKEN_IDENTIFIER && idx + 1 < tokens.size() && tokens[idx + 1].row != current().row)
            break;

        nodes.push_back(statement());

        if(current().token == TOKEN_EOL || current().token == TOKEN_FULL_STOP || current().token == TOKEN_SEMICOLON)
            idx++;
    }
    return nodes;
}

std::variant<double, long, int, std::string> Parser::Evalulate(AST* node) {
    if(auto n = dynamic_cast<NumberNode*>(node))
        return n->value;
    else if(auto s = dynamic_cast<StringNode*>(node))
        return s->value;
    else if(auto v = dynamic_cast<VariableNode*>(node)) {
        if (variableTable.find(v->name) == variableTable.end()) {
            ORB_TOKEN err_tkn;
            err_tkn.row = v->row;
            err_tkn.col = v->col;
            err_tkn.value = v->name;
            lex.error(err_tkn, "Undefined variable/function: " + v->name);
            return 0L;
        }
        const std::string& val = variableTable[v->name].value;
        
        try {
            size_t pos = 0;
            double d = std::stod(val, &pos);
            if (pos == val.size()) {
                return d;
            }
        } catch(...) {}
        
        try {
            size_t pos = 0;
            long l = std::stol(val, &pos);
            if (pos == val.size()) {
                return l;
            }
        } catch(...) {
            return val;
        }
    }
    else if(auto b = dynamic_cast<BinOpNode*>(node)) {
        std::variant<double, long, int, std::string> leftVal = Evalulate(b->left);
        std::variant<double, long, int, std::string> rightVal = Evalulate(b->right);
        
        bool isString = std::holds_alternative<std::string>(leftVal) || std::holds_alternative<std::string>(rightVal);
        
        if(b->opr == TOKEN_PLUS && isString) {
            std::string leftStr = std::visit([](auto&& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::string>) return v;
                else return std::to_string(v);
            }, leftVal);
            std::string rightStr = std::visit([](auto&& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::string>) return v;
                else return std::to_string(v);
            }, rightVal);
            return leftStr + rightStr;
        }

        auto getDouble = [](const auto& v) -> double {
            return std::visit([](auto&& arg) -> double {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, double>) return arg;
                else if constexpr (std::is_same_v<T, long>) return static_cast<double>(arg);
                else if constexpr (std::is_same_v<T, int>) return static_cast<double>(arg);
                else return 0.0;
            }, v);
        };

        double left = getDouble(leftVal);
        double right = getDouble(rightVal);

        switch(b->opr) {
            case TOKEN_PLUS:
                return left + right;
            case TOKEN_MINUS:
                return left - right;
            case TOKEN_MULTIPLY:
                return left * right;
            case TOKEN_DIVIDE:
                if(left == 0 || right == 0) {
                    lex.error(tokens[idx], "Cannot Divide By Zero");
                    return 0.0;
                }
                else
                    return left / right;
        }
    }
    else if(auto c = dynamic_cast<CallNode*>(node)) {
        if(functionTable.find(c->name) == functionTable.end()) {
            lex.error(current(), "Undefined variable/function");
            return 0L;
        }

        FunctionNode* fn = functionTable[c->name];
        if(fn->args.size() > c->args.size()) {
            lex.error(current(), std::to_string(fn->args.size() - c->args.size()) + " Parameter/Argurement Is Required But Missing.");
            return 0L;
        }
        else if(fn->args.size() < c->args.size()) {
            std::string totalArgs = std::to_string(fn->args.size() - c->args.size());
            lex.error(current(), "Over " + totalArgs + " Parameter/Argurement Found But Not Defiend " + totalArgs + " Overload On The" + c->name + " Function.");
            return 0L;
        }

        std::map<std::string, VariableInfo> backup;

        for (size_t i = 0; i < fn->args.size(); i++) {
            const std::string& argName = fn->args[i];

            auto it = variableTable.find(argName);
            if (it != variableTable.end())
                backup[argName] = it->second;

            std::variant<double, long, int, std::string> argValue = Evalulate(c->args[i]);

            VariableInfo vinfo;
            vinfo.value = std::visit([](auto&& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::string>) return v;
                else return std::to_string(v);
            }, argValue);
            vinfo.dtype = std::visit([&](auto&& v) -> DATA_TYPE {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::string>) return DATA_TYPE_STRING;
                else if constexpr (std::is_same_v<T, int>) return DATA_TYPE_INT;
                else if constexpr (std::is_same_v<T, float>) return DATA_TYPE_FLOAT;
                else if constexpr (std::is_same_v<T, double>) return DATA_TYPE_DOUBLE;
                else if (vinfo.value == "True" || vinfo.value == "False") return DATA_TYPE_BOOLEAN;
                else return DATA_TYPE_STRING;
            }, argValue);
            vinfo.isConst = false;

            variableTable[argName] = vinfo;
        }

        if(isBuiltin(c->name)) {
            try {
                return exec(c);
            }
            catch(const std::exception& e) {
                lex.error(current(), e.what());
                std::exit(1);
            }
        }
        for (AST* stmt : fn->body) {
            if(auto ret = dynamic_cast<ReturnNode*>(stmt))
                return ret->value;
            else
                Evalulate(stmt);
        }

        for(std::string argName : fn->args) {
            if(backup.find(argName) != backup.end()) {
                variableTable[argName] = backup.find(argName)->second;
            }
        }

        return 0L; // Proper Implement Return Function Later
    }
    else if(auto ifnd = dynamic_cast<IfNode*>(node)) {
        std::variant<double, long, int, std::string> condition = Evalulate(ifnd->condition);
        if(std::holds_alternative<std::string>(condition)) {
            std::string condStr = std::get<std::string>(condition);
            if(condStr == "True" || condStr == "False") {
                if(condStr == "True") {
                    for(AST* stmt : ifnd->body) {
                        if(auto ret = dynamic_cast<ReturnNode*>(stmt))
                            return ret->value;
                        Evalulate(stmt);
                    }
                    
                    return 0L;
                }
                else
                    return 0L;
            }
            else {
                lex.error(current(), "Condition is not boolean.");
                return 0L;
            }
        }
        else {
            lex.error(current(), "Condition is not boolean.");
            return 0L;
        }
    }
    else if(auto fn = dynamic_cast<FunctionNode*>(node))
        return 0L;
    else if(auto a = dynamic_cast<AssignNode*>(node)) {
        if(isBuiltin(a->name)) {
            lex.error(current(), "Builtin Funciton Cannot Be Overrided/Overwritten.");
            return 0L;
        }

        if(functionTable.find(a->name) != functionTable.end())
            functionTable.erase(a->name);

        std::variant<double, long, int, std::string> val = Evalulate(a->value);
        VariableInfo vinfo;
        vinfo.value = std::visit([](auto&& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) return v;
            else return std::to_string(v);
        }, val);
        vinfo.dtype = std::visit([&](auto&& v) -> DATA_TYPE {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) return DATA_TYPE_STRING;
            else if constexpr (std::is_same_v<T, int>) return DATA_TYPE_INT;
            else if constexpr (std::is_same_v<T, float>) return DATA_TYPE_FLOAT;
            else if constexpr (std::is_same_v<T, double>) return DATA_TYPE_DOUBLE;
            else if (vinfo.value == "True" || vinfo.value == "False") return DATA_TYPE_BOOLEAN;
            else return DATA_TYPE_STRING;
        }, val);
        vinfo.isConst = false;

        variableTable.insert_or_assign(a->name, vinfo);
        return val;
    }
    else if(auto bl = dynamic_cast<BooleanNode*>(node)) {
        std::variant<double, long, int, std::string> l = Evalulate(bl->left);
        std::variant<double, long, int, std::string> r = Evalulate(bl->right);

        if (
            (std::holds_alternative<int>(l) || std::holds_alternative<double>(l) || std::holds_alternative<long>(l)) &&
            (std::holds_alternative<int>(r) || std::holds_alternative<double>(r) || std::holds_alternative<long>(r))
        )
        {
            auto toDouble = [](const auto& v) -> double {
                return std::visit([](auto&& arg) -> double {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, double>) return arg;
                    else if constexpr (std::is_same_v<T, long>) return static_cast<double>(arg);
                    else if constexpr (std::is_same_v<T, int>) return static_cast<double>(arg);
                    else return 0.0;
                }, v);
            };

            double ld = toDouble(l);
            double rd = toDouble(r);

            switch(bl->op) {
                case TOKEN_GREATER: return ld > rd ? "True" : "False";
                case TOKEN_GREATER_EQUAL: return ld >= rd ? "True" : "False";
                case TOKEN_SHORTER: return ld < rd ? "True" : "False";
                case TOKEN_SHORTER_EQUAL: return ld <= rd ? "True" : "False";
                case TOKEN_EQUAL: return ld == rd ? "True" : "False";
                default:
                    lex.error(current(), "Cannot Compare " + std::to_string(ld) + " " + std::to_string(rd));
            }
        }
        else {
            switch(bl->op) {
                case TOKEN_EQUAL: return l == r ? "True" : "False";
                default:
                    lex.error(current(), "Cannot Compare " + std::visit([](auto&& v) -> std::string {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, std::string>) return v;
                        else return std::to_string(v);
                    }, l) + " " + std::visit([](auto&& v) -> std::string {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, std::string>) return v;
                        else return std::to_string(v);
                    }, r));
            }
        }
    }
    else if (auto ld = dynamic_cast<ImportNode*>(node)) {
        // Import each specified file: resolve path relative to current file,
        // prevent duplicate/circular imports via importedFiles set, parse and interpret.
        for (const std::string& rawPath : ld->files) {
            if (rawPath.empty())
                continue;

            std::filesystem::path p(rawPath);
            if (p.is_relative() && !fname.empty()) {
                p = std::filesystem::path(fname).parent_path() / p;
            }

            if (!p.has_extension())
                p += ".obt";

            if (!std::filesystem::exists(p)) {
                lex.error(current(), "Import file not found: " + p.string());
                return 0L;
            }

            std::filesystem::path canonical;
            try {
                canonical = std::filesystem::canonical(p);
            } catch(...) {
                canonical = std::filesystem::absolute(p);
            }

            std::string canonicalStr = canonical.string();
            if (importedFiles.find(canonicalStr) != importedFiles.end()) {
                // already imported
                continue;
            }

            importedFiles.insert(canonicalStr);

            std::ifstream file(canonical);
            if (!file.is_open()) {
                lex.error(current(), "Unable to open import file: " + canonical.string());
                return 0L;
            }

            std::stringstream buf;
            buf << file.rdbuf();
            std::string content = buf.str();
            file.close();

            // Parse and interpret the imported file in its own Parser instance
            Parser importer(content, canonicalStr);
            importer.Interpret();
        }
        return 0L;
    }
    else {
        lex.error(tokens[idx], "Syntax Error.");
        return 0L;
    }
    return 0L;
}
