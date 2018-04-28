
#include "parser.h"
#include "astbuilder.h"
#include "operator.h"

#include "util/errors.h"


std::map<std::string, ASTBase*> RDParser::ast_cache;
std::unordered_set<StringRef> RDParser::typename_cache = { "void", "bool", "char", "int", "float" };

ASTBase* RDParser::parse_string(const std::string& str) {

    StrReader* reader = new StrReader(str);
    _lexer.load(reader);
    eat();
    auto parse_result = parse_expr();
    delete reader;
    return parse_result;
}


ExprAST* RDParser::parse_unary_expr() {

    ExprASTBuilder builder_prefix, builder_postfix;

    while (1) {
        if (match_op(OpName::INC) || match_op(OpName::DEC) || match_op(OpName::ADDR)) {
            builder_prefix.extend_child(new OpAST(static_cast<Operator>(cur_token.get_operator())));
        }
        else if (match_op(OpName::ADD)) {
            builder_prefix.extend_child(new OpAST(Operator::PLUS));
        }
        else if (match_op(OpName::SUB)) {
            builder_prefix.extend_child(new OpAST(Operator::MINUS));
        }
        else if (match_op(OpName::NOT)) {
            builder_prefix.extend_child(new OpAST(Operator::NOT));
        }
        else if (match_op(OpName::MUL)) {
            builder_prefix.extend_child(new OpAST(Operator::DEREF));
        }
        else {
            break;
        }
    }

    if (match(Token::ID)) {
        StringRef name = cur_token.get_name();
        builder_postfix.extend_child(static_cast<ExprAST*>(new IdAST(name)));
    }
    else if (match(Token::VALUE)) {
        Constant* c = parse_value(cur_token.get_value());
        builder_postfix.extend_child(new ValueAST(c));
    }
    else if (match_op(OpName::BRAC)) {
        builder_postfix.extend_child(parse_expr());
        match_required_symbol(OpName::RBRAC, ')');
    }
    else {
        throw SyntaxError("Token with id/value required");
    }

    while (1) {
        if (match_op(OpName::INDEX)) {
            builder_postfix.extend_parent(new OpAST(Operator::INDEX));
            builder_postfix.add_child(parse_expr());
            match_required_symbol(OpName::RINDEX, ']');
        }
        else if (match_op(OpName::BRAC)) {
            if (!builder_postfix.get_ast()->is_id()) {
                throw SyntaxError("Requires an identifier");
            }

            builder_postfix.extend_parent(new CallAST());

            if (!match_op(OpName::RBRAC)) {
                while (1) {
                    builder_postfix.add_child(parse_expr());
                    if (!match_op(OpName::COMMA)) {
                        break;
                    }
                }
                if (!match_op(OpName::RBRAC)) {
                    throw SyntaxError("')' required");
                }
            }

        }
        else if (match_op(OpName::MBER) || match_op(OpName::ARROW)) {
            builder_postfix.extend_parent(new OpAST(static_cast<Operator>(cur_token.get_operator())));
            if (match(Token::ID)) {
                builder_postfix.add_child(static_cast<ExprAST*>(new IdAST(cur_token.get_name())));
            }
            else {
                throw SyntaxError("Member name required");
            }
        }
        else if (match_op(OpName::INC)) {
            builder_postfix.extend_parent(new OpAST(Operator::POSTINC));
        }
        else if (match_op(OpName::DEC)) {
            builder_postfix.extend_parent(new OpAST(Operator::POSTDEC));
        }
        else {
            break;
        }
    }

    builder_prefix.extend_child(builder_postfix.get_ast());

    return builder_prefix.get_ast();

}


ExprAST* RDParser::parse_simple_expr() {

    std::vector<Operator> op_stack = { Operator::NONE };
    std::vector<ExprAST*> value_stack;

    while (1) {
        value_stack.push_back(parse_unary_expr());
        if (!try_match(Token::OP)) {
            break;
        }

        OpName opname = next_token.get_operator();
        if (opname == OpName::RBRAC || opname == OpName::RINDEX) break;
        
        Operator op = static_cast<Operator>(opname);

        if (is_assignment(op)) break;

        match(Token::OP);

        if (!is_arithmetic(op) && !is_logic(op)) throw SyntaxError("Arithmetic operator");

        unsigned pred = get_precedence(op);

        if (pred < get_precedence(op_stack.back())) {
            op_stack.push_back(op);
        }
        else {
            while (pred >= get_precedence(op_stack.back())) {
                auto rval = value_stack.back();
                value_stack.pop_back();
                auto lval = value_stack.back();
                value_stack.pop_back();

                value_stack.push_back(new OpAST(op_stack.back(), lval, rval));
                op_stack.pop_back();
            }
            op_stack.push_back(op);
        }
    }

    while (op_stack.size() > 1) {

        auto rval = value_stack.back();
        value_stack.pop_back();
        auto lval = value_stack.back();
        value_stack.pop_back();

        value_stack.push_back(new OpAST(op_stack.back(), lval, rval));
        op_stack.pop_back();
    }

    auto ret = value_stack.back();
    value_stack.pop_back();
    return ret;
}


