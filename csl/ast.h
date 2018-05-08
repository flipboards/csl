
#ifndef CSL_AST_H
#define CSL_AST_H


#include <vector>
#include <string>
#include <ostream>
#include <cstring>

#include "util/memory.h"
#include "operator.h"
#include "type.h"
#include "value.h"



class ASTBase {
public:
    
    enum ASTType {
        NONE = 0, // default
        /* Expressions */
        OP = 1,
        VALUE = 2,
        ID = 3,
        CALL = 4,
        LIST = 5,
        /* Declarations */
        DECL = 6,
        FUNCTION = 7,
        CLASS = 8,
        /* Type */
        TYPE = 9,
        /* Statements */
        BLOCK = 0x10,
        IF = 0x11,
        WHILE = 0x12,
        FOR = 0x13,
        CONTINUE = 0x14,
        BREAK = 0x15,
        RETURN = 0x16
    };

    ASTBase() : mytype(NONE) {

    }

    explicit ASTBase(ASTType type) : mytype(type) {

    }

    virtual void print(std::ostream&, char indent='\t', int level = 0)const {

    }

    bool is_stmt()const {
        return mytype >= OP && mytype < DECL || mytype >= BLOCK;
    }

    bool is_expr()const {
        return mytype >= OP && mytype < DECL;
    }

    bool is_decl()const {
        return mytype >= DECL && mytype < TYPE;
    }

    bool is_type()const {
        return mytype == TYPE;
    }

    bool is_id()const {
        return mytype == ID;
    }

    bool is_control()const {
        return mytype >= IF;
    }

    /*  Notice: CSL Member class (AST, Token, ...) will not release *ANY* pointers
        it holds. Please use references in MemoryPool.
    */
    virtual ~ASTBase() {

    }

protected:

    ASTType mytype;
};

typedef ConstMemoryRef<ASTBase> ASTRef;


// General Statement
class StmtAST : public ASTBase {
public:

    StmtAST() {

    }

    explicit StmtAST(ASTType type) : ASTBase(type) {

    }
};

typedef ConstMemoryRef<StmtAST> StmtASTRef;

// General Expression
class ExprAST : public StmtAST {
public:

    ExprAST() {

    }

    explicit ExprAST(ASTType type) : StmtAST(type) {

    }

    virtual void print(std::ostream&, char indent='\t', int level = 0)const {

    }

    virtual void add_child(const ConstMemoryRef<ExprAST>& child) {

    }

private:

};

typedef ConstMemoryRef<ExprAST> ExprASTRef;

// Operator (1)
class OpAST : public ExprAST {
public:

    OpAST() : ExprAST(OP), lhs(nullptr), rhs(nullptr) {

    }

    explicit OpAST(Operator op) : op(op), lhs(nullptr), rhs(nullptr) {

    }

    explicit OpAST(Operator op, const ExprASTRef& lhs, const ExprASTRef& rhs) : op(op),
        lhs(lhs), rhs(rhs) {

    }

    void add_child(const ExprASTRef& child) {
        if (!lhs) {
            lhs = child;
        }
        else if (!rhs) {
            rhs = child;
        }
        else {
            throw std::out_of_range("OpAST already full");
        }
    }

    void print(std::ostream& os, char indent='\t', int level=0)const {
        os << std::string(level, indent);

        print_op(op, os);
        os << std::endl;
        lhs->print(os, indent, level + 1);
        if (rhs.exists()) rhs->print(os, indent, level + 1);
    }

private:

    Operator op;
    ExprASTRef lhs, rhs;
};

// VALUE (2)
class ValueAST : public ExprAST {
public:

    ValueAST() : ExprAST(VALUE), data(nullptr) {

    }

    explicit ValueAST(const ConstMemoryRef<Constant>& c) : data(c) {
    }

    void add_child(const ExprASTRef&) {
        throw std::out_of_range("Cannot add child to identifier");
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        os << std::string(level, indent);

        data->get_type()->print(os);
        os << ' ';

        switch (data->get_type()->get_id())
        {
            case Type::BOOL:
                os << data->get_bool() ? "true" : "false";
                break;
            case Type::CHAR:
                os << '\'' << data->get_char() << '\'';
                break;
            case Type::INT:
                os << data->get_int();
                break;
            case Type::FLOAT:
                os << data->get_float();
                break;
            default:
                break;
        }
        os << std::endl;
    }

private:

    const ConstMemoryRef<Constant> data;

};

// ID (3)
class IdAST : public ExprAST {
public:
    IdAST() : ExprAST(ID) {

    }

    explicit IdAST(const StringRef& name) : name(name) {

    }

    void add_child(const ExprASTRef&) {
        throw std::out_of_range("Cannot add child to identifier");
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        os << std::string(level, indent);

        os << "id " << name.to_cstr() << std::endl;
    }

    StringRef get_name()const {
        return name;
    }

private:

    StringRef name;
};

