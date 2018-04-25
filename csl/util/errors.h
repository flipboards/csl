#pragma once

#ifndef CSL_UTIL_ERRORS_H
#define CSL_UTIL_ERRORS_H

#include <stdexcept>

class CSLError : public std::runtime_error {
public:
    
    enum ErrorID {
        NONE,
        SYNTAX,
        TRANSLATE
    };

    CSLError(const char* msg) : std::runtime_error(msg) {

    }

    CSLError(const std::string& msg) : std::runtime_error(msg) {

    }

    virtual ErrorID get_id()const {
        return NONE;
    }
};


class SyntaxError : public CSLError {
public:

    using CSLError::CSLError;

    ErrorID get_id()const {
        return SYNTAX;
    }

};

#endif