ExprAST* RDParser::parse_expr() {

    if (match_op(OpName::SEMICOLON)) {
        return nullptr;
    }

    ExprAST* ast_lhs = parse_simple_expr(), *ast_ret;

    if (try_match(Token::OP)) {
        Operator op = static_cast<Operator>(next_token.get_operator());
        if (is_assignment(op)) {
            eat();
            ast_ret = new OpAST(op, ast_lhs, parse_expr());
        }
        else {
            ast_ret = ast_lhs;
        }
    }
    else {
        ast_ret = ast_lhs;
    }

    return ast_ret;

}

TypeAST * RDParser::parse_type_base(StringRef name) {
    
    const std::map<StringRef, Type::TypeID> typeloc = {
        {"void", Type::VOID}, {"bool", Type::BOOL}, {"char", Type::CHAR},
        {"int", Type::INT}, {"float", Type::FLOAT}
    };

    auto find_result = typeloc.find(name);
    if (find_result == typeloc.end()) {
        return new TypeAST(name);
    }
    else {
        return new TypeAST(new PrimitiveType(find_result->second));
    }
}

TypeAST * RDParser::parse_type() {
    TypeAST* vartype;

    if (match(Token::ID)) {
        if (typename_cache.find(cur_token.get_name()) == typename_cache.end()) {
            throw SyntaxError("Type undefined: " + cur_token.get_name());
        }
        vartype = parse_type_base(cur_token.get_name());
    }
    else {
        throw SyntaxError("Type name required");
    }

    while (1) {
        // pointer
        if (match_op(OpName::MUL)) {
            vartype = new TypeAST(vartype);

        }
        else if (match_op(OpName::INDEX)) {
            ExprAST* idx_ast = nullptr;
            if (!match_op(OpName::RINDEX)) {
                idx_ast = parse_expr();
                match_required_symbol(OpName::RINDEX, ']');
            }
            vartype = new ArrayTypeAST(vartype, idx_ast);
        }
        else {
            break;
        }
    }

    return vartype;
}

std::vector<VarDeclAST*> RDParser::parse_var_decl() {

    TypeAST* vartype;

    if (match(Token::ID)) {
        if (typename_cache.find(cur_token.get_name()) == typename_cache.end()) {
            throw SyntaxError("Type undefined: " + cur_token.get_name());
        }
        vartype = parse_type_base(cur_token.get_name());
    }
    else {
        throw SyntaxError("Type name required");
    }

    std::vector<VarDeclAST*> decl_ast_list;

    while (1) {

        TypeAST* cur_vartype = vartype;
        StringRef cur_varname;
        VarDeclAST* decl_ast;

        while (1) {
            // pointer
            if (match_op(OpName::MUL)) {
                cur_vartype = new TypeAST(cur_vartype);
                
            }
            else if (match_op(OpName::INDEX)) {
                ExprAST* idx_ast = nullptr;
                if (!match_op(OpName::RINDEX)) {
                    idx_ast = parse_expr();
                    match_required_symbol(OpName::RINDEX, ']');
                }
                cur_vartype = new ArrayTypeAST(cur_vartype, idx_ast);
            }
            else {
                break;
            }
        }

        if (match(Token::ID)) {
            cur_varname = cur_token.get_name();
        }
        else {
            throw SyntaxError("Identifier required for declaration");
        }

        if (match_op(OpName::COMMA)) {
            decl_ast_list.push_back(new VarDeclAST(cur_vartype, cur_varname));
            continue;
        }
        else if (match_op(OpName::ASN)) {
            ExprAST* initializer = parse_initializer();
            decl_ast_list.push_back(new VarDeclAST(cur_vartype, cur_varname, initializer));
        }
        else {
            break;
        }
    }

    return decl_ast_list;

}

