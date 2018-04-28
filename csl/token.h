#pragma once

#ifndef CSL_TOKEN_H
#define CSL_TOKEN_H

#include <cassert>
#include <string>

#ifdef EOF
#undef EOF

enum class Keyword : unsigned {
	// control
	IF = 0x01, 
	ELSE = 0x02, 
	FOR = 0x03, 
	WHILE = 0x04, 
	RETURN = 0x05, 
	BREAK = 0x06, 
	CONTINUE = 0x07,
	// decl
	FN = 0x10, 
	CLASS = 0x11, 
	IMPORT = 0x12
};

enum class OpName : unsigned {
    ADD = 0x01,
    SUB = 0x02,
    MUL = 0x03,
    DIV = 0x04,
    MOD = 0x05,
    POW = 0x06,

    INC = 0x09,
    DEC = 0x0a,

    MBER = 0x10,
    ARROW = 0x11,
    ADDR = 0x12,
    DEREF = 0x13,
    INDEX = 0x14,
    RINDEX = 0x15,
    BRAC = 0x16,
    RBRAC = 0x17,

    EQ = 0x20,
    NE = 0x21,
    LT = 0x22,
    LE = 0x23,
    GT = 0x24,
    GE = 0x25,
    AND = 0x26,
    OR = 0x27,
    XOR = 0x28,
    NOT = 0x29,

    ASN = 0x30,
    ADDASN = 0x31,
    SUBASN = 0x32,
    MULASN = 0x33,
    DIVASN = 0x34,
    MODASN = 0x35,
    POWASN = 0x36,

    COMMA = 0x40,
    COLON = 0x41,
    SEMICOLON = 0x42,
	COMP = 0x43,
	RCOMP = 0x44
};

struct RawValue {
	enum Type {
		BOOL=0x02,
		CHAR=0x03,
		INT=0x04,
		FLOAT=0x05,
		STRING=0x08
	};

	Type type;
	std::string strval;
};

class Token {
public:

	enum TokenType {
		NONE = 0, VALUE, ID, OP, KEYWORD, EOF
	};

	Token() : _type(NONE) {
	}
	Token(TokenType type) : _type(type) {
	}
	Token(TokenType type, const std::string& str) : _type(type) {
		set_name(str);
	}
	Token(TokenType type, unsigned data) : _type(type), _uintdata(data) {
	}

	Token(const Token& token) : _type(token._type), _uintdata(token._uintdata) {
		if (_type == ID || _type == VALUE) {
			_strdata = token._strdata;
		}
	}


    TokenType get_type() const {
		return _type;
	}
	void set_type(TokenType type) {
		_type = type;
	}
	bool is_type(TokenType type) const {
		return _type == type;
	}

	void set_keyword(Keyword keyword) {
		assert(_type == KEYWORD && "Cannot set keyword");

		_uintdata = static_cast<unsigned>(keyword);
	}

	Keyword get_keyword() const {
		assert(_type == KEYWORD && "Cannot get keyword from non-literal");

		return static_cast<Keyword>(_uintdata);
	}

	void set_operator(OpName opname) {
		assert(_type == OP && "Cannot set operator");

		_uintdata = static_cast<unsigned>(opname);
	}

	OpName get_operator() {
		assert(_type == OP && "Cannot set operator");
		
		return static_cast<OpName>(_uintdata);
	}

	const std::string get_name()const {
		assert(_type == ID && "Cannot get name from non-id");
		return _strdata;
	}

	void set_name(const std::string& name) {
		assert(_type == ID && "Cannot set name to non-id");
		_strdata = name;
	}

	// set value of Token. Only used when type is VALUE
	void set_value(RawValue::Type type, const std::string& data) {
		assert(_type == VALUE && "Cannot set value to non-value");
		_uintdata = static_cast<unsigned>(type);
		_strdata = data;
	}

	// 
	RawValue get_value() {
		assert(_type == VALUE && "Cannot get value to non-value");
		return { static_cast<RawValue::Type>(_uintdata), _strdata};
	}

private:

    TokenType _type;
	unsigned _uintdata;			// stores op/keyword
	std::string _strdata;		// stores id/value
};

#endif
#endif