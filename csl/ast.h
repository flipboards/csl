
#ifndef CSL_AST_H
#define CSL_AST_H

#include "operator.h"
#include "type.h"
#include "value.h"

#include <vector>
#include <string>
#include <ostream>
#include <string.h>

typedef const Type* TypeRef;
typedef std::string StringRef;


class ASTBase {
public:
    
    enum ASTType {
        NONE = 0x0, // default
        VALUE = 0x1,// terminal: value
        ID = 0x2,	// terminal: id
        TYPE = 0x3,	// terminal: type
        OP = 0x4,	// operator
        CALL = 0x5,	// function call
        LIST = 0x6,	// initialization list
        EXPR = 0x7,	// general expression
        CTRL = 0x8,	// control
        BLOCK = 0x9,// block of statements
        DECL = 0xa,	// declaration
        FN = 0xb,	// function definition
        ROOT = 0xc	// root
    };

    ASTBase() {

    }

    explicit ASTBase(ASTType type) : mytype(type) {

    }

    virtual void print(std::ostream&, char indent='\t', int level = 0)const {

    }

    bool is_id()const {
        return mytype == ID;
    }

    /* Notice: When destructing, only delete member that is also AST;
        But left other types (StringRef, TypeRef)
    */
    virtual ~ASTBase() {

    }

protected:

    ASTType mytype;
};


class StmtAST : public ASTBase {

};


// EXPR (7)
class ExprAST : public StmtAST {
public:

    virtual void print(std::ostream&, char indent='\t', int level = 0)const {

    }

    virtual void add_child(ExprAST* child) {

    }

private:

};

// OP (4)
class OpAST : public ExprAST {
public:

    OpAST() : _lhs(nullptr), _rhs(nullptr) {

    }

    OpAST(Operator op) : _op(op), _lhs(nullptr), _rhs(nullptr) {

    }

    OpAST(Operator op, ExprAST* lhs, ExprAST* rhs) : _op(op),
        _lhs(lhs), _rhs(rhs) {

    }

    ~OpAST() {
        delete _lhs;
        delete _rhs;
    }

    void add_child(ExprAST* child) {
        if (!_lhs) {
            _lhs = child;
        }
        else if (!_rhs) {
            _rhs = child;
        }
        else {
            throw std::out_of_range("OpAST already full");
        }
    }

    void print(std::ostream& os, char indent='\t', int level=0)const {
        for (int i = 0; i < level; i++) os << indent;

        print_op(_op, os);
        os << std::endl;
        _lhs->print(os, indent, level + 1);
        if (_rhs) _rhs->print(os, indent, level + 1);
    }

private:

    Operator _op;
    ExprAST * _lhs, * _rhs;
};

// VALUE (1)
class ValueAST : public ExprAST {
public:

    ValueAST() {

    }

    ValueAST(const Constant* c) : data(c) {
    }

    void add_child(ExprAST*) {
        throw std::out_of_range("Cannot add child to identifier");
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        for (int i = 0; i < level; i++) os << indent;

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

    const Constant* data;

};

// ID (2)
class IdAST : public ExprAST {
public:
    IdAST() {

    }

    IdAST(const StringRef& name) : name(name) {

    }

    void add_child(ExprAST*) {
        throw std::out_of_range("Cannot add child to identifier");
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        for (int i = 0; i < level; i++) os << indent;

        os << "id " << name << std::endl;
    }

    StringRef get_name()const {
        return name;
    }

private:

    StringRef name;
};

// CALL (5)
class CallAST : public ExprAST {
public:

    CallAST() : callee(nullptr) {

    }

    ~CallAST() {
        for (const ExprAST* arg : argv) {
            delete arg;
        }
        delete callee;
    }

    void add_child(ExprAST* child) {
        if (!callee) {
            callee = static_cast<IdAST*>(child);
        }
        else {
            argv.push_back(child);
        }
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        for (int i = 0; i < level; i++) os << indent;

        os << "function call" << std::endl;
        callee->print(os, indent, level + 1);
        for (const ExprAST* arg : argv) {
            arg->print(os, indent, level+1);
        }
    }

private:
    IdAST * callee;
    std::vector<ExprAST*> argv;
};

// LIST (6)
class ListAST : public ExprAST {
public:

    void add_child(ExprAST* child) {
        member.push_back(child);
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        for (int i = 0; i < level; i++) os << indent;

        os << "initialization list" << std::endl;
        for (const ExprAST* m : member) {
            m->print(os, indent, level + 1);
        }
    }

private:

    std::vector<ExprAST*> member;
};

// Declaration of function/class/variable
class DeclAST : ASTBase {

};


class TypeAST : public ASTBase {
public:

    enum RelationToChild {
        NONE = 0,
        POINTER,
        ARRAY,
        CLASS
    };

    struct ClassChild {
        StringRef name;
    };

    TypeAST() {

    }

    explicit TypeAST(Type* type) : relation(NONE), child(type) {

    }

    explicit TypeAST(TypeAST* pointee) : relation(POINTER), child(pointee) {

    }

    explicit TypeAST(const StringRef& classname) : relation(CLASS), child(new ClassChild{ classname }) {

    }

    ~TypeAST() {
        if (is_class_type()) {
            delete child;
        }
    }

