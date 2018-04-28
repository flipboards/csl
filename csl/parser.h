
#ifndef CSL_PARSER_H
#define CSL_PARSER_H

#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "value.h"

#include <string>
#include <map>
#include <unordered_set>

// Parser using recursive descent algorithm
class RDParser {
public:

    RDParser() {

    }

    ~RDParser() {

    }

    void clear() {
        cur_token = Token(Token::NONE);
        next_token = Token(Token::NONE);
        next_look_token = Token(Token::NONE);
        _lexer.clear();
    }

    ASTBase* parse_file(const std::string& filename);

    ASTBase* parse_string(const std::string& str);

    const Lexer& get_lexer()const {
        return _lexer;
    }

private:
    
    ExprAST* parse_simple_expr();

    ExprAST* parse_unary_expr();

    ExprAST* parse_expr();	

    TypeAST* parse_type_base(StringRef);

    TypeAST* parse_type();

    std::vector<VarDeclAST*> parse_var_decl();

    ExprAST* parse_initializer();

    StmtAST* parse_stmt();

    BlockStmtAST* parse_block_stmt();

    FunctionAST* parse_function_decl();

    ClassAST* parse_class_decl();

    Constant* parse_value(const RawValue&);


    void eat();

    bool match(Token::TokenType);

    bool match(Token::TokenType, bool(*unary_cond)(const Token&));

    bool try_match(Token::TokenType)const;

    bool match_op(OpName);

    bool try_match_op(OpName);

    bool match_keyword(Keyword);

    bool try_match_keyword(Keyword);

    void match_required_symbol(OpName, char);

    static std::map<std::string, ASTBase*> ast_cache; // global cache for import different files
    static std::unordered_set<StringRef> typename_cache; // defined classes

    Token cur_token, next_token, next_look_token;

    Lexer _lexer;

};

#endif // !CSL_PARSER_H
