/*
 * OS Intraction Layer  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <orbtlio.h>
#include <Parser.h>
#include <cstdbool>
#include <map>

bool isInitialized = false;
extern std::map<std::string, VariableInfo> variableTable;
extern std::map<std::string, FunctionNode*> functionTable;

/* bool isBuiltin(std::string function) {
	return BuiltinFunctions.find(function) != BuiltinFunctions.end();
} */

void init() {
	functionTable.insert({ "print", new FunctionNode("print", { "text" })});
}

void exec(CallNode* cn) {
	if (cn->name == "print") {
		std::cout << variableTable.find("text")->second.value << std::endl;
	}
}