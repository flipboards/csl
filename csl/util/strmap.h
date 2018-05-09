
#ifndef STRMAP_H
#define STRMAP_H

#include <map>
#include <set>
#include <list>
#include <string>

#define Min(a, b) (a < b ? (a) : (b))

/* Unstable reference to a string to avoid copy; Usually only for lookup */
class StringTmpRef {
public:

    StringTmpRef() : begin(nullptr), end(nullptr) {

    }

    explicit StringTmpRef(const char* str) : begin(str), end(str + strlen(str)) {

    }

    explicit StringTmpRef(const std::string& str) : begin(&str[0]), end(&str[0] + str.length()) {

    }

    explicit StringTmpRef(const char* begin, const char* end) : begin(begin), end(end) {

    }

    size_t length()const {
        return end - begin;
    }

    const char* get()const {
        return begin;
    }

    bool operator==(const StringTmpRef& other)const {
        return length() == other.length() && strncmp(begin, other.begin, length()) == 0;
    }

    bool operator==(const char* other)const {
        return length() == strlen(other) && strncmp(begin, other, length()) == 0;
    }

    bool operator==(const std::string& other)const {
        return length() == other.length() && strncmp(begin, &other[0], length()) == 0;
    }

    bool operator!=(const StringTmpRef& other)const {
        return length() != other.length() || strncmp(begin, other.begin, length()) != 0;
    }

    bool operator<(const StringTmpRef& other)const {
        int ret = strncmp(begin, other.begin, Min(length(), other.length()));
        
        return ret != 0 ? ret < 0 : length() < other.length();
    }

    std::string copy()const {
        return std::string(begin, end);
    }

    std::string substr(unsigned a, unsigned b)const {
        if (a > length())throw std::out_of_range("Index out of range");
        
        return std::string(begin + a, begin + Min(a + b, length()));
    }

private:

    const char* begin;
    const char* end;
};


/* Implementation of std::map on StringTmpRef */
template<typename Ty>
class StrMap {
public:

    typedef typename std::map<StringTmpRef, Ty> _MyMap;
    typedef typename _MyMap::const_iterator const_iterator;
    typedef typename _MyMap::iterator iterator;

    StrMap() {

    }

    StrMap(const std::initializer_list<std::pair<std::string, Ty> >& init_list) {
        for (const auto& p : init_list) {
            _strkeys.push_front(p.first);
            _map.insert({ StringTmpRef(_strkeys.front()), p.second });
        }
    }

    const_iterator begin()const {
        return _map.begin();
    }

    const_iterator end()const {
        return _map.end();
    }

    const_iterator find(const StringTmpRef& str)const {
        return _map.find(str);
    }

    const_iterator find(const std::string& str)const {
        return find(StringTmpRef(str));
    }

    const_iterator find(const char* str)const {
        return find(StringTmpRef(str));
    }

    bool has_key(const StringTmpRef& str)const {
        return _map.find(str) != _map.end();
    }

    std::pair<iterator, bool> insert(const std::pair<std::string, Ty>& p) {
        _strkeys.push_front(p.first);
        auto insert_ret = _map.insert({StringTmpRef(_strkeys.front()), p.second });
        if (!insert_ret.second) { // insert unsuccessful
            _strkeys.pop_front();   // undo local copy
        }
        return insert_ret;
    }

    const Ty& at(const StringTmpRef& str)const {
        return _map.at(str);
    }

    const Ty& at(const char* str)const {
        return _map.at(StringTmpRef(str));
    }

    const Ty& at(const std::string& str)const {
        return _map.at(StringTmpRef(str));
    }

private:

    std::list<std::string> _strkeys;
    _MyMap _map;
};



class StrSet {
public:

    typedef std::set<StringTmpRef> _MySet;
    typedef _MySet::const_iterator const_iterator;
    typedef _MySet::iterator iterator;

    StrSet() {

    }

    StrSet(const std::initializer_list<std::string>& init_list) {
        for (const auto& p : init_list) {
            _strkeys.push_front(p);
            _set.insert(StringTmpRef(_strkeys.front()));
        }
    }

    const_iterator begin()const {
        return _set.begin();
    }

    const_iterator end()const {
        return _set.end();
    }

    const_iterator find(const StringTmpRef& str)const {
        return _set.find(str);
    }

    const_iterator find(const std::string& str)const {
        return _set.find(StringTmpRef(str));
    }

    const_iterator find(const char* str)const {
        return _set.find(StringTmpRef(str));
    }

    bool has_key(const StringTmpRef& str)const {
        return _set.find(str) != _set.end();
    }

    bool has_key(const char* str)const {
        return has_key(StringTmpRef(str));
    }

    bool has_key(const std::string& str)const {
        return has_key(StringTmpRef(str));
    }

    std::pair<iterator, bool> insert(const std::string& str) {
        _strkeys.push_front(str);
        return _set.insert(StringTmpRef(_strkeys.front()));
    }

private:

    std::list<std::string> _strkeys;
    _MySet _set;
};


#undef Min
#endif