/*
 * OS Intraction Layer  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <cstdbool>
#include <map>
#include <memory>
#include <orbtlio.h>
#include <Parser.h>

bool isInitialized = false;
extern std::map<std::string, VariableInfo> variableTable;
extern std::map<std::string, FunctionNode*> functionTable;
const std::map<std::string, std::unique_ptr<FunctionNode>> builtin = [] {
    std::map<std::string, std::unique_ptr<FunctionNode>> m;
    m["print"] = std::make_unique<FunctionNode>("print", std::vector<std::string>{"text"});
    return m;
}();


bool isBuiltin(std::string function) {
	if(!isInitialized)
		init(); // Initialize If Not Initialized

	return builtin.find(function) != builtin.end();
}

void init() {
	for (auto const& [name, node] : builtin) {
		functionTable.emplace(name, node.get());
	}

	isInitialized = true;
}

void exec(CallNode* cn) {
	if (cn->name == "print") {
		std::cout << variableTable.find("text")->second.value << std::endl;
	}
}