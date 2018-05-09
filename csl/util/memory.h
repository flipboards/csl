#pragma once

#ifndef CSL_UTIL_MEMORYPOOL_H
#define CSL_UTIL_MEMORYPOOL_H

#include <list>
#include <string>

struct MemoryBlock {
    void* ptr;
    int ref;

    MemoryBlock() : ptr(nullptr), ref(0) {

    }

    explicit MemoryBlock(void* ptr) : ptr(ptr), ref(0) {

    }
};


/* A const reference to a MemoryBlock. Using a reference-count to check reference. */
/* If the reference is <=0; The pointer will not be destoryed, but exists() will return false. */
template<typename Ty>
class ConstMemoryRef {
public:
    typedef typename ConstMemoryRef<Ty> _Myt;
    typedef typename const Ty* pointer;

    ConstMemoryRef() : _p(nullptr) {

    }

    ConstMemoryRef(std::nullptr_t) : _p(nullptr) {

    }

    ConstMemoryRef(const _Myt& other) : _p(other._p) {
        if (_p) _p->ref++;
    }

    ~ConstMemoryRef() {
        if (_p) _p->ref--;
    }

    _Myt& operator=(const _Myt& other) {
        if (_p) _p->ref--;
        _p = other._p;
        if (_p) _p->ref++;
        return *this;
    }

    _Myt& operator=(std::nullptr_t) {
        if (_p) _p->ref--;
        _p = nullptr;
    }

    const Ty& operator*()const {
        return *(static_cast<Ty*>(_p->ptr));
    }

    const Ty* operator->()const {
        return static_cast<Ty*>(_p->ptr);
    }

    bool operator!()const {
        return _p == nullptr || _p->ref <= 0;
    }

    bool exists()const {
        return _p != nullptr && _p->ref > 0;
    }

    const Ty* get()const {
        return static_cast<Ty*>(_p->ptr);
    }

    int use_count()const {
        return _p->ref;
    }

    /* Static cast to other pointer */
    template<typename NTy>
    ConstMemoryRef<NTy> cast()const {
        return ConstMemoryRef<NTy>::_build(_p);
    }

    /* Build a new ConstMemRef from a pointer. Do not use directly*/
    static _Myt _build(MemoryBlock* p) {
        _Myt m;
        m._p = p;
        m._p->ref++;
        return m;
    }

protected:
    MemoryBlock * _p;
};


/* A multable reference to a pointer */
template<typename Ty>
class MemoryRef : public ConstMemoryRef<Ty> {
public:
    typedef typename MemoryRef<Ty> _Myt;
    typedef typename Ty* pointer;

    MemoryRef() {

    }

    MemoryRef(const _Myt& other) : ConstMemoryRef<Ty>(other) {

    }

    _Myt& operator=(const _Myt& other) {
        if (this->_p) this->_p->ref--;
        this->_p = other._p;
        if (this->_p) this->_p->ref++;
        return *this;
    }

    Ty& operator*(){
        return *(static_cast<Ty*>(this->_p->ptr));
    }

    Ty* operator->(){
        return static_cast<Ty*>(this->_p->ptr);
    }

    Ty* get() {
        return static_cast<Ty*>(this->_p->ptr);
    }

    ConstMemoryRef<Ty> to_const()const {
        return ConstMemoryRef<Ty>::_build(this->_p);
    }

    /* Static cast to other pointer */
    template<typename NTy>
    MemoryRef<NTy> cast()const {
        return MemoryRef<NTy>::_build(this->_p);
    }

    /* Build a new ConstMemRef from a pointer. Do not use directly*/
    static _Myt _build(MemoryBlock* p) {
        _Myt m;
        m._p = p;
        m._p->ref++;
        return m;
    }
};


class MemoryPool {
public:

    MemoryPool() {

    } 

    ~MemoryPool() {
        for (auto iter = _mylist.begin(); iter != _mylist.end(); ++iter) {
            delete iter->ptr;
        }
    }

    /* Allocate a new object; Requires constructor */
    template<typename Ty>
    MemoryRef<Ty> allocate() {
        Ty* newptr = new Ty();
        _mylist.push_front(MemoryBlock(newptr));
        return MemoryRef<Ty>::_build(&_mylist.front());
    }

    /* Copy an exist object. Requires the copy constructor */
    template<typename Ty>
    MemoryRef<Ty> assign(const Ty& t) {
        Ty* newptr = new Ty(t);
        _mylist.push_front(MemoryBlock(newptr));
        return MemoryRef<Ty>::_build(&_mylist.front());
    }

    /* Collect an exist opinter. The pointer must be on HEAP using *NEW*.
    Do not *FREE* the original pointer */
    template<typename Ty>
    MemoryRef<Ty> collect(Ty* t) {
        if (t == nullptr) {
            return MemoryRef<Ty>();
        }
        _mylist.push_front(MemoryBlock(t));
        return MemoryRef<Ty>::_build(&_mylist.front());
    }


private:

    std::list<MemoryBlock> _mylist;
};


class StringRef : public ConstMemoryRef<char> {
public:

    typedef StringRef _Myt;

    StringRef() {

    }

    StringRef(const _Myt& other) : ConstMemoryRef<char>(other) {
    }

    _Myt& operator=(const _Myt& other) {
        if (_p) _p->ref--;
        _p = other._p;
        if (_p) _p->ref++;
        return *this;
    }

    const char* to_cstr()const {
        return static_cast<char*>(_p->ptr);
    }

    std::string to_string()const {
        return std::string(to_cstr());
    }

    static StringRef null() {
        return StringRef();
    }

    bool operator<(const std::string& str)const {
        return strcmp(to_cstr(), str.c_str()) < 0;
    }

    /* String-like behavior */

    size_t length()const {
        return strlen(to_cstr());
    }

    bool operator==(const std::string& str)const {
        return strcmp(to_cstr(), str.c_str()) == 0;
    }

    bool operator==(const char* str)const {
        return strcmp(to_cstr(), str) == 0;
    }

    std::string operator+(const std::string& str)const {
        return to_string() + str;
    }

    friend static std::string operator+(const std::string& lhs, const StringRef& rhs) {
        return lhs + rhs.to_string();
    }

    static _Myt _build(MemoryBlock* p) {
        _Myt m;
        m._p = p;
        m._p->ref++;
        return m;
    }
};


class ConstStringPool {
public:

    ConstStringPool() {

    }

    ~ConstStringPool() {
        for (auto& p : _mylist) {
            free(p.ptr);
        }
    }

    StringRef assign(const char* str) {
        char* newstr = static_cast<char*>(malloc(strlen(str) + 1));
        memcpy(newstr, str, strlen(str));
        _mylist.push_front(MemoryBlock(newstr));
        return StringRef::_build(&_mylist.front());
    }

    StringRef assign(const std::string& str) {
        return assign(str.c_str(), str.c_str() + str.length());
    }

    StringRef assign(const char* strbegin, const char* strend) {
        
        char* newstr = static_cast<char*>(malloc(strend - strbegin + 1));
        memcpy(newstr, strbegin, strend - strbegin);
        newstr[strend - strbegin] = '\0';
        _mylist.push_front(MemoryBlock(newstr));
        return StringRef::_build(&_mylist.front());
    }

private:

    std::list<MemoryBlock> _mylist;
};

#endif