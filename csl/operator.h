#pragma once

#ifndef CSL_OPERATOR_H
#define CSL_OPERATOR_H

#include <unordered_map>
#include <ostream>

enum class Operator : unsigned {
    NONE = 0x00,
    ADD = 0x01, 
    SUB = 0x02, 
    MUL = 0x03,
    DIV = 0x04,
    MOD = 0x05,
    POW = 0x06,
    PLUS = 0x07, 
    MINUS = 0x08, 
    INC = 0x09,
    DEC = 0x0a,
    POSTINC = 0x0b,
    POSTDEC = 0x0c,

    MBER = 0x10,
    ARROW = 0x11,
    ADDR = 0x12,
    DEREF = 0x13,
    INDEX = 0x14,

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
    POWASN = 0x36

};


inline bool is_valid(Operator op) {
    return op <= Operator::POSTINC || 
        op >= Operator::MBER && op <= Operator::INDEX || 
        op >= Operator::EQ && op <= Operator::NOT || 
        op >= Operator::ASN && op <= Operator::POWASN;
}

// return if op belongs to '[+-*/%^]?=' (assignment)
inline bool is_assignment(Operator op) {
    return op >= Operator::ASN && op <= Operator::POWASN;
}

// return if op belongs to + - * / ^ %
inline bool is_arithmetic(Operator op) {
    return op >= Operator::ADD && op < Operator::PLUS;
}

// return if op belongs to (and or xor not > < ...)
inline bool is_logic(Operator op) {
    return op >= Operator::EQ && op < Operator::ASN;
}

// return if op belongs to (and or xor > < ...)
inline bool is_binary_logic(Operator op) {
    return op >= Operator::EQ && op < Operator::ASN && op != Operator::NOT;
}

// if a binary operator
inline bool is_binary(Operator op) {
    return op <= Operator::POW || op == Operator::MBER || op == Operator::ARROW || op >= Operator::EQ && op != Operator::NOT;
}

// return precedence
inline unsigned get_precedence(Operator op) {

    static std::unordered_map<Operator, unsigned> precedence{
        { Operator::NONE, 100 },
        { Operator::ADD, 5 },{ Operator::SUB, 5 },
        { Operator::MUL, 4 },{ Operator::DIV, 4 },{ Operator::MOD, 4 },
        { Operator::POW, 3 },
        { Operator::PLUS, 2 },{ Operator::MINUS, 2 },{ Operator::INC, 2 },{ Operator::DEC, 2 },
        { Operator::POSTINC, 1 },{ Operator::POSTDEC, 1 },
        { Operator::EQ, 7 },{ Operator::NE, 7 },
        { Operator::LT, 6 },{ Operator::LE, 6 },{ Operator::GT, 6 },{ Operator::GE, 6 },
        { Operator::AND, 8 },{ Operator::XOR, 9 },{ Operator::OR, 10 }
    };

    return precedence.at(op);
}

inline void print_op(Operator op, std::ostream& os) {
#define PRINT_OP(p) case(Operator::p): os << #p; break;

    switch (op)
    {
        PRINT_OP(ADD)
        PRINT_OP(SUB)
        PRINT_OP(MUL)
        PRINT_OP(DIV)
        PRINT_OP(MOD)
        PRINT_OP(POW)
        PRINT_OP(PLUS)
        PRINT_OP(MINUS)
        PRINT_OP(INC)
        PRINT_OP(DEC)
        PRINT_OP(POSTINC)
        PRINT_OP(POSTDEC)
        PRINT_OP(MBER)
        PRINT_OP(ARROW)
        PRINT_OP(ADDR)
        PRINT_OP(DEREF)
        PRINT_OP(INDEX)
        PRINT_OP(EQ)
        PRINT_OP(NE)
        PRINT_OP(LT)
        PRINT_OP(LE)
        PRINT_OP(GT)
        PRINT_OP(GE)
        PRINT_OP(AND)
        PRINT_OP(OR)
        PRINT_OP(XOR)
        PRINT_OP(NOT)
        PRINT_OP(ASN)
        PRINT_OP(ADDASN)
        PRINT_OP(SUBASN)
        PRINT_OP(MULASN)
        PRINT_OP(DIVASN)
        PRINT_OP(MODASN)
        PRINT_OP(POWASN)
    default:
        break;
    }
#undef PRINT_OP
}


#endif