#pragma once

#include <orbtlio.h>
#include <string>
#include <iostream>
#include <variant>
#include <filesystem>
#include <Parser.h>

class OMath : public OBTModule {
public:
    void init() override;
    std::variant<double, long, int, std::string> exec(CallNode* cn) override;
};