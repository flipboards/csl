#pragma once

#ifndef CSL_LEXER_H
#define CSL_LEXER_H

#include "token.h"
#include "util/ioutil.h"

#include <string>
#include <regex>
#include <deque>
#include <unordered_map>


class Lexer {
public:

    Lexer();

    ~Lexer() {
        _reader = nullptr;
    }

    void clear();

    void load(StrReader* reader) {
        _reader = reader;
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

    static std::regex re_ws;
    static std::regex re_op;
    static std::regex re_int;
    static std::regex re_float;
    static std::regex re_id;
    static std::regex re_str;
    static std::regex re_char;
    static std::unordered_map<std::string, OpName> op_loc;
    static std::unordered_map<std::string, Keyword> keyword_loc;

    StrReader* _reader;
    std::deque<Token> token_buf;
    std::string token_str;
    size_t next_get_pos;
    size_t next_look_pos;
};


#endif