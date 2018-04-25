#pragma once

#ifndef CSL_VALUE_H
#define CSL_VALUE_H

#include <cassert>
#include <cstdint>

class Value {
public:

	enum Type {
		NONE = 0x00,
		CHAR = 0x03, // '.'
		INT = 0x04,
		FLOAT = 0x05,
		STR =0x06 // " ..."
	};

	Value() : _type(Type::NONE), _strdata(nullptr) {
	}
	Value(Type type) : _type(type), _strdata(nullptr) {
	}

	

	// set data for int/char
	void set_data(int data) {
		assert(_type == Type::CHAR || _type == Type::INT);
		_uintdata = static_cast<uint32_t>(data);
	}

	int get_data()const {
		assert(_type == Type::CHAR || _type == Type::INT);
		return static_cast<int>(_uintdata);
	}

	void set_string(const char* str) {
		_strdata = const_cast<char*>(str);
	}

	const char* get_string()const {
		return _strdata;
	}

private:

	Type _type;
	uint32_t _uintdata;
	char* _strdata;
};

#endif