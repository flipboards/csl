
#ifndef CSL_VALUE_H
#define CSL_VALUE_H

#include <cassert>
#include <cstdint>
#include <vector>

#include "util/memory.h"
#include "type.h"

namespace csl {

typedef std::vector<char> ByteRef;

/* Base of all values. Do not use directly */
class Value {
public:

	enum ValueID {
		CONSTANT,
		GLOBALVAR,
		FUNCTION,
		LOCALVAR
	};

    Value() : type(nullptr) {

    }

    TypeRef get_type()const {
        return type;
    }

    bool is_constant()const {
        return _id == CONSTANT || _id == GLOBALVAR || _id == FUNCTION;
    }

protected:

	explicit Value(ValueID id) : _id(id) {

	}

	explicit Value(const TypeRef& tp, ValueID id) : type(tp), _id(id) {

	}

    TypeRef type;
	ValueID _id;
};

/* Constant value (constant before compiling) */
class Constant : public Value {
public:

    Constant() : Value(CONSTANT) {

    }

    Constant(const TypeRef& tp, const char* v_start, size_t size) :
        Value(tp, CONSTANT), buffer(v_start, v_start + size) {

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

/* Global variable & function. Only serves as a virtual pointer */
class GlobalValue : public Value {

protected:

    explicit GlobalValue(ValueID id) : Value(id) {

    }
};

class GlobalVar : public GlobalValue {
public:

    GlobalVar() : GlobalValue(GLOBALVAR) {

    }
};


class Function : public GlobalValue {

};

class MemoryEntry : public Value {

};

}

#endif