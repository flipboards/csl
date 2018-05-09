
#ifndef CSL_VALUE_H
#define CSL_VALUE_H

#include <cassert>
#include <cstdint>
#include <vector>

#include "util/memory.h"
#include "type.h"


typedef std::vector<char> ByteRef;


class Value {
public:

    Value() : type(nullptr) {

    }

    explicit Value(bool is_const) : is_const(is_const) {

    }

    explicit Value(const TypeRef& tp, bool is_const) : type(tp), 
        is_const(is_const) {

    }

    virtual ~Value() {

    }

    TypeRef get_type()const {
        return type;
    }

    bool is_constant()const {
        return is_const;
    }

protected:

    TypeRef type;
    bool is_const;
};


class Constant : public Value {
public:

    Constant() : Value(true) {

    }

    Constant(const TypeRef& tp, const char* v_start, size_t size) :
        Value(tp, true), buffer(v_start, v_start + size) {

    }

    bool get_bool()const {
        assert(type->get_id() == Type::BOOL && "Is not boolean");
        return *(bool*)&buffer[0];
    }

    char get_char()const {
        assert(type->get_id() == Type::CHAR && "Is not char");
        return buffer[0];
    }

    unsigned get_int()const {
        assert(type->get_id() == Type::INT && "Is not int");
        return *(int*)&buffer[0];
    }

    double get_float()const {
        assert(type->get_id() == Type::FLOAT && "Is not float");
        return *(float*)&buffer[0];
    }

    unsigned get_integer_value()const {
        assert(type->is_integer_type() && "Is not inteager type");
        switch (type->get_id())
        {
        case Type::BOOL:
            return (int)get_bool();
            break;
        case Type::CHAR:
            return (int)get_char();
            break;
        case Type::INT:
            return (int)get_int();
            break;
        default:
            break;
        }
    }

    // output raw data
    const char* get_string()const {
        return &buffer[0];
    }

    ByteRef get_byteref()const {
        return buffer;
    }

private:
    ByteRef buffer;
};

typedef ConstMemoryRef<Constant> ConstantRef;

class GlobalValue : public Value {
public:

    GlobalValue() : Value(true) {

    }
};


class GlobalVar : public GlobalValue {
public:

    GlobalVar() {

    }
};


class Function : public GlobalValue {

};

class MemoryEntry : public Value {

};

#endif