// CALL (4)
class CallAST : public ExprAST {
public:

    CallAST() : ExprAST(CALL), callee(nullptr) {

    }

    ~CallAST() {
    }

    void add_child(const ExprASTRef& child) {
        if (!callee) {
            if (child->is_id()) {
                callee = child.cast<IdAST>();
            }
            else {
                throw std::runtime_error("Not an id");
            }
        }
        else {
            argv.push_back(child);
        }
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        os << std::string(level, indent) << std::endl;

        os << "function call" << std::endl;
        callee->print(os, indent, level + 1);
        for (const auto& arg : argv) {
            arg->print(os, indent, level+1);
        }
    }

private:
    ConstMemoryRef<IdAST> callee;
    std::vector<ExprASTRef> argv;
};

// LIST (5)
class ListAST : public ExprAST {
public:

    ListAST() : ExprAST(LIST) {

    }

    void add_child(const ExprASTRef& child) {
        member.push_back(child);
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        os << std::string(level, indent);

        os << "initialization list" << std::endl;
        for (const auto& m : member) {
            m->print(os, indent, level + 1);
        }
    }

private:

    std::vector<ExprASTRef> member;
};

// Declaration of function/class/variable
class DeclAST : public ASTBase {
public:

    DeclAST() {

    }

    DeclAST(ASTType type) : ASTBase(type) {

    }
};

// TYPE(9)
class TypeAST : public ASTBase {
public:

    enum RelationToChild {
        NONE = 0,
        POINTER,
        ARRAY,
        CLASS
    };

    TypeAST() : ASTBase(TYPE) {

    }

    explicit TypeAST(const TypeRef& type) : relation(NONE), child(type.cast<void>()) {

    }

    explicit TypeAST(const ConstMemoryRef<TypeAST>& pointee) : relation(POINTER), child(pointee.cast<void>()) {

    }

    explicit TypeAST(const StringRef& classname) : relation(CLASS), child(classname.to_memref().cast<void>()) {

    }

    ~TypeAST() {

    }

    RelationToChild get_relation()const {
        return relation;
    }

    ConstMemoryRef<TypeAST> get_pointee()const {
        assert(relation == POINTER && "Is not pointer");
        return child.cast<TypeAST>();
    }

    TypeRef get_type()const {
        assert(relation == NONE && "Is not primitive type");
        return child.cast<Type>();
    }

    bool is_primitive_type()const {
        return relation == NONE;
    }

    bool is_pointer_type()const {
        return relation == POINTER;
    }

    bool is_array_type()const {
        return relation == ARRAY;
    }

    bool is_class_type()const {
        return relation == CLASS;
    }

    virtual void print(std::ostream& os, char indent = '\t', int level = 0)const {

        os << std::string(level, indent);


        switch (relation)
        {
        case TypeAST::NONE:
            child.cast<Type>()->print(os);
            os << std::endl;
            break;
        case TypeAST::POINTER:
            os << "[Pointer of]" << std::endl;
            child.cast<TypeAST>()->print(os, indent, level + 1);
            break;
        case TypeAST::CLASS:
            os << "Custom type: " << child.cast<char>().get() << std::endl;
            break;
        default:
            break;
        }
    }


protected:
    
    RelationToChild relation;
    ConstMemoryRef<void> child;
};

typedef ConstMemoryRef<TypeAST> TypeASTRef;


class ArrayTypeAST : public TypeAST {
public:

    ArrayTypeAST() : TypeAST() {

    }

    ArrayTypeAST(const TypeASTRef& typee, const ExprASTRef& expr_size) : TypeAST(typee), expr_size(expr_size) {

    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        os << std::string(level, indent);
        os << "[Array type]" << std::endl;
        child.cast<TypeAST>()->print(os, indent, level + 1);
        expr_size->print(os, indent, level + 1);
    }
private:

    ExprASTRef expr_size;
};


// Variable declaration
class VarDeclAST : public DeclAST {
public:

    VarDeclAST() : DeclAST(DECL), vartype(nullptr), initializer(nullptr) {

    }

    explicit VarDeclAST(const TypeASTRef& type, const StringRef& name) : vartype(type), varname(name) {

    }

    explicit VarDeclAST(const TypeASTRef& type, const StringRef& name, const ExprASTRef& initializer) :
        vartype(type), varname(name), initializer(initializer) {

    }

    ~VarDeclAST() {
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        os << std::string(level, indent) << std::endl;
        os << "Declaration of name: " << varname.to_cstr() << std::endl;
        vartype->print(os, indent, level + 1);
        if (initializer.exists()) {
            initializer->print(os, indent, level + 1);
        }
    }


private:
    TypeASTRef vartype;
    StringRef varname;
    ExprASTRef initializer;
};

typedef typename ConstMemoryRef<VarDeclAST> VarDeclASTRef;


class BlockStmtAST : public StmtAST {
public:

