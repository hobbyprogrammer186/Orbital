/*
 * OS Intraction Layer  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#pragma once
#include <Parser.h>
#include <iostream>
#include <variant>
#include <optional>

class OBTModule {
public:
    virtual ~OBTModule() = default;
    virtual void init() = 0;
    virtual std::variant<double, long, int, std::string> exec(CallNode* cn) = 0;
};

extern void init();
extern std::variant<double, long, int, std::string> exec(CallNode* cn);
extern bool isBuiltin(std::string function);
extern void importBuiltin(const std::string& module);
extern void pushFunc(std::string name, std::optional<std::vector<std::string>> args, OBTModule* module = nullptr);