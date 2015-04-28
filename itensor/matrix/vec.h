//
// Distributed under the ITensor Library License, Version 1.2.
//    (See accompanying LICENSE file.)
//
#ifndef __ITENSOR_VEC_H_
#define __ITENSOR_VEC_H_

#include "types.h"
#include "print.h"
#include "strideiter.h"

namespace itensor {

template<typename T>
class Vector;

template<typename T>
class VectorRef;

using Vec = Vector<Real>;
using VecRef = VectorRef<Real>;
using VecRefc = VectorRef<const Real>;

//using CVec = Vector<Complex>;
//using CVecRef = VectorRef<Complex>;
//using CVecRefc = VectorRef<const Complex>;

template<typename T>
class VectorRef
    {
    public:
    using base_type = T;
    using iterator = stride_iter<T*>;
    using const_iterator = stride_iter<const T*>;
    using value_type = std::remove_const_t<T>;
    using pointer = std::add_pointer_t<T>;
    using reference = std::add_lvalue_reference_t<T>;
    using size_type = long;
    using vec_type = std::conditional_t<std::is_const<T>::value,
                                        const Vector<value_type>,
                                        Vector<value_type>>;
    private:
    pointer pdata_ = nullptr;
    size_type strd_ = 1;
    size_type size_ = 0;
    public:

    VectorRef() { }

    VectorRef(pointer pdata, 
              long size,
              long stride = 1)
        :
        pdata_(pdata),
        strd_(stride),
        size_(size)
        { }

    VectorRef(pointer pdata,
              long offset,
              long size,
              long stride)
        : VectorRef(pdata+offset,size,stride)
        { }

    VectorRef(vec_type& v) { assignFromVec(v); }
    
    VectorRef&
    operator=(vec_type& v) { assignFromVec(v); return *this; }

    VectorRef(vec_type&& v) = delete;

    VectorRef&
    operator=(vec_type&& v) = delete;

    operator VectorRef<const value_type>() const { return VectorRef<const T>(pdata_,size_,strd_); }

    explicit operator bool() const { return bool(pdata_); }

    size_type
    size() const { return size_; }

    size_type
    stride() const { return strd_; }

    bool
    contiguous() const { return strd_ == 1; }

    reference
    operator()(long i) const { return pdata_[(i-1)*strd_]; }

    iterator 
    begin() const { return iterator(pdata_,strd_); }

    iterator 
    end() const { return iterator(pdata_+size_*strd_,strd_); }

    const_iterator 
    cbegin() const { return const_iterator(pdata_,strd_); }

    const_iterator 
    cend() const { return const_iterator(pdata_+size_*strd_,strd_); }

    pointer
    data() const { return pdata_; }

    void
    reset()
        {
        pdata_ = nullptr;
        strd_ = 1;
        size_ = 0;
        }

    private:
    void
    assignFromVec(vec_type& v);
    };

VecRef
operator*=(VecRef v, Real fac);

VecRef
operator/=(VecRef v, Real fac);

VecRef
operator+=(VecRef a, VecRefc b);

VecRef
operator-=(VecRef a, VecRefc b);

//Copy data referenced by b to memory referenced by a
VecRef
operator&=(VecRef a, VecRefc b);

//Dot product
Real
operator*(VecRefc a, VecRefc b);

template<typename T>
class Vector
    {
    public:
    using storage_type = std::vector<T>;
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;
    using value_type = std::remove_const_t<T>;
    using pointer = std::add_pointer_t<T>;
    using reference = std::add_lvalue_reference_t<T>;
    using size_type = typename storage_type::size_type;
    public:
    storage_type data_;
    public:

    Vector() { }

    explicit
    Vector(long size) : data_(size) { }

    Vector(const Vector& other) { assignFromVec(other); }

    Vector(Vector&& other) { moveFromVec(std::move(other)); }

    explicit
    Vector(VecRefc ref) { assignFromRef(ref); }

    Vector&
    operator=(const Vector& other) { assignFromVec(other); return *this; }

    Vector& 
    operator=(Vector&& other) { moveFromVec(std::move(other)); return *this; }

    Vector&
    operator=(VecRefc ref) { assignFromRef(ref); return *this; }

    explicit operator bool() const { return !data_.empty(); }

    size_type
    size() const { return data_.size(); }

    reference
    operator()(long i) 
        { 
#ifdef DEBUG
        return data_.at(i-1); 
#else
        return data_[i-1]; 
#endif
        }

    value_type
    operator()(long i) const 
        { 
#ifdef DEBUG
        return data_.at(i-1); 
#else
        return data_[i-1]; 
#endif
        }

    Vector&
    operator*=(Real fac);

    Vector&
    operator/=(Real fac);

    Vector&
    operator+=(const Vector& other);

    Vector&
    operator-=(const Vector& other);

    Vector&
    operator+=(VectorRef<const T> other);

    Vector&
    operator-=(VectorRef<const T> other);

    iterator
    begin() { return data_.begin(); }

    iterator
    end() { return data_.end(); }

