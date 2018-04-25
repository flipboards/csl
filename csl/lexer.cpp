
#include <cassert>
#include <regex>

#include "lexer.h"
#include "util/errors.h"
#include "util/strutil.h"

std::regex Lexer::re_id("[_a-zA-Z][_0-9a-zA-Z]*");
std::regex Lexer::re_char("'(?:\\\\.|[^'\\\\])*'");
std::regex Lexer::re_str("\"(?:\\\\.|[^\"\\\\])*\"");
std::regex Lexer::re_int("\\d+");
std::regex Lexer::re_float("\\d*(?:\\.|e[+\\-]?)\\d+");
std::regex Lexer::re_op("\\+\\+|\\-\\-|\\!\\=|\\-\\>|[\\+\\-\\*\\/\\%\\=\\^\\<\\>]\\=?|[\\.\\,\\:\\{\\}\\(\\)\\[\\]]");
std::regex Lexer::re_ws("[ \\t\\n]+");
std::regex Lexer::re_eol(";");

std::regex re_single_char("'\\\\?.'");

std::unordered_map<std::string, Keyword> Lexer::keyword_loc {
    {"if", Keyword::IF}, {"else", Keyword::ELSE}, {"while", Keyword::ELSE}, 
    {"for", Keyword::FOR}, {"break", Keyword::BREAK}, {"continue", Keyword::CONTINUE}, 
    {"return", Keyword::RETURN}, {"fn", Keyword::FN}, {"class", Keyword::CLASS}, 
    {"import", Keyword::IMPORT}
};

std::unordered_map<std::string, OpName> Lexer::op_loc {
    {"+", OpName::ADD}, {"-", OpName::SUB}, {"*", OpName::MUL}, {"/", OpName::DIV}, {"%", OpName::MOD}, {"^", OpName::POW},
    {"++", OpName::INC}, {"--", OpName::DEC}, {"==", OpName::EQ}, {"!=", OpName::NE}, {"<", OpName::LT}, {"<=", OpName::LE}, 
    {">", OpName::GT}, {">=", OpName::GE}, {"and", OpName::AND}, {"or", OpName::OR}, {"xor", OpName::XOR}, {"not", OpName::NOT},
    {"(", OpName::BRAC}, {")", OpName::RBRAC}, {"[", OpName::INDEX}, {"]", OpName::RINDEX}, {"=", OpName::ASN}, {"+=", OpName::ADDASN},
    {"-=", OpName::SUBASN}, {"*=", OpName::MULASN}, {"/=", OpName::DIVASN}, {"%=", OpName::MODASN}, {"^=", OpName::POWASN},
    {".", OpName::MBER}, {"->", OpName::ARROW}, {",", OpName::COMMA}, {":", OpName::COLON}, {"{", OpName::COMP}, {"}", OpName::RCOMP}
};


Lexer::Lexer() : _reader(nullptr), next_get_pos(0), next_look_pos(0) {

}

void Lexer::clear() {
    _reader = nullptr;
    token_buf.clear();
    next_get_pos = next_look_pos = 0;
}

Token Lexer::get_token() {
    if (token_buf.empty()) {
        fetch_token();
    }
    next_get_pos++;
    next_look_pos = next_get_pos;
    Token ret = token_buf.front();
    token_buf.pop_front();
    return ret;
}

Token Lexer::look_ahead() {

    int buf_idx = next_look_pos - next_get_pos;

    if (buf_idx == token_buf.size()) {
        fetch_token();
    }
    else if (buf_idx > token_buf.size()) {
        throw std::out_of_range("Buffer index out of range");
    }
    next_look_pos++;
    return token_buf[buf_idx];
}

void Lexer::go_back() {
    assert(next_look_pos > next_get_pos && "Cannot go back");

    next_look_pos--;
}

void Lexer::fetch_token() {
    token_buf.push_back(_fetch_token());
}

Token Lexer::_fetch_token() {

    if (_reader->eof()) {
        return Token(Token::Type::EOF);
    }

    match(re_ws);

    if (_reader->eof()) {
        return Token(Token::Type::EOF);
    }
    else if (match(re_eol)) {
        return Token(Token::Type::EOL);
    }
    else if (match(re_char)) {

        RawValue::Type vtype = std::regex_match(token_str, re_single_char) ? RawValue::Type::STRING : RawValue::Type::CHAR;

        Token token(Token::Type::VALUE);
        token.set_value(vtype, token_str.substr(1, token_str.length() - 2));
        return token;
    }
    else if (match(re_str)) {
        Token token(Token::Type::VALUE);
        token.set_value(RawValue::Type::STRING, token_str.substr(1, token_str.length() - 2));
        return token;
    }
    else if (match(re_op)) {
        return Token(Token::Type::OP, static_cast<unsigned>(op_loc.find(token_str)->second));
    }
    else if (match(re_float)) {
        Token token(Token::Type::VALUE);
        token.set_value(RawValue::Type::FLOAT, token_str);
        return token;
    }
    else if (match(re_int)) {
        Token token(Token::Type::VALUE);
        token.set_value(RawValue::Type::INT, token_str);
        return token;
    }
    else if (match(re_id)) {

        if (token_str == "true" || token_str == "false") {
            Token token(Token::Type::VALUE);
            token.set_value(RawValue::Type::BOOL, token_str);
            return token;
        }

        auto find_op_result = op_loc.find(token_str);
        if (find_op_result != op_loc.end()) {
            return Token(Token::Token::OP, static_cast<unsigned>(find_op_result->second));
        }

        auto find_result = keyword_loc.find(token_str);
        if (find_result == keyword_loc.end()) {
            return Token(Token::Type::ID, token_str);
        }
        else if (find_result->second >= Keyword::FN) {
            return Token(Token::Type::DECL, static_cast<unsigned>(find_result->second));
        }
        else {
            return Token(Token::Type::CTRL, static_cast<unsigned>(find_result->second));
        }
    }

    else {
        throw SyntaxError("Unrecognized token");
        return Token();
    }

}

bool Lexer::match(const std::regex& re) {
    std::smatch match_result;
    if (std::regex_search(_reader->iter(), _reader->end(), match_result, re, std::regex_constants::match_continuous)) {
        token_str = match_result[0].str();
        _reader->forward(match_result[0].length());
        return true;
    }
    else {
        return false;
    }
}