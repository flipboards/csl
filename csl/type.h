#pragma once

#ifndef CSL_TYPE_H
#define CSL_TYPE_H

#include <ostream>
#include <vector>
#include <string>

class Type {
public:

    enum TypeID {
        VOID = 0x0,
        BOOL = 0x1,
        CHAR = 0x2,
        INT = 0x3,
        FLOAT = 0x4,
        Label = 0x9,
        Pointer = 0xa,
        Array = 0xb,
        Class = 0xc
    };

    Type() : id(VOID) {

    }

    Type(TypeID id) : id(id) {

    }

    virtual ~Type() {

    }

    TypeID get_id()const {
        return id;
    }

    bool is_primitive()const {
        return get_id() < Label;
    }

    bool is_aggregate()const {
        return get_id() > Pointer;
    }

    bool is_pointer()const {
        return get_id() == Pointer;
    }

    virtual void print(std::ostream& os)const {
        switch (id)
        {
        case Type::VOID:
            os << "void";
            break;
        case Type::BOOL:
            os << "bool";
            break;
        case Type::CHAR:
            os << "char";
            break;
        case Type::INT:
            os << "int";
            break;
        case Type::FLOAT:
            os << "float";
            break;
        default:
            break;
        }
    }

protected:

    TypeID id;

};

class PrimitiveType : public Type {

    using Type::Type;
};

class PointerType : public Type {
public:

    explicit PointerType(Type* pointee) : Type(Pointer), pointee(pointee) {

    }

    void print(std::ostream& os)const {
        pointee->print(os);
        os << '*';
    }

    ~PointerType() {
        delete pointee;
    }

private:

    Type * pointee;
};


class ArrayType : public Type {
public:

    explicit ArrayType(Type* elmtype, unsigned elmnum) : eltype(elmtype), elnum(elmnum) {

    }

    void print(std::ostream& os)const {
        os << "[ " << elnum << " x ";
        eltype->print(os);
        os << "]";
    }

    ~ArrayType() {
        delete eltype;
    }

private:

    Type * eltype;
    unsigned elnum;
};


class ClassType : public Type {
public:

    explicit ClassType(const std::string& name, const std::vector<Type*>& eltypes) : 
        name(name), eltypes(eltypes) {

    }

    void add_element(Type* type) {
        eltypes.push_back(type);
    }

    void print(std::ostream& os)const {
        os << "class" << name;
    }

    std::vector<Type*>::const_iterator begin() {
        return eltypes.begin();
    }

    std::vector<Type*>::const_iterator end() {
        return eltypes.end();
    }

    ~ClassType() {
        for (auto elt : eltypes) {
            delete elt;
        }
    }

private:

    std::string name;
    std::vector<Type*> eltypes;
};

#endif
