#pragma once
#include "stdafx.h"

template<typename TypeToClone>
concept Cloneable = requires(TypeToClone obj) { obj.clone(); };

template <Cloneable T>
class PtrArray
{
public:
    class Wrapper;
    class Iterator;

    using value_type = Wrapper;
    using reference = value_type&;
private:
    size_t _length;
    size_t _capacity;
    Wrapper* _array;
        
    class Wrapper
    {
    private:
        using value_type = T*;
        using reference = value_type&;
        using pointer = value_type*; 
        
        pointer _wrapped;//Pointer at objects pointer
    public:
        Wrapper() noexcept
            : _wrapped(new value_type{ nullptr }) { }

        explicit Wrapper(const value_type m_ptr) noexcept
            : _wrapped(&m_ptr) { }

        explicit Wrapper(value_type&& m_ptr) noexcept
        {
            this->operator=(std::forward<value_type>(m_ptr));
        }

        reference operator=(const value_type other)
        {
            if (*this->_wrapped)
                delete* this->_wrapped;

            this->_wrapped = other->clone();
        }

        //Move m_ptr to wrapped
        reference operator=(value_type&& m_ptr)
        {
            if (this->_wrapped)
            {
                if (*this->_wrapped)
                    delete* this->_wrapped;

                *this->_wrapped = m_ptr;
                m_ptr = nullptr;
            }

            return *this;
        }

        explicit(false) operator T* ()
        {
            return *this->_wrapped;
        }
    };

    class Iterator
    {
    private:
        T* m_ptr;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Wrapper&;
        using pointer = T*;
        using reference = pointer&;

        explicit Iterator(pointer m_ptr) noexcept : m_ptr(m_ptr) { };

        Iterator(Iterator const& other) noexcept : m_ptr(other.m_ptr) { }

        value_type operator*() const { return m_ptr; }

        pointer operator->() const { return m_ptr; }

        Iterator& operator++() { ++m_ptr;  return *this; }

        Iterator operator++(int) { auto temp = *this; ++m_ptr;  return temp; }

        Iterator& operator--() { --m_ptr;  return *this; }

        Iterator operator--(int) { auto temp = *this; --m_ptr;  return temp; }

        Iterator operator+(difference_type n) const { return Iterator(m_ptr + n); }

        Iterator operator-(difference_type n) const { return Iterator(m_ptr - n); }

        Iterator& operator=(Iterator const& other) { this->m_ptr = other.m_ptr; return *this; }

        difference_type operator-(Iterator const& rhs) const { return m_ptr - rhs.m_ptr; }

        std::strong_ordering operator<=>(Iterator const& rhs) const = default;
    };

    //Fields
    size_t _length = 0;
    size_t _capacity = _init_capacity;
    Wrapper* _array = nullptr;

    //Private methods
    void _change_Array(Wrapper* array, size_t length, size_t capacity)
    {
        this->_array = array;
        this->_capacity = capacity;
        this->_length = length;
    }

    //Delete all the pointers and set _array to nullptr
    void _deallocate()
    {
        //Clear all the data pointers point at
        for (size_t i = 0; i < this->_length; ++i)
            delete this->_array[i];
        delete[] this->_array;

        //Set pointer to nullptr to avoid unexpected behaviour
        this->_array = nullptr;
    }

    //Check whether _array is full of elements
    bool _isFull() const
    {
        return this->_length >= this->_capacity;
    }

    //Allocate array of nullptrs if _new_Capacity > 0 otherwise _array = nullptr, set new capacity
    void _allocate(size_t _new_Capacity)
    {
        this->_array = _new_Capacity > 0 ?
            new Wrapper[_new_Capacity]
            : nullptr;

        this->_capacity = _new_Capacity;
    }

    //Double capacity -> move to new array -> assign
    void _expandAndMove()
    {
        size_t newCapacity = (this->_capacity + 1) << 1;

        auto newArray = new Wrapper[newCapacity];
        std::move(this->begin(), this->end(), newArray);

        delete[] _array;
        this->_array = newArray;
        this->_capacity = newCapacity;
    }

    template<typename U>
    void _emplaceChecked(U&& obj, Iterator iter)
    {
        *iter = obj;

        ++this->_length;
    }

    const size_t _init_capacity = 10;
public:
    //Constructors
    PtrArray()
    {
        this->_allocate(this->_capacity);
    }

    PtrArray(PtrArray<T> const& other)
    {
        this->operator=(other);
    }

    PtrArray(PtrArray<T>&& other) noexcept
    {
        this->operator=(std::move(other));
    }

    //Copy and assignment operators
    PtrArray<T>& operator=(PtrArray<T> const& other)
    {
        this->_deallocate();
        if (!other.empty())
        {
            this->_allocate(other._capacity);
            std::copy(other.begin(), other.begin() + other._capacity, this->begin());
        }
        else this->_array = nullptr;

        this->_length = other._length;
        this->_capacity = other._capacity;
        return *this;
    }

    PtrArray<T>& operator=(PtrArray<T>&& other) noexcept
    {
        //Move data from other to current object
        this->_deallocate();
        this->_change_Array(other._array, other._length, other._capacity);

        //Clear the other
        other._change_Array(nullptr, 0, 0);
        return *this;
    }

    ~PtrArray()
    {
        this->_deallocate();
    }

    //Modifiers
    template <typename U>
    void emplace(Iterator position, U&& obj)
    {
        size_t index = position - this->begin();
        if (this->_isFull())
        {
            this->_expandAndMove();
            position = this->begin() + index;
        }

        // Move elements to make space for the new element
        std::move_backward(position, this->end(), position + 2);

        this->_emplaceChecked(std::forward<U>(obj), position);
    }

    template<typename U>
    void emplace_back(U&& obj)
    {
        this->emplace(this->end(), std::forward<U>(obj));
    }

    template<typename U>
    void push_back(U&& obj)
    {
        this->emplace(this->end(), std::forward<U>(obj));
    }

    //Erase elements at [first, last)
    void erase(Iterator _First, Iterator _Last)
    {
        if (this->empty() || _First < this->begin() || _Last > this->end() || _First >= _Last)
            return;

        auto _dist = std::distance(_First, _Last);

        std::move(_Last, this->end(), _First);
        for (decltype(_dist) i = 0; i < _dist; ++i)
            *(this->end() - i - 1) = nullptr;

        this->_length -= _dist;
    }

    void erase(Iterator position)
    {
        this->erase(position, position + 1);
    }

    //Capacity
    size_t size() const noexcept
    {
        return this->_length;
    }

    void clear()
    {
        this->_deallocate();
        this->_length = 0;
        this->_capacity = _init_capacity;
    }

    bool empty() const noexcept
    {
        return !this->_length;
    }

    T const& at(const size_t index) const noexcept(false)
    {
        if (index >= this->_length)
            throw std::out_of_range("Index of the array is out of the range");

        return *this->_array[index];
    }

    T* operator[](const size_t index) const noexcept
    {
        if (index >= this->_length)
            return this->_array[0];

        return this->_array[index];
    }

    Iterator begin() const
    {
        return Iterator(this->_array);
    }

    Iterator end() const
    {
        return Iterator(this->_array + this->_length);
    }
};

