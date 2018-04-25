#pragma once

#ifndef CSL_UTIL_IOUTIL_H
#define CSL_UTIL_IOUTIL_H

#include <string>


class StrReader {
public:

    StrReader(const std::string& s): _buffer(s), _iter(_buffer.begin()) {

    }

    std::string::const_iterator iter()const {
        return _iter;
    }

    std::string::const_iterator end()const {
        return _buffer.end();
    }

    int pos()const {
        return _iter - _buffer.begin();
    }

    void forward(unsigned step) {
        _iter += step;
        if (_iter >= _buffer.end()) {
            _iter = _buffer.end();
        }
    }

    void backward(unsigned step) {
        _iter -= step;
        if (_iter < _buffer.begin()) {
            _iter = _buffer.begin();
        }
    }

    bool eof()const {
        return _iter >= _buffer.end();
    }

    // This is not efficient for large text!
    // lineno == 0 for first line.
    unsigned lineno()const {
        unsigned ln = 0;
        for (auto iter = _iter; iter > _buffer.begin(); --iter) {
            if (*iter == '\n')ln++;
        }
        return ln;
    }

    std::string::const_iterator cur_line_begin()const {
        auto iter = _iter;
        for (; iter > _buffer.begin(); --iter) {
            if (*iter == '\n')return ++iter;
        }
        return iter;
    }

    std::string::const_iterator cur_line_end()const {
        auto iter = _iter;
        for (; iter < _buffer.end(); ++iter) {
            if (*iter == '\n')return iter;
        }
        return iter;
    }

private:

    std::string _buffer;
    std::string::const_iterator _iter;
};

#endif