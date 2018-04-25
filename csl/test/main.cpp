#pragma once

#include "../lexer.h"
#include "../parser.h"
#include "../util/errors.h"
#include "../logger.h"

#include <cstdio>
#include <iostream>

int main() {
    
    std::string input("1*(2+)^(5 and 6 or not a)");

    RDParser parser;
    ASTBase* result;
    try {
        result = parser.parse_string(input);

    }
    catch (const SyntaxError& s) {
        output_error(std::cerr, s, parser.get_lexer().get_reader());
        return 0;
    }

    result->print(std::cout);
    delete result;
    return 0;
}