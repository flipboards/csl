
#include "parser.h"
#include "operator.h"

#include "util/errors.h"


std::map<std::string, ASTRef> RDParser::ast_cache;
StrSet RDParser::typename_cache = { "void", "bool", "char", "int", "float" };

ExprASTRef RDParser::parse_line_expr(const std::string& str) {

    StrReader reader(str);
    this->clear();
    _lexer.load(&reader, _context);
    eat();
    return parse_expr();
}

BlockStmtASTRef RDParser::parse_string(const std::string & str) {
    StrReader reader(str);
    this->clear();
    _lexer.load(&reader, _context);
    eat();
    return parse_block_stmt(true);
}


ExprASTRef RDParser::parse_unary_expr() {

    MemoryRef<OpAST> cur_ast_prefix, ast_prefix_ref;

    while (1) {
        OpAST* new_ast;
        if (match_op(OpName::INC) || match_op(OpName::DEC) || match_op(OpName::ADDR)) {
            new_ast = new OpAST(static_cast<Operator>(cur_token.get_operator()));
        }
        else if (match_op(OpName::ADD)) {
            new_ast = new OpAST(Operator::PLUS);
        }
        else if (match_op(OpName::SUB)) {
            new_ast = new OpAST(Operator::MINUS);
        }
        else if (match_op(OpName::NOT)) {
            new_ast = new OpAST(Operator::NOT);
        }
        else if (match_op(OpName::MUL)) {
            new_ast = new OpAST(Operator::DEREF);
        }
        else {
            break;
        }

        MemoryRef<OpAST> new_ast_ref = store_ast_unconst(new_ast);

        if (!ast_prefix_ref) {
            cur_ast_prefix = ast_prefix_ref = new_ast_ref;
        }
        else {
            cur_ast_prefix->add_child(new_ast_ref.to_const().cast<ExprAST>());
            cur_ast_prefix = new_ast_ref;
        }
    }

    ExprASTRef ast_id_ref;

    if (match(Token::ID)) {
        StringRef name = cur_token.get_name();
        ast_id_ref = store_ast<ExprAST>(new IdAST(name));
    }
    else if (match(Token::VALUE)) {
        ConstantRef c = parse_value(cur_token.get_value());
        ast_id_ref = store_ast<ExprAST>(new ValueAST(c));
    }
    else if (match_op(OpName::BRAC)) {
        ast_id_ref = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');
    }
    else {
        throw SyntaxError("Token with id/value required");
    }

    ExprASTRef ast_postfix_ref = ast_id_ref;

    while (1) {
        if (match_op(OpName::INDEX)) {
            MemoryRef<OpAST> op = store_ast_unconst(new OpAST(Operator::INDEX));
            op->add_child(ast_postfix_ref);
            op->add_child(parse_expr());
            ast_postfix_ref = op.cast<ExprAST>();
            match_required_symbol(OpName::RINDEX, ']');
        }
        else if (match_op(OpName::BRAC)) {
            CallAST* call_ast = new CallAST();

            if (ast_postfix_ref->is_id()) {
                throw SyntaxError("Requires an identifier");
            }
            else {
                call_ast->set_callee(ast_postfix_ref.cast<IdAST>());
            }

            if (!match_op(OpName::RBRAC)) {
                while (1) {
                    call_ast->add_arg(parse_expr());
                    if (!match_op(OpName::COMMA)) {
                        break;
                    }
                }
                if (!match_op(OpName::RBRAC)) {
                    throw SyntaxError("')' required");
                }
            }

            ast_postfix_ref = store_ast<ExprAST>(call_ast);
        }
        else if (match_op(OpName::MBER) || match_op(OpName::ARROW)) {
            OpAST* op = new OpAST(static_cast<Operator>(cur_token.get_operator()));
            if (match(Token::ID)) {
                op->add_child(store_ast<ExprAST>(new IdAST(cur_token.get_name())));
            }
            else {
                throw SyntaxError("Member name required");
            }
            ast_postfix_ref = store_ast<ExprAST>(op);
        }
        else if (match_op(OpName::INC)) {
            OpAST* op = new OpAST(Operator::POSTINC, ast_postfix_ref);
            ast_postfix_ref = store_ast<ExprAST>(op);
        }
        else if (match_op(OpName::DEC)) {
            OpAST* op = new OpAST(Operator::POSTDEC, ast_postfix_ref);
            ast_postfix_ref = store_ast<ExprAST>(op);
        }
        else {
            break;
        }
    }

    if (ast_prefix_ref.exists()) {
        cur_ast_prefix->add_child(ast_postfix_ref);
        return ast_prefix_ref.cast<ExprAST>().to_const();
    }
    else {
        return ast_postfix_ref;
    }
}


