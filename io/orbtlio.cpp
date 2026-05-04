/*
 * OS Intraction Layer  Copyright (C) 2026  First Person
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <iostream>
#include <charconv>
#include <memory>
#include <variant>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <orbtlio.h>
#include <Parser.h>

namespace fs = std::filesystem;
bool isInitialized = false;
const std::map<std::string, std::unique_ptr<FunctionNode>> builtin = [] {
    std::map<std::string, std::unique_ptr<FunctionNode>> m;
    m["print"] = std::make_unique<FunctionNode>("print", std::vector<std::string>{"text"});
	m["exit"] = std::make_unique<FunctionNode>("exit", std::vector<std::string>{"code"});
	m["input"] = std::make_unique<FunctionNode>("input", std::vector<std::string>{"message"});
    m["read"] = std::make_unique<FunctionNode>("read", std::vector<std::string>{"filename"});
    m["write"] = std::make_unique<FunctionNode>("write", std::vector<std::string>{"filename", "contents"});
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

std::variant<double, long, int, std::string> vinput() {
    std::string s;
    std::cin >> s;

    // int
    {
        int v;
        auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
        if (ec == std::errc() && p == s.data() + s.size())
            return v;
    }

    // long
    {
        long v;
        auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
        if (ec == std::errc() && p == s.data() + s.size())
            return v;
    }

    // double
    {
        char* end;
        double v = std::strtod(s.c_str(), &end);
        if (end != s.c_str() && *end == '\0')
            return v;
    }

    return s;
}

std::variant<double, long, int, std::string> exec(CallNode* cn) {
	if (cn->name == "print") {
		std::cout << variableTable.find("text")->second.value << std::endl;
		return 0L;
	}
	else if(cn->name == "exit") {
		std::exit(std::stoi(variableTable.find("code")->second.value));
		return 0L;
	}
	else if(cn->name == "input") {
		std::cout << variableTable.find("message")->second.value;
		return vinput();
	}
    else if(cn->name == "read") {
        if(variableTable.find("filename")->second.value == "")
            throw std::runtime_error("Filename Should Be Not Empty.");
        
        if(!fs::exists(variableTable.find("filename")->second.value))
            throw std::runtime_error("File Is Not Exist");
        
        std::ifstream file(variableTable.find("filename")->second.value);
        if(!file.is_open())
            throw std::runtime_error("Access Is Deniend.");

        std::string line;
        std::string s;
        while (std::getline(file, line))
            s += line + "\n";
        file.close();
        
        {
            int v;
            auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
            if (ec == std::errc() && p == s.data() + s.size())
                return v;
        }

        // long
        {
            long v;
            auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), v);
            if (ec == std::errc() && p == s.data() + s.size())
                return v;
        }

        // double
        {
            char* end;
            double v = std::strtod(s.c_str(), &end);
            if (end != s.c_str() && *end == '\0')
                return v;
        }

        return s;
    }
    else if(cn->name == "write") {
        if(variableTable.find("filename")->second.value == "")
            throw std::runtime_error("Filename Should Be Not Empty.");
        
        std::ofstream file(variableTable.find("filename")->second.value);
        if(!file.is_open())
            throw std::runtime_error("Unable To Create File.");
        else {
            file << variableTable.find("contents")->second.value;
        }
        return 0L;
    }
}