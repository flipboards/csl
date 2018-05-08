#pragma once

#ifndef CSL_TYPE_H
#define CSL_TYPE_H

#include <ostream>
#include <vector>
#include <string>

#include "util/memory.h"


// Base of all types
class Type {
public:

    enum TypeID {
        VOID = 0x0,
        BOOL = 0x1,
        CHAR = 0x2,
        INT = 0x3,
        FLOAT = 0x4,
        Label = 0x8,
        Function = 0x9,
        Pointer = 0xa,
        Array = 0xb,
        Class = 0xc
    };

    Type() : id(VOID) {

    }

    Type(const TypeID id) : id(id) {

    }

    virtual ~Type() {

    }

    TypeID get_id()const {
        return id;
    }

    bool is_void()const {
        return get_id() == VOID;
    }

    bool is_integer_type()const {
        return get_id() < FLOAT && get_id() > VOID;
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
    }

protected:

    TypeID id;

};

typedef ConstMemoryRef<Type> TypeRef;

// basic types (void, bool, char, int, float)
class PrimitiveType : public Type {
public:
    PrimitiveType() : Type() {

    }

    PrimitiveType(const TypeID id) : Type(id) {
        assert(is_primitive() && "Not a primitive type");
    }

    void print(std::ostream& os)const {
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
};

// pointer
class PointerType : public Type {
public:

    explicit PointerType(const TypeRef& pointee) : Type(Pointer), pointee(pointee) {

    }

    void print(std::ostream& os)const {
        pointee->print(os);
        os << '*';
    }

    ~PointerType() {
    }

private:

    TypeRef pointee;
};


class ArrayType : public Type {
public:

    explicit ArrayType(const TypeRef& elmtype, unsigned elmnum) : eltype(elmtype), elnum(elmnum) {

    }

    void print(std::ostream& os)const {
        os << "[ " << elnum << " x ";
        eltype->print(os);
        os << "]";
    }

    ~ArrayType() {

    }

private:

    TypeRef eltype;
    unsigned elnum;
};


class ClassType : public Type {
public:

    explicit ClassType(const StringRef& name, const std::vector<TypeRef>& eltypes) :
        name(name), eltypes(eltypes) {

    }

    void add_element(const TypeRef& type) {
        eltypes.push_back(type);
    }

    void print(std::ostream& os)const {
        os << "class" << name.to_cstr();
    }

private:

    StringRef name;
    std::vector<TypeRef> eltypes;
};

#endif