    const_iterator
    begin() const { return data_.cbegin(); }

    const_iterator
    end() const { return data_.cend(); }

    const_iterator
    cbegin() const { return data_.cbegin(); }

    const_iterator
    cend() const { return data_.cend(); }

    T*
    data() { return data_.data(); }

    const T*
    data() const { return data_.data(); }

    void
    clear() { data_.clear(); }

    private:

    void
    assignFromRef(VecRefc other)
        {
        //Copy data from other contiguously into data_
        data_ = storage_type(other.cbegin(),other.cend());
        }

    void
    assignFromVec(const Vector& other)
        {
        if(&other == this) return;
        data_ = other.data_;
        }

    void
    moveFromVec(Vector&& other)
        {
        data_ = std::move(other.data_);
        other.clear();
        }

    };

template<typename T>
void VectorRef<T>::
assignFromVec(vec_type& v)
    {
    pdata_ = v.data();
    size_ = v.size();
    strd_ = 1;
    }

//
// makeRef functions
//

template<typename T>
auto
makeRef(VectorRef<T>& v) { return v; }

template<typename T>
auto
makeRef(VectorRef<const T>& v) { return v; }

template<typename T>
auto
makeRef(Vector<T>& v) { return VectorRef<T>(v); }

template<typename T>
auto
makeRef(const Vector<T>& v) { return VectorRef<const T>(v); }

template<typename T, typename Arg, typename... Rest>
auto
makeRef(VectorRef<T>& v, Arg&& arg, Rest&&... args) 
    { return VectorRef<T>(v.data(),std::forward<Arg>(arg),std::forward<Rest>(args)...); }

template<typename T, typename Arg, typename... Rest>
auto
makeRef(VectorRef<const T>& v, Arg&& arg, Rest&&... args) 
    { return VectorRef<const T>(v.data(),std::forward<Arg>(arg),std::forward<Rest>(args)...); }

template<typename T, typename Arg, typename... Rest>
auto
makeRef(Vector<T>& v, Arg&& arg, Rest&&... args) 
    { return VectorRef<T>(v.data(),std::forward<Arg>(arg),std::forward<Rest>(args)...); }

template<typename T, typename Arg, typename... Rest>
auto
makeRef(const Vector<T>& v, Arg&& arg, Rest&&... args) 
    { return VectorRef<const T>(v.data(),std::forward<Arg>(arg),std::forward<Rest>(args)...); }


//This version of makeRef intended to fail,
//forbids explicitly making VectorRef's to temporaries
template<typename T, typename... Rest>
auto
makeRef(Vector<T>&& v, Rest&&... args) { return VectorRef<T>(std::move(v)); }


//
// Finish defining Vector operators
//

template<typename T>
Vector<T>& Vector<T>::
operator*=(Real fac) { makeRef(*this) *= fac; return *this; }

template<typename T>
Vector<T>& Vector<T>::
operator/=(Real fac) { makeRef(*this) /= fac; return *this; }

template<typename T>
Vector<T>& Vector<T>::
operator+=(const Vector<T>& other) { makeRef(*this) += makeRef(other); return *this; }

template<typename T>
Vector<T>& Vector<T>::
operator-=(const Vector<T>& other) { makeRef(*this) -= makeRef(other); return *this; }

template<typename T>
Vector<T>& Vector<T>::
operator+=(VectorRef<const T> other) {  makeRef(*this) += other; return *this; }

template<typename T>
Vector<T>& Vector<T>::
operator-=(VectorRef<const T> other) { makeRef(*this) -= other; return *this; }

Vec inline
operator+(VecRefc A, VecRefc B)
    { 
    Vec res(A);
    res += B; 
    return res; 
    }

Vec inline
operator+(VecRefc A, Vec&& B) 
    { 
    Vec res(std::move(B)); 
    res += A; 
    return res; 
    }

Vec inline
operator+(Vec&& A, VecRefc B) 
    { 
    Vec res(std::move(A)); 
    res += B; 
    return res; 
    }

Vec inline
operator-(VecRefc A, VecRefc B)
    { 
    Vec res(A);
    res -= B; 
    return res; 
    }

Vec inline
operator-(VecRefc A, Vec&& B) 
    { 
    Vec res(std::move(B)); 
    res *= -1;
    res += A; 
    return res; 
    }

Vec inline
operator-(Vec&& A, VecRefc B) 
    { 
    Vec res(std::move(A)); 
    res -= B; 
    return res; 
    }

Real
norm(VecRefc v);

VecRef
randomize(VecRef v);

Vec
randomVec(long size);

std::ostream&
operator<<(std::ostream& s, VecRefc v);

inline std::ostream&
operator<<(std::ostream& s, const Vec& v) { return operator<<(s,makeRef(v)); }


//
// Vector slicing operations
//

//Return ref to elements [start,stop], inclusive, of a vector
template<typename Vec_>
auto
subVector(Vec_&& v,
          long start,
          long stop)
    {
    return makeRef(std::forward<Vec_>(v),start-1,stop-start+1,1);
    }

};

#endif
