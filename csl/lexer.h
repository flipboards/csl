#pragma once

#ifndef CSL_LEXER_H
#define CSL_LEXER_H

#include "util/ioutil.h"
#include "util/memory.h"
#include "util/strmap.h"
#include "token.h"

#include <string>
#include <regex>
#include <deque>
#include <unordered_map>


class Lexer {
public:

    Lexer();

    ~Lexer() {
        _reader = nullptr;
        _strpool = nullptr;
    }

    void clear();

    void load(StrReader* reader, ConstStringPool* strpool) {
        _reader = reader;
        _strpool = strpool;
    }

    Token get_token();

    Token look_ahead();

    void go_back();

    size_t cur_pos()const {
        return _reader->pos();
    }

    const StrReader* get_reader()const {
        return _reader;
    }

private:

    void fetch_token();

    Token _fetch_token();

    bool match(const std::regex&);

    StringRef make_ref(const std::string&);

    static std::regex re_ws;
    static std::regex re_op;
    static std::regex re_int;
    static std::regex re_float;
    static std::regex re_id;
    static std::regex re_str;
    static std::regex re_char;
    static StrMap<OpName> op_loc;
    static StrMap<Keyword> keyword_loc;

    StrReader* _reader;
    ConstStringPool* _strpool;
    std::deque<Token> token_buf;
    StringTmpRef token_str;
    size_t next_get_pos;
    size_t next_look_pos;
};


#endif