
#ifndef CSL_PARSER_H
#define CSL_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "value.h"
#include "util/memory.h"
#include "util/strmap.h"


namespace csl{

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
    
    void load_context(Context* context) {
        this->_context = context;
    }

    ASTRef parse_file(const std::string& filename);

    ExprASTRef parse_line_expr(const std::string& str);

    BlockStmtASTRef parse_string(const std::string& str);

    const Lexer& get_lexer()const {
        return _lexer;
    }

private:
    
    ExprASTRef parse_simple_expr();

    ExprASTRef parse_unary_expr();

    ExprASTRef parse_expr();	

    TypeASTRef parse_type_base(const StringRef&);

    TypeASTRef parse_type();

    std::vector<VarDeclASTRef> parse_var_decl();

    ExprASTRef parse_initializer();

    StmtASTRef parse_stmt();

    BlockStmtASTRef parse_block_stmt(bool implicit_bracket=false);

    FunctionASTRef parse_function_decl();

    ClassASTRef parse_class_decl();

    ConstantRef parse_value(const RawValue&);


    void eat();

    bool match(Token::TokenType);

    bool match(Token::TokenType, bool(*unary_cond)(const Token&));

    bool try_match(Token::TokenType)const;

    bool match_op(OpName);

    bool try_match_op(OpName);

    bool match_keyword(Keyword);

    bool try_match_keyword(Keyword);

    void match_required_symbol(OpName, char);

    template<typename Ty>
    MemoryRef<Ty> store_ast_unconst(Ty* ptr) {
        return _context->astpool.collect<Ty>(ptr);
    }

    template<typename Ty>
    ConstMemoryRef<Ty> store_ast(Ty* ptr) {
        return _context->astpool.collect<Ty>(ptr).to_const();
    }

    TypeRef store_type(Type* ptr) {
        return _context->typepool.collect<Type>(ptr).to_const();
    }

    /* StringRef is only used for intermediate representation; For searching, std::string is used */

    static std::map<std::string, ASTRef> ast_cache;     // global cache for import different files
    static StrSet typename_cache;                       // defined classes

    Token cur_token, next_token, next_look_token;
    Context* _context;

    Lexer _lexer;

};

}

#endif // !CSL_PARSER_H