ExprASTRef RDParser::parse_simple_expr() {

    std::vector<Operator> op_stack = { Operator::NONE };
    std::vector<ExprASTRef> value_stack;

    while (1) {
        value_stack.push_back(parse_unary_expr());
        if (!try_match(Token::OP)) {
            break;
        }

        OpName opname = next_token.get_operator();
        if (opname == OpName::RBRAC || opname == OpName::RINDEX) break;
        
        Operator op = static_cast<Operator>(opname);
        if (!is_valid(op))break;

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

                value_stack.push_back(store_ast<ExprAST>(new OpAST(op_stack.back(), lval, rval)));
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

        value_stack.push_back(store_ast<ExprAST>(new OpAST(op_stack.back(), lval, rval)));
        op_stack.pop_back();
    }

    auto ret = value_stack.back();
    value_stack.pop_back();
    return ret;
}


ExprASTRef RDParser::parse_expr() {

    if (match_op(OpName::SEMICOLON)) {
        return ExprASTRef();
    }

    ExprASTRef ast_lhs = parse_simple_expr();
    ExprASTRef ast_ret;

    if (try_match(Token::OP) && is_valid(static_cast<Operator>(next_token.get_operator()))) {
        Operator op = static_cast<Operator>(next_token.get_operator());
        if (is_assignment(op)) {
            eat();
            ast_ret = store_ast<ExprAST>(new OpAST(op, ast_lhs, parse_expr()));
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

TypeASTRef RDParser::parse_type_base(const StringRef& name) {
    
    const std::map<std::string, Type::TypeID> typeloc = {
        {"void", Type::VOID}, {"bool", Type::BOOL}, {"char", Type::CHAR},
        {"int", Type::INT}, {"float", Type::FLOAT}
    };

    auto find_result = typeloc.find(name.to_string());  /// TODO: Using StringRef directly
    if (find_result == typeloc.end()) {
        return store_ast<TypeAST>(new TypeAST(name));
    }
    else {
        TypeRef type = store_type(new PrimitiveType(find_result->second));
        return store_ast<TypeAST>(new TypeAST(type));
    }
}

TypeASTRef RDParser::parse_type() {
    TypeASTRef vartype;

    if (match(Token::ID)) {
        if (!typename_cache.has_key(cur_token.get_name().to_cstr())) {
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
            vartype = store_ast<TypeAST>(new TypeAST(vartype));

        }
        else if (match_op(OpName::INDEX)) {
            ExprASTRef idx_ast;
            if (!match_op(OpName::RINDEX)) {
                idx_ast = parse_expr();
                match_required_symbol(OpName::RINDEX, ']');
            }
            vartype = store_ast<TypeAST>(new ArrayTypeAST(vartype, idx_ast));
        }
        else {
            break;
        }
    }

    return vartype;
}

std::vector<VarDeclASTRef> RDParser::parse_var_decl() {

    TypeASTRef vartype;

    if (match(Token::ID)) {
        if (!typename_cache.has_key(cur_token.get_name().to_cstr())) {
            throw SyntaxError("Type undefined: " + cur_token.get_name());
        }
        vartype = parse_type_base(cur_token.get_name());
    }
    else {
        throw SyntaxError("Type name required");
    }

    while (1) {
        if (match_op(OpName::MUL)) {
            vartype = store_ast<TypeAST>(new TypeAST(vartype));

        }
        else if (match_op(OpName::INDEX)) {
            ExprASTRef idx_ast;
            if (!match_op(OpName::RINDEX)) {
                idx_ast = parse_expr();
                match_required_symbol(OpName::RINDEX, ']');
            }
            vartype = store_ast<TypeAST>(new ArrayTypeAST(vartype, idx_ast));
        }
        else {
            break;
        }
    }

    std::vector<VarDeclASTRef> decl_ast_list;

    while (1) {

        StringRef cur_varname;
        VarDeclASTRef decl_ast;


        if (match(Token::ID)) {
            cur_varname = cur_token.get_name();
        }
        else {
            throw SyntaxError("Identifier required for declaration");
        }

        ExprASTRef initializer;
        if (match_op(OpName::ASN)) {
            initializer = parse_initializer();
        }
        decl_ast_list.push_back(store_ast<VarDeclAST>(new VarDeclAST(vartype, cur_varname, initializer)));

        if (match_op(OpName::COMMA)) {
            continue;
        }
        else {
            break;
        }

    }

    return decl_ast_list;

}

ExprASTRef RDParser::parse_initializer()
{
    if (match_op(OpName::COMP)) {
        ExprAST* initializer = new ListAST();
        while (1) {
            initializer->add_child(parse_initializer());
            if (!match_op(OpName::COMMA)) {
                break;
            }
        }
        match_required_symbol(OpName::RCOMP, '}');
        return store_ast<ExprAST>(initializer);
    }
    else {
        return parse_expr();
    }
}

StmtASTRef RDParser::parse_stmt(){
    
    if (try_match_op(OpName::COMP)) {
        return parse_block_stmt().cast<StmtAST>();
    }

    else if (match_keyword(Keyword::IF)) {
        
        ExprASTRef expr_cond;
        StmtASTRef ast1, ast2;

        match_required_symbol(OpName::BRAC, '(');
        expr_cond = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');

        ast1 = parse_stmt();

        if (match_keyword(Keyword::ELSE)) {
            ast2 = parse_stmt();
        }
        return store_ast<StmtAST>(new IfAST(expr_cond, ast1, ast2));
    }

    else if (match_keyword(Keyword::WHILE)) {

        ExprASTRef expr_cond;
        StmtASTRef ast1;

        match_required_symbol(OpName::BRAC, '(');
        expr_cond = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');

        return store_ast<StmtAST>(new WhileAST(expr_cond, parse_stmt()));
    }

    else if (match_keyword(Keyword::FOR)) {

        ExprASTRef expr_init, expr_cond, expr_loop;

        match_required_symbol(OpName::BRAC, '(');
        expr_init = parse_expr();
        match_required_symbol(OpName::SEMICOLON, ';');
        expr_cond = parse_expr();
        match_required_symbol(OpName::SEMICOLON, ';');
        expr_loop = parse_expr();
        match_required_symbol(OpName::RBRAC, ')');

        return store_ast<StmtAST>(new ForAST(expr_init, expr_cond, expr_loop, parse_stmt()));
    }

    else if (match_keyword(Keyword::BREAK)) {
        return store_ast<StmtAST>(new BreakAST());
    }
    else if (match_keyword(Keyword::CONTINUE)) {
        return store_ast<StmtAST>(new ContinueAST());
    }
    else if (match_keyword(Keyword::RETURN)) {
        return store_ast<StmtAST>(new ReturnAST(parse_expr()));
    }
    else {
        return parse_expr().cast<StmtAST>();
    }

}

BlockStmtASTRef RDParser::parse_block_stmt(bool implicit_bracket) {
    if (!implicit_bracket) {
        match_required_symbol(OpName::COMP, '{');
    }

    MemoryRef<BlockStmtAST> ast = store_ast_unconst(new BlockStmtAST());

    while (1) {
        if (!implicit_bracket && match_op(OpName::RCOMP)) {
            break;
        }
        else if (match_op(OpName::SEMICOLON)) {
            continue;
        }
        else if (match(Token::EOF)) {
            if (implicit_bracket) {
                break;
            }
            else {
                throw SyntaxError("Reach end of file");
            }
        }
        else if (try_match(Token::ID)) {
            if (typename_cache.has_key(next_token.get_name().to_cstr())) {
                for (const auto& i : parse_var_decl()) {
                    ast->append(i);
                }
            }
            else {
                auto stmt_ast = parse_stmt();
                if (stmt_ast.exists()) {
                    ast->append(stmt_ast);
                }
            }
        }
        else {
            auto stmt_ast = parse_stmt();
            if (stmt_ast.exists()) {
                ast->append(stmt_ast);
            }
        }
    }

    return ast;
}

FunctionASTRef RDParser::parse_function_decl() {

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
            TypeASTRef arg_type;
            if (match_op(OpName::COLON)) {
                arg_type = parse_type();
            }
            else {
                arg_type = store_ast<TypeAST>(new TypeAST(store_type(new Type(Type::VOID))));
            }
            func->add_argument(arg_type, arg_name);
            if (match_op(OpName::RBRAC)) {
                break;
            }
            match_required_symbol(OpName::COMMA, ',');
        }
        else if (match_op(OpName::COLON)) { // :(type)
            TypeASTRef arg_type;
            if (match_op(OpName::COLON)) {
                arg_type = parse_type();
            }
            else {
                arg_type = store_ast<TypeAST>(new TypeAST(store_type(new Type(Type::VOID))));
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
        TypeASTRef ret_type = parse_type();
        func->set_return_type(ret_type);
    }
    else {
        func->set_return_type(store_ast<TypeAST>(new TypeAST(
            store_type(new Type(Type::VOID)))));
    }

    if (try_match_op(OpName::COMP)) {
        func->set_body_ast(parse_block_stmt());
    }
    else {
        match_required_symbol(OpName::SEMICOLON, ';');
    }
    
    return store_ast<FunctionAST>(func);
}


ClassASTRef RDParser::parse_class_decl() {
    
    if (!match_keyword(Keyword::CLASS)) {
        throw SyntaxError("Invalid definition");
    }

    if (!match(Token::ID)) {
        throw SyntaxError("Requires an identifier");
    }
    StringRef name = cur_token.get_name();
    MemoryRef<ClassAST> new_class = store_ast_unconst(new ClassAST(name));

    if (match_op(OpName::COMP)) {

    }
    else if (match_op(OpName::SEMICOLON)) {
        return new_class.to_const();
    }
    else {
        throw SyntaxError("Invalid class definition");
    }

    if (typename_cache.has_key(name.to_cstr())) {
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
    typename_cache.insert(name.to_string());
    return new_class;
}


ConstantRef RDParser::parse_value(const RawValue& rawval) {

    Constant* ret;

    if (rawval.type != RawValue::STRING) {
        const char* vstr = rawval.strval.to_cstr();
        char* end;

        if (rawval.type == RawValue::BOOL) {
            char val = rawval.strval == "true" ? 0 : 1;
            ret = new Constant(store_type(new PrimitiveType(Type::BOOL)), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::CHAR) {
            char val = vstr[0];
            ret = new Constant(store_type(new PrimitiveType(Type::INT)), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::INT) {
            unsigned val = strtol(vstr, &end, 10);
            ret = new Constant(store_type(new PrimitiveType(Type::INT)), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::FLOAT) {
            double val = strtod(vstr, &end);
            ret = new Constant(store_type(new PrimitiveType(Type::FLOAT)), (char*)&val, sizeof(val));
        }
        else { //won't actually happen
            ret = nullptr;
        }
    }
    else {
        ret = new Constant(store_type(new PointerType(store_type(new PrimitiveType(Type::CHAR)))),
            rawval.strval.to_cstr(), rawval.strval.length());
    }

    return _context->constantpool.collect(ret).to_const();
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
