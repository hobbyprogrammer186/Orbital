#ifdef _WIN32
    #include <curses.h>
#else
    #include <ncurses.h>
#endif

#include <orbtlio.h>
#include <string>
#include <iostream>
#include <variant>
#include <filesystem>
#include <orbt_math.h>
#include <Parser.h>
#include <cmath>

void OMath::init() {
    pushFunc("sqrt", std::vector<std::string>{"x"}, this);
    pushFunc("cbrt", std::vector<std::string>{"x"}, this);
    pushFunc("pow", std::vector<std::string>{"x", "y"}, this);

    pushFunc("celi", std::vector<std::string>{"x"}, this);
    pushFunc("floor", std::vector<std::string>{"x"}, this);
    pushFunc("round", std::vector<std::string>{"x"}, this);

    pushFunc("abs", std::vector<std::string>{"x"}, this);
    pushFunc("fabs", std::vector<std::string>{"x"}, this);

    pushFunc("sin", std::vector<std::string>{"x"}, this);
    pushFunc("cos", std::vector<std::string>{"x"}, this);
    pushFunc("tan", std::vector<std::string>{"x"}, this);

    pushFunc("asin", std::vector<std::string>{"x"}, this);
    pushFunc("acos", std::vector<std::string>{"x"}, this);
    pushFunc("atan", std::vector<std::string>{"x"}, this);

    pushFunc("log1", std::vector<std::string>{"x"}, this);
    pushFunc("log2", std::vector<std::string>{"x"}, this);
    pushFunc("log10", std::vector<std::string>{"x"}, this);

    pushFunc("exp", std::vector<std::string>{"x"}, this);

    pushFunc("fmod", std::vector<std::string>{"x", "y"}, this);
    pushFunc("remainder", std::vector<std::string>{"x", "y"}, this);
}

static double getDouble(const std::string& name) {
    auto it = variableTable.find(name);
    if (it == variableTable.end()) return 0.0;
    try {
        return std::stod(it->second.value);
    } catch(...) {
        return 0.0;
    }
}

std::variant<double, long, int, std::string> OMath::exec(CallNode* cn) {
    if (cn == nullptr)
        return 0L;

    if (cn->name == "sqrt") {
        double x = getDouble("x");
        return std::sqrt(x);
    } else if (cn->name == "cbrt") {
        double x = getDouble("x");
        return std::cbrt(x);
    } else if (cn->name == "pow") {
        double x = getDouble("x");
        double y = getDouble("y");
        return std::pow(x, y);
    } else if (cn->name == "celi") {
        double x = getDouble("x");
        return std::ceil(x);
    } else if (cn->name == "floor") {
        double x = getDouble("x");
        return std::floor(x);
    } else if (cn->name == "round") {
        double x = getDouble("x");
        return std::round(x);
    } else if (cn->name == "abs") {
        double x = getDouble("x");
        return std::abs(x);
    } else if (cn->name == "fabs") {
        double x = getDouble("x");
        return std::fabs(x);
    } else if (cn->name == "sin") {
        double x = getDouble("x");
        return std::sin(x);
    } else if (cn->name == "cos") {
        double x = getDouble("x");
        return std::cos(x);
    } else if (cn->name == "tan") {
        double x = getDouble("x");
        return std::tan(x);
    } else if (cn->name == "asin") {
        double x = getDouble("x");
        return std::asin(x);
    } else if (cn->name == "acos") {
        double x = getDouble("x");
        return std::acos(x);
    } else if (cn->name == "atan") {
        double x = getDouble("x");
        return std::atan(x);
    } else if (cn->name == "log1") {
        double x = getDouble("x");
        return std::log(x);
    } else if (cn->name == "log2") {
        double x = getDouble("x");
        return std::log2(x);
    } else if (cn->name == "log10") {
        double x = getDouble("x");
        return std::log10(x);
    } else if (cn->name == "exp") {
        double x = getDouble("x");
        return std::exp(x);
    } else if (cn->name == "fmod") {
        double x = getDouble("x");
        double y = getDouble("y");
        return std::fmod(x, y);
    } else if (cn->name == "remainder") {
        double x = getDouble("x");
        double y = getDouble("y");
        return std::remainder(x, y);
    }

    return 0L;
}