    RelationToChild get_relation()const {
        return relation;
    }

    TypeAST* get_pointee()const {
        assert(relation == POINTER && "Is not pointer");
        return reinterpret_cast<TypeAST*>(child);
    }

    Type* get_type()const {
        assert(relation == NONE && "Is not primitive type");
        return reinterpret_cast<Type*>(child);
    }

    bool is_primitive_type() {
        return relation == NONE;
    }

    bool is_array_type() {
        return relation == ARRAY;
    }

    bool is_class_type() {
        return relation == CLASS;
    }

protected:
    
    RelationToChild relation;
    void * child;
};


class ArrayTypeAST : public TypeAST {
public:

    ArrayTypeAST() {

    }

    ArrayTypeAST(TypeAST* typee, ExprAST* expr_size) : TypeAST(typee), expr_size(expr_size) {

    }

private:

    ExprAST * expr_size;
};


// Variable declaration
class VarDeclAST : DeclAST {
public:

    VarDeclAST() : vartype(nullptr), initializer(nullptr) {

    }

    VarDeclAST(TypeAST* type, StringRef name) : vartype(type), varname(name) {

    }

    VarDeclAST(TypeAST* type, StringRef name, ExprAST* initializer) :
        vartype(type), varname(name), initializer(initializer) {

    }

    ~VarDeclAST() {
        delete initializer;
    }

private:
    TypeAST* vartype;
    StringRef varname;
    ExprAST * initializer;
};



class BlockStmtAST : public StmtAST {
public:

    BlockStmtAST() {

    }

    void append(VarDeclAST* d) {
        decl_list.push_back(d);
    }

    void append(StmtAST* d) {
        stmt_list.push_back(d);
    }

private:

    std::vector<VarDeclAST*> decl_list;
    std::vector<StmtAST*> stmt_list;
};

class ControlAST : public StmtAST {
public:
};


class IfAST : public ControlAST {

public:

    IfAST() : ControlAST(), condition(nullptr), true_stmt(nullptr), false_stmt(nullptr) {

    }

    IfAST(ExprAST* expr_cond, StmtAST* true_stmt) :
        condition(expr_cond), true_stmt(true_stmt), false_stmt(nullptr) {

    }

    IfAST(ExprAST* expr_cond, StmtAST* true_stmt, StmtAST* false_stmt) :
        condition(expr_cond), true_stmt(true_stmt), false_stmt(false_stmt) {

    }

private:
    ExprAST * condition;
    StmtAST* true_stmt;
    StmtAST* false_stmt;
};


class WhileAST : public ControlAST {
public:

    WhileAST() : condition(nullptr), loop_stmt(nullptr) {

    }

    WhileAST(ExprAST* expr_cond, StmtAST* stmt) :
        condition(expr_cond), loop_stmt(stmt) {

    }

private:
    ExprAST * condition;
    StmtAST* loop_stmt;
};


class ForAST : public ControlAST {
public:

    ForAST() : init_expr(nullptr),
        condition(nullptr),
        loop_expr(nullptr),
        loop_stmt(nullptr) {

    }

    explicit ForAST(ExprAST* init_expr, ExprAST* cond_expr, ExprAST* loop_expr, StmtAST* loop_stmt) :
        init_expr(init_expr),
        condition(cond_expr),
        loop_expr(loop_expr),
        loop_stmt(loop_stmt) {

    }


private:
    ExprAST * init_expr;
    ExprAST * condition;
    ExprAST * loop_expr;
    StmtAST * loop_stmt;
};

class ContinueAST : public ControlAST {
public:

private:
};


class BreakAST : public ControlAST {
public:

private:
};

class ReturnAST : public ControlAST {
public:

    ReturnAST() : ret_expr(nullptr) {

    }

    ReturnAST(ExprAST* ret_expr) : ret_expr(ret_expr) {

    }

private:

    ExprAST * ret_expr;
};

class FunctionAST : public DeclAST {
public:

    FunctionAST() : ret_type(nullptr), body(nullptr) {

    }

    explicit FunctionAST(StringRef name) : name(name), ret_type(nullptr), body(nullptr) {

    }

    void add_argument(TypeAST* type) {
        arg_types.push_back(type);
        arg_names.push_back(StringRef());
    }

    void add_argument(TypeAST* type, StringRef name) {
        arg_types.push_back(type);
        arg_names.push_back(name);
    }

    void set_return_type(TypeAST* type) {
        ret_type = type;
    }

    void set_body_ast(BlockStmtAST* body_ast) {
        body = body_ast;
    }

private:
    StringRef name;
    std::vector<TypeAST*> arg_types;
    std::vector<StringRef> arg_names;
    TypeAST* ret_type;

    BlockStmtAST* body;
};


class ClassAST : public DeclAST {
public:

    ClassAST() : DeclAST() {

    }

    ClassAST(StringRef name) : name(name) {

    }

    void add_member(VarDeclAST* ast_member) {
        ast_members.push_back(ast_member);
    }

    void add_method(FunctionAST* ast_method) {
        ast_methods.push_back(ast_method);
    }

private:

    StringRef name;
    std::vector<VarDeclAST*> ast_members;
    std::vector<FunctionAST*> ast_methods;

};

#endif // !CSL_AST_H
