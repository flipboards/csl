
#ifndef CSL_PARSER_H
#define CSL_PARSER_H

#include "ast.h"
#include "token.h"
#include "lexer.h"

#include <string>
#include <map>

// Parser using recursive descent algorithm
class RDParser {
public:

    RDParser() {

    }

    ~RDParser() {

    }

    void clear() {
        cur_token = Token(Token::Type::NONE);
        next_token = Token(Token::Type::NONE);
        next_look_token = Token(Token::Type::NONE);
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

    std::vector<VarDeclAST*> parse_var_decl();

    ExprAST* parse_initializer();

    StmtAST* parse_stmt();

    BlockStmtAST* parse_block_stmt();

    FunctionAST* parse_function_decl();

    ValueAST* build_value_ast(const RawValue&);

    unsigned eval_const_expr(const ExprAST*);

    void eat();

    bool match(Token::Type);

    bool match(Token::Type, bool *f(const Token&));

    bool try_match(Token::Type)const;

    bool match_op(OpName);

    bool match_sep(OpName);

    bool try_match_sep(OpName);


    // global cache for import different files
    static std::map<StringRef, ASTBase*> ast_cache;

    std::map<StringRef, ClassType*> type_cache; // cache for self-defined classes
    

    Token cur_token, next_token, next_look_token;

    Lexer _lexer;

};

#endif // !CSL_PARSER_H