    BlockStmtAST() : StmtAST(BLOCK) {

    }

    void append(const ConstMemoryRef<VarDeclAST>& d) {
        decl_list.push_back(d);
    }

    void append(const StmtASTRef& d) {
        stmt_list.push_back(d);
    }

    void print(std::ostream& os, char indent='\t', unsigned level=0) {
        os << std::string(level, indent) << "[Block]" << std::endl;
        for (const auto& d : decl_list) {
            d->print(os, indent, level + 1);
        }
        for (const auto& s : stmt_list) {
            s->print(os, indent, level + 1);
        }
    }

private:

    std::vector<ConstMemoryRef<VarDeclAST> > decl_list;
    std::vector<StmtASTRef> stmt_list;
};

typedef typename ConstMemoryRef<BlockStmtAST> BlockStmtASTRef;

class IfAST : public StmtAST {

public:

    IfAST() : StmtAST(IF), condition(nullptr), true_stmt(nullptr), false_stmt(nullptr) {

    }

    explicit IfAST(const ExprASTRef& expr_cond, const StmtASTRef& true_stmt) :
        condition(expr_cond), true_stmt(true_stmt), false_stmt(nullptr) {

    }

    explicit IfAST(const ExprASTRef& expr_cond, const StmtASTRef& true_stmt, const StmtASTRef& false_stmt) :
        condition(expr_cond), true_stmt(true_stmt), false_stmt(false_stmt) {

    }

private:
    ExprASTRef condition;
    StmtASTRef true_stmt;
    StmtASTRef false_stmt;
};


class WhileAST : public StmtAST {
public:

    WhileAST() : StmtAST(WHILE), condition(nullptr), loop_stmt(nullptr) {

    }

    explicit WhileAST(const ExprASTRef& expr_cond, const StmtASTRef& stmt) :
        condition(expr_cond), loop_stmt(stmt) {

    }

private:
    ExprASTRef condition;
    StmtASTRef loop_stmt;
};


class ForAST : public StmtAST {
public:

    ForAST() : StmtAST(FOR), init_expr(nullptr),
        condition(nullptr),
        loop_expr(nullptr),
        loop_stmt(nullptr) {

    }

    explicit ForAST(const ExprASTRef& init_expr, const ExprASTRef& cond_expr, const ExprASTRef& loop_expr, 
        const StmtASTRef& loop_stmt) :
        init_expr(init_expr),
        condition(cond_expr),
        loop_expr(loop_expr),
        loop_stmt(loop_stmt) {

    }


private:
    ExprASTRef init_expr;
    ExprASTRef condition;
    ExprASTRef loop_expr;
    StmtASTRef loop_stmt;
};

class ContinueAST : public StmtAST {
public:

    ContinueAST() : StmtAST(CONTINUE) {

    }

};


class BreakAST : public StmtAST {
public:
    BreakAST() : StmtAST(BREAK) {

    }
};

class ReturnAST : public StmtAST {
public:

    ReturnAST() : StmtAST(RETURN), ret_expr(nullptr) {

    }

    explicit ReturnAST(const ExprASTRef& ret_expr) : ret_expr(ret_expr) {

    }

private:

    ExprASTRef ret_expr;
};

/* FUNCTION(7) */
class FunctionAST : public DeclAST {
public:

    FunctionAST() : DeclAST(FUNCTION), ret_type(nullptr), body(nullptr) {

    }

    explicit FunctionAST(const StringRef& name) : name(name), ret_type(nullptr), body(nullptr) {

    }

    void add_argument(const TypeASTRef& type) {
        arg_types.push_back(type);
        arg_names.push_back(StringRef::null());
    }

    void add_argument(const TypeASTRef& type, const StringRef& name) {
        arg_types.push_back(type);
        arg_names.push_back(name);
    }

    void set_return_type(const TypeASTRef& type) {
        ret_type = type;
    }

    void set_body_ast(const BlockStmtASTRef& body_ast) {
        body = body_ast;
    }

private:
    StringRef name;
    std::vector<TypeASTRef> arg_types;
    std::vector<StringRef> arg_names;
    TypeASTRef ret_type;

    BlockStmtASTRef body;
};

typedef typename ConstMemoryRef<FunctionAST> FunctionASTRef;


// CLASS (8)
class ClassAST : public DeclAST {
public:

    ClassAST() : DeclAST(CLASS) {

    }

    explicit ClassAST(const StringRef& name) : name(name) {

    }

    void add_member(const VarDeclASTRef& ast_member) {
        ast_members.push_back(ast_member);
    }

    void add_method(const FunctionASTRef& ast_method) {
        ast_methods.push_back(ast_method);
    }

private:

    StringRef name;
    std::vector<VarDeclASTRef> ast_members;
    std::vector<FunctionASTRef> ast_methods;

};

typedef typename ConstMemoryRef<ClassAST> ClassASTRef;

#endif // !CSL_AST_H