ExprAST * RDParser::parse_initializer()
{
    ExprAST* initializer;
    if (match_op(OpName::COMP)) {
        initializer = new ListAST();
        while (1) {
            initializer->add_child(parse_initializer());
            if (!match_op(OpName::COMMA)) {
                break;
            }
        }
        match_required_symbol(OpName::RCOMP, '}');
    }
    else {
        initializer = parse_expr();
    }
    return initializer;
}

StmtAST* RDParser::parse_stmt(){
    
    if (try_match_op(OpName::COMP)) {
        return parse_block_stmt();
    }

    else if (match_keyword(Keyword::IF)) {
        
        ExprAST *expr_cond;
        StmtAST *ast1, *ast2 = nullptr;

        match_required_symbol(OpName::BRAC, '(');
        expr_cond = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');

        ast1 = parse_stmt();

        if (match_keyword(Keyword::ELSE)) {
            ast2 = parse_stmt();
        }
        return new IfAST(expr_cond, ast1, ast2);
    }

    else if (match_keyword(Keyword::WHILE)) {

        ExprAST *expr_cond;
        StmtAST *ast1;

        match_required_symbol(OpName::BRAC, '(');
        expr_cond = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');

        return new WhileAST(expr_cond, parse_stmt());
    }

    else if (match_keyword(Keyword::FOR)) {

        ExprAST *expr_init, *expr_cond, *expr_loop;

        match_required_symbol(OpName::BRAC, '(');
        expr_init = parse_expr();
        match_required_symbol(OpName::SEMICOLON, ';');
        expr_cond = parse_expr();
        match_required_symbol(OpName::SEMICOLON, ';');
        expr_loop = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');

        return new ForAST(expr_init, expr_cond, expr_loop, parse_stmt());
    }

    else if (match_keyword(Keyword::BREAK)) {
        return new BreakAST();
    }
    else if (match_keyword(Keyword::CONTINUE)) {
        return new ContinueAST();
    }
    else if (match_keyword(Keyword::RETURN)) {
        return new ReturnAST(parse_expr());
    }
    else {
        return parse_expr();
    }

}

BlockStmtAST * RDParser::parse_block_stmt() {
    match_required_symbol(OpName::COMP, '{');

    BlockStmtAST* ast = new BlockStmtAST();

    while (1) {
        if (match_op(OpName::RCOMP)) {
            break;
        }
        else if (match_op(OpName::SEMICOLON)) {
            continue;
        }
        else if (match(Token::EOF)) {
            throw SyntaxError("Reach end of file");
        }
        else if (try_match(Token::ID)) {
            auto find_result = typename_cache.find(next_token.get_name());
            if (find_result == typename_cache.end()) {
                for (const auto& i : parse_var_decl()) {
                    ast->append(i);
                }
            }
            else {
                auto expr_ast = parse_expr();
                if (expr_ast) {
                    ast->append(expr_ast);
                }
            }
        }
        else {
            auto expr_ast = parse_expr();
            if (expr_ast) {
                ast->append(expr_ast);
            }
        }
    }

    return ast;
}

FunctionAST * RDParser::parse_function_decl() {

    if (!match_keyword(Keyword::FN)) {
        throw SyntaxError("Requires 'fn' for function declaration");
    }

    StringRef fname;
    if (match(Token::ID)) {
        fname = next_token.get_name();
    }
    else {
        throw SyntaxError("Expect an identifier");
    }

    match_required_symbol(OpName::BRAC, '(');
    FunctionAST* func = new FunctionAST(fname);

    while (1) {
        if (match(Token::ID)) { // id:(type)
            StringRef arg_name = cur_token.get_name();
            TypeAST* arg_type;
            if (match_op(OpName::COLON)) {
                arg_type = parse_type();
            }
            else {
                arg_type = new TypeAST(new Type(Type::VOID));
            }
            func->add_argument(arg_type, arg_name);
            if (match_op(OpName::RBRAC)) {
                break;
            }
            match_required_symbol(OpName::COMMA, ',');
        }
        else if (match_op(OpName::COLON)) { // :(type)
            TypeAST* arg_type;
            if (match_op(OpName::COLON)) {
                arg_type = parse_type();
            }
            else {
                arg_type = new TypeAST(new Type(Type::VOID));
            }
            func->add_argument(arg_type);
            if (match_op(OpName::RBRAC)) {
                break;
            }
            match_required_symbol(OpName::COMMA, ',');
        }
        else if (match_op(OpName::RBRAC)) {
            break;
        }
        else {
            throw SyntaxError("Expected an id");
        }
    }
    match_required_symbol(OpName::RBRAC, ')');

    if (match_op(OpName::ARROW)) {
        TypeAST* ret_type = parse_type();
        func->set_return_type(ret_type);
    }
    else {
        func->set_return_type(new TypeAST(new Type(Type::VOID)));
    }

    if (try_match_op(OpName::COMP)) {
        func->set_body_ast(parse_block_stmt());
    }
    else {
        match_required_symbol(OpName::SEMICOLON, ';');
    }
    
    return func;
}


