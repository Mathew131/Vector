#pragma once
#include <memory>
#include <tracer/tracer.h>
#include <stdexcept>
#include <iostream>

template <typename T, typename A>
struct vector_base {
    A alloc;
    int sz;
    T* elem;
    int space;

    vector_base(const A& a, int n) : alloc{a}, elem{alloc.allocate(n)}, sz{n}, space{n} {}
    ~vector_base() { alloc.deallocate(elem, space); }
};

template <typename T, typename A = std::allocator<T>> class vector {
    A alloc;
    int sz;
    T* elem;
    int space;

public:
    static int count;

    vector() : sz{0}, elem{nullptr}, space{0} { 
        TRACE_FUNC; 
        count++;
        std::cout << "count: " << count << '\n'; 
    }

    explicit vector(int s, T val = T()) : sz{s}, elem{alloc.allocate(s)}, space{s} {
        TRACE_FUNC;
        count++;
        std::cout << "count: " << count << '\n';
        for (int i = 0; i<s; ++i) elem[i] = val;
    }

    vector(std::initializer_list<T> lst) : sz{lst.size()}, elem{alloc.allocate(lst.size())}, space{lst.size()} {
        TRACE_FUNC;
        count++;
        std::cout << "count: " << count << '\n';
        std::copy(lst.begin(), lst.end(), elem);
    }

    vector(const vector<T>& arg) : sz{arg.size()}, elem{alloc.allocate(arg.sz)} {
        TRACE_FUNC;
        count++;
        std::cout << "count: " << count << '\n';
        std::copy(arg.elem, arg.elem+sz, elem);
    }

    vector(vector<T>&& a) : sz{a.sz}, elem{a.elem} { 
        // Перемещающий конструктор
        TRACE_FUNC;
        count++;
        std::cout << "count: " << count << '\n';
        a.sz = 0;
        a.elem = nullptr;
    }

    vector<T>& operator=(vector<T>& a);

    vector<T>& operator=(vector<T>&& a); // перемещающее копирование

    ~vector() { 
        TRACE_FUNC;
        for (int i = 0; i<sz; ++i) alloc.destroy(elem+i);
        count--;
        std::cout << "count: " << count << '\n';
    }

    T& at(int n);

    T& operator[](int n) { return elem[n]; }

    int size() const { return sz; }
    int capacity() const { return space; }

    void resize(int newsize, T val = T());
    void push_back(const T& val);
    void reserve(int newalloc);
};

template <typename T, typename A> T& vector <T, A>::at(int n) {
    if (n < 0 || sz <= n) throw std::out_of_range("Error");
    return elem[n];
}

template <typename T, typename A> 
vector<T>& vector<T, A>::operator=(vector<T>& a) {
    TRACE_FUNC;

    if (this == &a) return *this;

    if (a.sz <= space) {
        for (int i = 0; i<a.sz; ++i) elem[i] = a.elem[i];
        sz = a.sz;
        return *this;
    }

    T* p = alloc.allocate(a.sz);
    for (int i = 0; i<a.sz; ++i) p[i] = a.elem[i];
    alloc.deallocate(a.elem, a.sz);

    space = sz = a.sz;
    elem = p;
    return *this;
}

template <typename T, typename A> 
vector<T>& vector<T, A>::operator=(vector<T>&& a) {
    TRACE_FUNC; // перемещающее копирование

    alloc.deallocate(elem, sz);
    elem = a.elem;
    
    sz = a.sz;
    a.elem = nullptr;
    a.sz = 0;
    return *this;
}

template <typename T, typename A> 
void vector<T, A>::reserve(int newalloc) {
    if (newalloc <= this->space) return;
    vector_base <T, A> b(this->alloc, newalloc);

    std::uninitialized_copy(this->elem, this->elem+sz, b.elem);
    for (int i = 0; i<this->sz; ++i) {
        this->alloc.destroy(&this->elem[i]);
    }
    
    this->space = newalloc;
    std::swap(this->elem, b.elem);
}

template <typename T, typename A> 
void vector<T, A>::resize(int newsize, T val) {
    reserve(newsize);

    for (int i = sz; i<newsize; ++i) alloc.construct(&elem[i], val); 

    for (int i = newsize; i<sz; ++i) alloc.destroy(&elem[i]);
    sz = newsize;
}

template <typename T, typename A> 
void vector<T, A>::push_back(const T& val) {

    if (space == 0) {
        reserve(8);

    }
    else if (sz == space) {
        reserve(2*space);
    }
    alloc.construct(&elem[sz], val);
    ++sz;
}
template <typename T, typename A> int vector<T, A>::count = 0;