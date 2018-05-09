#pragma once

#include "test_parser.h"

int main() {
    
    ParserTest test;
    test.test_parse_expr();
    test.test_parse_decl();

    return 0;
}