ClassAST * RDParser::parse_class_decl() {
    
    if (!match_keyword(Keyword::CLASS)) {
        throw SyntaxError("Invalid definition");
    }

    if (!match(Token::ID)) {
        throw SyntaxError("Requires an identifier");
    }
    StringRef name = cur_token.get_name();
    ClassAST* new_class = new ClassAST(name);

    if (match_op(OpName::COMP)) {

    }
    else if (match_op(OpName::SEMICOLON)) {
        return new_class;
    }
    else {
        throw SyntaxError("Invalid class definition");
    }

    if (typename_cache.find(name) != typename_cache.end()) {
        throw SyntaxError("Class has already defined: " + name);
    }

    while (1) {
        if (match_op(OpName::RCOMP)) {
            break;
        }
        else if (try_match(Token::ID)) {
            for (const auto& i : parse_var_decl()) {
                new_class->add_member(i);
            }
        }
        else if (try_match_keyword(Keyword::FN)) {
            new_class->add_method(parse_function_decl());
        }
        else {
            throw SyntaxError("Require a declaration");
        }
    }

    // add into symbol table
    typename_cache.insert(name);
}


Constant* RDParser::parse_value(const RawValue& rawval) {

    if (rawval.type != RawValue::STRING) {
        const char* vstr = rawval.strval.c_str();
        char* end;

        if (rawval.type == RawValue::BOOL) {
            char val = rawval.strval == "true" ? 0 : 1;
            return new Constant(new PrimitiveType(Type::BOOL), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::CHAR) {
            char val = rawval.strval[0];
            return new Constant(new PrimitiveType(Type::INT), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::INT) {
            unsigned val = strtol(vstr, &end, 10);
            return new Constant(new PrimitiveType(Type::INT), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::FLOAT) {
            double val = strtod(vstr, &end);
            return new Constant(new PrimitiveType(Type::FLOAT), (char*)&val, sizeof(val));
        }
        else { //won't actually happen
            return nullptr;
        }
    }
    else {
        return new Constant(
            static_cast<Type*>(new PointerType(new PrimitiveType(Type::CHAR))),
            rawval.strval.c_str(), rawval.strval.length());
    }

}

void RDParser::eat() {
    cur_token = next_token;
    next_token = _lexer.get_token();
}

bool RDParser::match(Token::TokenType type) {
    if (next_token.is_type(type)) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::match(Token::TokenType type, bool(*unary_cond)(const Token &))
{
    if (next_token.is_type(type) && unary_cond(next_token)) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::try_match(Token::TokenType type)const {
    return next_token.is_type(type);
}

bool RDParser::match_op(OpName opname) {
    if (next_token.is_type(Token::OP) && next_token.get_operator() == opname) {
        eat();
        return true;
    }
    else {
        return false;
    }
}


bool RDParser::match_keyword(Keyword keyword) {
    if (next_token.is_type(Token::KEYWORD) && next_token.get_keyword() == keyword) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::try_match_keyword(Keyword keyword) {
    if (next_token.is_type(Token::KEYWORD) && next_token.get_keyword() == keyword) {
        return true;
    }
    else {
        return false;
    }
}

void RDParser::match_required_symbol(OpName opname, char symbol)
{
    if (!match_op(opname)) {
        std::string err_string = "Symbol ' ' required";
        err_string[8] = symbol;
        throw SyntaxError(err_string);
    }
}

bool RDParser::try_match_op(OpName opname)
{
    if (next_token.is_type(Token::OP) && next_token.get_operator() == opname) {
        return true;
    }
    else {
        return false;
    }
}
