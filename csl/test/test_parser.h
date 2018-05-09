
#include "../parser.h"
#include <iostream>

class ParserTest {
public:

    void test_parse_expr() {

        RDParser parser;
        Context context;

        parser.load_context(&context);

        parser.parse_line_expr("1 + 2")->print(std::cout);     // test basic
        parser.parse_line_expr("x=y=++a+++=4==5")->print(std::cout);      // unary & assignment
        parser.parse_line_expr("1+3^x*(3 and 4 or 5)")->print(std::cout); // test priority
    }

    void test_parse_decl() {
        RDParser parser;
        Context context;

        parser.load_context(&context);

        parser.parse_string("int a;")->print(std::cout);
        parser.parse_string("int* a, b=1+2, c=a;")->print(std::cout);
        parser.parse_string("int[10] c;")->print(std::cout);
        parser.parse_string("void[6+a] d")->print(std::cout);
        parser.parse_string("int[10] d = {1,2,{2,3}}")->print(std::cout);
    }
};