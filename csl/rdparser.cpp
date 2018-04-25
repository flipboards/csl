
#include "parser.h"
#include "astbuilder.h"
#include "operator.h"

#include "util/errors.h"

std::unordered_map<std::string, ASTBase*> RDParser::ast_cache;

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

    if (match(Token::Type::ID)) {
        builder_postfix.extend_child(static_cast<ExprAST*>(new IdAST(cur_token.get_name())));
    }
    else if (match(Token::Type::VALUE)) {
        builder_postfix.extend_child(build_value_ast(cur_token.get_value()));
    }
    else if (match_op(OpName::BRAC)) {
        builder_postfix.extend_child(parse_expr());
        if (!match_op(OpName::RBRAC)) {
            throw SyntaxError("')' expected");
        }
    }
    else {
        throw SyntaxError("Token with id/value required");
    }

    while (1) {
        if (match_op(OpName::INDEX)) {
            builder_postfix.extend_parent(new OpAST(Operator::INDEX));
            builder_postfix.add_child(parse_expr());
            if (!match_op(OpName::RINDEX)) {
                throw SyntaxError("']' required");
            }
        }
        else if (match_op(OpName::BRAC)) {
            try {
                builder_postfix.extend_parent(new CallAST());
            }
            catch (const std::bad_alloc&) {
                throw SyntaxError("Must call an identifier");
            }
            if (!match_op(OpName::RBRAC)) {
                while (1) {
                    builder_postfix.add_child(parse_expr());
                    if (!match_sep(OpName::COMMA)) {
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
            if (match(Token::Type::ID)) {
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
        if (!try_match(Token::Type::OP)) {
            break;
        }

        OpName opname = next_token.get_operator();
        if (opname == OpName::RBRAC || opname == OpName::RINDEX) break;
        
        Operator op = static_cast<Operator>(opname);

        if (is_assignment(op)) break;

        match(Token::Type::OP);

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

    ExprAST* ast = parse_simple_expr();

    if (try_match(Token::Type::OP)) {
        Operator op = static_cast<Operator>(next_token.get_operator());
        if (is_assignment(op)) {
            eat();
            return new OpAST(op, ast, parse_expr());
        }
        else {
            return ast;
        }
    }
    else {
        return ast;
    }

}

std::vector<VarDeclAST*> RDParser::parse_var_decl() {

    Type* vartype;

    if (match(Token::Type::TYPE)) {
        vartype = new PrimitiveType(static_cast<Type::TypeID>(cur_token.get_type()));
    }
    else if (match(Token::Type::ID)) {
        try {
            vartype = type_cache.at(cur_token.get_name());
        }
        catch (const std::out_of_range&) {
            throw SyntaxError("Type undefined: " + cur_token.get_name());
        }
    }
    else {
        throw SyntaxError("Type name required");
    }

    std::vector<VarDeclAST*> decl_ast_list;

    while (1) {

        Type* cur_vartype;
        StringRef cur_varname;
        VarDeclAST* decl_ast;

        while (1) {
            // pointer
            if (match_op(OpName::MUL)) {
                cur_vartype = new PointerType(cur_vartype);
                
            }
            else if (match_op(OpName::INDEX)) {
                ExprAST* idx_ast = parse_expr();
                if (!match_op(OpName::RINDEX)) {
                    throw SyntaxError("']' required");
                }
                unsigned arr_size = eval_const_expr(idx_ast);
                cur_vartype = new ArrayType(cur_vartype, arr_size);
            }
            else {
                break;
            }
        }

        if (match(Token::Type::ID)) {
            cur_varname = cur_token.get_name();
        }
        else {
            throw SyntaxError("Identifier required for declaration");
        }

        if (match_sep(OpName::COMMA)) {
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

}

ExprAST * RDParser::parse_initializer()
{
    ExprAST* initializer;
    if (match_sep(OpName::COMP)) {
        initializer = new ListAST();
        while (1) {
            initializer->add_child(parse_initializer());
            if (!match_sep(OpName::COMMA)) {
                break;
            }
        }
        if (!match_sep(OpName::RCOMP)) {
            throw SyntaxError("'}' required");
        }
    }
    else {
        initializer = parse_expr();
    }
    return initializer;
}

ValueAST* RDParser::build_value_ast(const RawValue& rawval) {
    if (rawval.type != RawValue::Type::STRING) {
        const char* vstr = rawval.strval.c_str();
        char* end;

        if (rawval.type == RawValue::Type::BOOL) {
            char val = rawval.strval == "true" ? 0 : 1;
            return new ValueAST(new PrimitiveType(Type::TypeID::BOOL), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::Type::CHAR) {
            char val = rawval.strval[0];
            return new ValueAST(new PrimitiveType(Type::TypeID::INT), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::Type::INT) {
            unsigned val = strtol(vstr, &end, 10);
            return new ValueAST(new PrimitiveType(Type::TypeID::INT), (char*)&val, sizeof(val));
        }

        else if (rawval.type == RawValue::Type::FLOAT) {
            double val = strtod(vstr, &end);
            return new ValueAST(new PrimitiveType(Type::TypeID::FLOAT), (char*)&val, sizeof(val));
        }
        else { //won't actually happen
            return nullptr;
        }
    }
    else {
        return new ValueAST(
            static_cast<Type*>(new PointerType(new PrimitiveType(Type::TypeID::CHAR))),
            rawval.strval.c_str(), rawval.strval.length());
    }
}

void RDParser::eat() {
    cur_token = next_token;
    next_token = _lexer.get_token();
}

bool RDParser::match(Token::Type type) {
    if (next_token.is_type(type)) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::match(Token::Type type, bool *f(const Token&)) {
    if (next_token.is_type(type) && f(next_token)) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::try_match(Token::Type type)const {
    return next_token.is_type(type);
}

bool RDParser::match_op(OpName opname) {
    if (next_token.is_type(Token::Type::OP) && next_token.get_operator() == opname) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::match_sep(OpName opname) {
    if (next_token.is_type(Token::Type::SEP) && next_token.get_operator() == opname) {
        eat();
        return true;
    }
    else {
        return false;
    }
}

bool RDParser::try_match_sep(OpName opname)
{
    if (next_token.is_type(Token::Type::SEP) && next_token.get_operator() == opname) {
        return true;
    }
    else {
        return false;
    }
}
