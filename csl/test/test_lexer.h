#include "../lexer.h"

class LexerTest {
public:

    void test_token() {

        ConstStringPool pool;
        std::string a = "Hello!";
        std::string val = "123";

        Keyword kwd = Keyword::BREAK;
        OpName op = OpName::ADD;

        auto strref = pool.assign(a);
        auto valstrref = pool.assign(val);

        Token token_id(Token::ID, strref);
        Token token_val(Token::VALUE);
        token_val.set_value(RawValue::INT, valstrref);
        Token token_kwd(Token::KEYWORD, static_cast<unsigned>(kwd));
        Token token_op(Token::OP, static_cast<unsigned>(op));

        assert(token_id.get_name() == a);
        assert(token_val.get_value().strval == val);
        assert(token_kwd.get_keyword() == kwd);
        assert(token_op.get_operator() == op);
    }

    void test_lexer() {

        ConstStringPool pool;
        StrReader reader("x_2  \t2e-7+-=++'\t'\"1+x\\\"\"  ");

        Lexer lexer;
        lexer.load(&reader, &pool);
        
        std::vector<Token> token_list;
        for (int i = 0; i < 8; i++) {
            token_list.push_back(lexer.get_token());
        }

        assert(token_list[0].get_name() == "x_2");
        auto rv = token_list[1].get_value();
        assert(rv.type == RawValue::FLOAT && rv.strval == "2e-7");
        assert(token_list[2].get_operator() == OpName::ADD);
        assert(token_list[3].get_operator() == OpName::SUBASN);
        assert(token_list[4].get_operator() == OpName::INC);
        printf("%s", token_list[6].get_value().strval.get());
    //    assert(token_list[5].get_value().type == RawValue::STRING && token_list[5].get_value().strval == "\t");
        assert(token_list[6].get_value().type == RawValue::STRING && token_list[6].get_value().strval == "1+x\\\"");
        assert(token_list[7].get_type() == Token::EOF);
    }

};