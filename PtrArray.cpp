#pragma once
#include "stdafx.h"

template<typename TypeToClone>
concept Cloneable = std::is_pointer_v<TypeToClone> &&
    requires(TypeToClone obj) { obj->clone(); };

template <Cloneable T>
class PtrArray
{
private:
    size_t _length;
    size_t _capacity;
    T* _array;
        
    class Iterator
    {
    private:
        T* m_ptr;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        explicit Iterator(pointer m_ptr) : m_ptr(m_ptr) { };

        Iterator(Iterator const& other) : m_ptr(other.m_ptr) { }

        reference operator*() const { return *m_ptr; }

        pointer operator->() { return m_ptr; }

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

    //Private methods
    void _clearPointers()
    {
        //Clear all the data pointers point at
        for (size_t i = 0; i < this->_length; ++i)
            delete this->_array[i];
        delete[] this->_array;

        //Set pointer to nullptr to avoid unexpected behaviour
        this->_array = nullptr;
    }

    //Perform unchecked cloning from [firstSource, lastSource) to [firstDestionation, ...)
    void _copyRangeUnchecked(Iterator firstSource, Iterator lastSource, Iterator firstDestination) const
    {
        for (; firstSource != lastSource; ++firstSource, ++firstDestination)
            *firstDestination = (*firstSource) ? (*firstSource)->clone() : nullptr;
    }
public:
    //Constructors
    PtrArray() : _length(0), _capacity(0), _array(nullptr) { }

    PtrArray(PtrArray<T> const& other)
        : _length(other._length), _capacity(other._capacity)
    {
        //Allocate new memory for the array if capacity is greater than zero and copy that
        this->_array = this->_capacity > 0 ? new T[this->_capacity] : nullptr;
        this->_copyRangeUnchecked(other.begin(), other.end(), this->begin());
    }

    //Explicitly delete move constructor to avoid any possible errors with memory
    PtrArray(PtrArray<T>&&) = delete;

    ~PtrArray()
    {
        this->_clearPointers();
    }

    //So delete move assignment operator to avoid any errors with memory
    PtrArray<T>& operator=(PtrArray<T>&&) = delete;

    void emplace_back(T const& obj)
    {
        if (this->_length >= this->_capacity)
        {
            // Initialize new capacity
            this->_capacity = (this->_capacity + 1) * 2;
            auto* newArray = new T[this->_capacity];

            //Only if array is not nullptr
            if (this->_array)
            {
                //Copy pointers(!!not data they store)
                for (size_t i = 0; i < this->_capacity; ++i)
                    newArray[i] = i < _length ? _array[i] : nullptr;

                // Delete old array
                delete[] this->_array;
            }
            this->_array = newArray;
        }
        this->_array[this->_length++] = obj->clone();
    }

    void erase(Iterator position)
    {
        //Check if the iterator in array's bounds
        if (this->_length == 0 || position < this->begin() || position >= this->end())
            return;

        //delete data 'under' pointer
        delete* position;

        // Shift elements to the left
        std::copy(position + 1, end(), position);

        // Update the size
        --this->_length;

        // Set the now-unused end element to nullptr to avoid dangling pointer
        *(this->begin() + this->_length) = nullptr;
    }

    size_t size() const
    {
        return this->_length;
    }

    void clear()
    {
        this->_clearPointers();
        this->_length = 0;
        this->_capacity = 0;
    }

    bool empty() const
    {
        return !this->_length;
    }

    T at(const size_t index) const
    {
        if (index >= this->_length)
            throw std::out_of_range("Index of the array is out of the range"); 

        return this->_array[index];
    }

    T& operator[](const size_t index) const
    {
        if (index >= this->_length)
            return this->_array[0];

        return this->_array[index];
    }

    PtrArray<T>& operator=(PtrArray<T> const& other)
    {
        this->_length = other._length;
        this->_capacity = other._capacity;

        this->_clearPointers();
        if (_capacity > 0)
        {
            _array = new T[_capacity];
            this->_copyRangeUnchecked(other.begin(), other.end(), this->begin());
        }
        else this->_array = nullptr;

        return *this;
    }

    //Iterators
    Iterator begin() const
    {
        return Iterator(this->_array);
    }

    Iterator end() const
    {
        return Iterator(this->_array + this->_length);
    }
};

