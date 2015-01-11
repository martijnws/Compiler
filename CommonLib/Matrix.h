#pragma once

#include <algorithm>

namespace mws { namespace common {

template<typename T>
class Matrix
{
public:
    class Row
    {
        friend class Matrix;

        Row(T* elements_)
        :
            _elements(elements_)
        {
        
        }

    public:
        T& operator [](std::size_t c_)
        {
            return _elements[c_];
        }

        const T& operator [](std::size_t c_) const
        {
            return _elements[c_];
        }

    private:
        T* _elements;
    };

    Matrix() 
    : 
        _elements(NULL)
    {
   
    }

    Matrix(std::size_t cr_, std::size_t cc_, const T& v_) 
    : 
        _elements(new T[cr_ * cc_]), _cr(cr_), _cc(cc_)
    {
        std::fill(_elements, _elements + cr_ * cc_, v_);
    }

    void init(std::size_t cr_, std::size_t cc_, const T& v_)
    {
        _elements = new T[cr_ * cc_]; _cr = cr_; _cc = cc_;

        std::fill(_elements, _elements + cr_ * cc_, v_);
    }

    Row operator [](std::size_t r_) const
    {
        assert(r_ < _cr);
        return Row(&_elements[r_ * _cc]);
    }

    std::size_t rowCount() const
    {
        return _cr;
    }

    std::size_t colCount() const
    {
        return _cc;
    }

private:
    T* _elements;
    std::size_t _cr;
    std::size_t _cc;
};

}}
