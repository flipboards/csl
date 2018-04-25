
#ifndef CSL_AST_H
#define CSL_AST_H

#include "operator.h"
#include "type.h"

#include <vector>
#include <string>
#include <ostream>
#include <string.h>

typedef const Type* TypeRef;
typedef std::string StringRef;


class ASTBase {
public:
    /*
    enum Type {
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
    };*/

    virtual void print(std::ostream&, char indent='\t', int level = 0)const {

    }

    /* Notice: When destructing, only delete member that is also AST;
        But left other types (StringRef, TypeRef)
    */
    virtual ~ASTBase() {

    }

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

    ValueAST(const Type* tp, const char* v_start, size_t size) : type(tp), 
        buffer(v_start, v_start + size) {
    }

    void add_child(ExprAST*) {
        throw std::out_of_range("Cannot add child to identifier");
    }

    void print(std::ostream& os, char indent = '\t', int level = 0)const {
        for (int i = 0; i < level; i++) os << indent;

        type->print(os);
        os << ' ';

        switch (type->get_id())
        {
            case Type::BOOL:
                os << buffer[0] ? "true" : "false";
                break;
            case Type::CHAR:
                os << '\'' << buffer[0] << '\'';
                break;
            case Type::INT:
                os << *(unsigned*)&buffer[0];
                break;
            case Type::FLOAT:
                os << *(double*)&buffer[0];
                break;
            default:
                break;
        }
        os << std::endl;
    }

private:

    const Type* type;
    std::vector<char> buffer;

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
            callee = dynamic_cast<IdAST*>(child);
            if (!callee) {
                throw std::bad_cast();
            }
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


// Variable declaration
class VarDeclAST : DeclAST {
public:

    VarDeclAST() : vartype(nullptr), initializer(nullptr) {

    }

    VarDeclAST(TypeRef type, StringRef name) : vartype(type), varname(name) {

    }

    VarDeclAST(TypeRef type, StringRef name, ExprAST* initializer) :
        vartype(type), varname(name), initializer(initializer) {

    }

    ~VarDeclAST() {
        delete initializer;
    }

private:
    TypeRef vartype;
    StringRef varname;
    ExprAST * initializer;
};



class BlockStmtAST : StmtAST {
public:

private:

    std::vector<VarDeclAST*> decl_list;
    std::vector<StmtAST*> stmt_list;
};

class ControlAST : public StmtAST {

};


class IfAST : ControlAST {

public:

private:
    ExprAST * condition;
    StmtAST* true_stmt;
    StmtAST* false_stmt;
};


class WhileAST : public StmtAST {
public:

private:
    ExprAST * condition;
    StmtAST* loop_stmt;
};


class ForAST : public StmtAST {
public:

private:
    ExprAST * init_expr;
    ExprAST * condition;
    ExprAST * loop_expr;
    StmtAST * loop_stmt;
};

class ContinueAST : public StmtAST {
public:

private:
};


class BreakAST : public StmtAST {
public:

private:
};

class ReturnAST : public StmtAST {
public:

private:

    ExprAST * ret_expr;
};

class FunctionAST : public DeclAST {
public:

    struct Signature {
        StringRef name;
        std::vector<const Type*> arg_types;
        const Type* ret_type;
    };

private:
    StringRef name;
    std::vector<Type*> arg_types;
    const Type* ret_type;

    std::vector<StringRef> arg_names;
    BlockStmtAST* body;
};

#endif // !CSL_AST_H
