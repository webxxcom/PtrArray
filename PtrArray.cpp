#pragma once
#include "stdafx.h"

template<typename TypeToClone>
concept cloneable = requires(TypeToClone obj)
{ obj.clone(); };

//Non-movable array that stores pointers
template <cloneable T>
class PtrArray
{
public:
    class Iterator;
    class Wrapper;

    //Aliases for std library algorithms
    using value_type = Wrapper;
    using pointer = value_type*;
    using reference = value_type&;

private:
    //Fields
    const size_t _init_capacity = 10;
    size_t _length = 0;
    size_t _capacity = 0;
    pointer _array = nullptr;

    //Private methods
    
    //Change length, capacity, array and if _Arr is nullptr allocate new memory
    void _change_Array(pointer _Arr, size_t _Len, size_t _Cap)
    {
        if (!_Arr) this->_allocate(_Cap);
        else this->_array = _Arr, this->_capacity = _Cap;

        this->_length = _Len;
    }

    //Allocate array of nullptrs if _new_Capacity > 0 otherwise _array = nullptr, set new capacity
    void _allocate(size_t _new_Capacity)
    {
        this->_array = _new_Capacity > 0 ?
            new value_type[_new_Capacity]
            : nullptr;

        this->_capacity = _new_Capacity;
    }
    
    //Delete all the pointers and set _array to nullptr
    void _deallocate()
    {
        //Clear all the data pointer points at
        delete[] this->_array;
    }

    //Check whether _array is full of elements
    bool _isFull() const
    {
        return this->_length >= this->_capacity;
    }

    //Double capacity -> move to new array -> assign
    void _expandAndMove()
    {
        size_t newCapacity = (this->_capacity + 1) << 1;

        auto newArray = new value_type[newCapacity];
        std::ranges::move(*this, newArray);

        this->_deallocate();
        this->_change_Array(newArray, this->_length, newCapacity);
    }

    void _expandAndMove(size_t _Total_Capacity)
    {
        size_t newCapacity = (_Total_Capacity + 1) << 1;

        auto newArray = new value_type[newCapacity];
        std::ranges::move(*this, newArray);

        this->_deallocate();
        this->_change_Array(newArray, this->_length, newCapacity);
    }

    template<typename... Args>
    void _Emplace_elements(Iterator it, Args&&... elems)
    {
        ((*(it++) = std::forward<Args>(elems)), ...);
        this->_length += sizeof...(elems);
    }

public:
    //Define wrapper for T pointer to handle the assignment operator for dereferenced iterators properly
    //Wrapper allows for an array to be working with STL library algorithms
    struct Wrapper
    {
    private:
        //Aliases
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        //Pointer at data
        pointer _data = nullptr;

    public:
        //Constructors
        explicit Wrapper() noexcept = default;

        explicit Wrapper(pointer const& _ptr) noexcept
            //To copy data we need to clone it if there is something in 'ptr'
            : _data(_ptr ? _ptr->clone() : nullptr)
        { }

        explicit Wrapper(pointer&& _ptr) noexcept
            : _data(std::move(_ptr))
        { _ptr = nullptr; }

        Wrapper(Wrapper const& other) noexcept
            : _data(other._data ? other._data->clone() : nullptr)
        { }

        Wrapper(Wrapper&& other) noexcept
            : _data(std::move(other._data))
        { other._data = nullptr; }

        ~Wrapper()
        {
            delete this->_data;
        }

        //Allow implicit conversion to T*
        explicit(false) operator pointer() const { return this->_data; }

        //Copy and move assignment operators for other object of 'Wrapper' type
        Wrapper& operator=(Wrapper const& other) noexcept
        {
            delete this->_data;

            this->_data = other._data ? other._data->clone() : nullptr;
            return *this;
        }

        Wrapper& operator=(Wrapper&& other) noexcept
        {
            if (this->_data != other._data)
            {
                delete this->_data;

                this->_data = other._data;
                other._data = nullptr;
            }

            return *this;
        }

        //Copy assignment operators for T* type
        Wrapper& operator=(pointer const& ptr) noexcept
        {
            if (this->_data != ptr)
            {
                delete this->_data;
                this->_data = ptr ? ptr->clone() : nullptr;
            }

            return *this;
        }

        //Move assignment operator for pointer to avoid memory leak when assigning rvalue reference
        Wrapper& operator=(pointer&& ptr) noexcept
        {
            delete this->_data;
            this->_data = std::move(ptr);

            ptr = nullptr;
            return *this;
        }

        //Overload '->' to get access to data without converting
        pointer operator->() const
        {
            return this->_data;
        }

        //Implement spaceship operator to compare 'Wrapper's without converting to 'T*'
        std::strong_ordering operator<=>(Wrapper const& other) const noexcept
        {
            if (!_data && !other._data)
                return std::strong_ordering::equivalent;

            if (!_data)
                return std::strong_ordering::less;

            if (!other._data)
                return std::strong_ordering::greater;

            if (*_data < *other._data)
                return std::strong_ordering::less;

            if (*_data > *other._data)
                return std::strong_ordering::greater;

            return std::strong_ordering::equivalent;
        }
};

    //Random-access iterator
    class Iterator
    {
    private:
        Wrapper* m_ptr;

    public:
        //Aliases for std library algorithms
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Wrapper;
        using pointer = value_type*;
        using reference = value_type&;

        //Constructors
        Iterator() : m_ptr(nullptr) { }

        explicit Iterator(Wrapper* m_ptr) : m_ptr(m_ptr) { };

        //Accesssors
        reference operator*() const { return *m_ptr; }
        T* operator->() { return *m_ptr; }

        reference operator[] (const int& ind) const { return this->m_ptr[ind]; }

        //Arithmetic
        Iterator& operator++() { ++m_ptr;  return *this; }
        Iterator& operator--() { --m_ptr;  return *this; }

        Iterator operator++(int) { auto temp = *this; ++m_ptr;  return temp; }
        Iterator operator--(int) { auto temp = *this; --m_ptr;  return temp; }

        Iterator operator+(difference_type n) const { return Iterator(m_ptr + n); }
        Iterator operator-(difference_type n) const { return Iterator(m_ptr - n); }
 
        Iterator& operator+=(difference_type n) { m_ptr += n; return *this; }
        Iterator& operator-=(difference_type n) { m_ptr -= n; return *this; }

        static friend Iterator operator+(difference_type n, Iterator other) { return Iterator(m_ptr + n); }
        difference_type operator-(Iterator const& rhs) const { return m_ptr - rhs.m_ptr; }
       
        //Comparison
        std::strong_ordering operator<=>(Iterator const& rhs) const = default;
    };

    //Constructors
    PtrArray()
    { this->_allocate(this->_capacity); }

    PtrArray(PtrArray<T> const& other)
    {
        this->operator=(other);
    }

    PtrArray(PtrArray<T>&& other) noexcept
    {
        this->operator=(std::move(other));
    }

    template<typename... Args>
    explicit PtrArray(Args&&... elems) noexcept
    {
        size_t size = sizeof...(elems);
        this->_allocate(size);

        this->_Emplace_elements(this->begin(), std::forward<Args>(elems)...);
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
    template <typename... Args>
    void emplace(Iterator position, Args&&... elems)
    {
        size_t index = position - this->begin();
        size_t size = sizeof...(elems);
        if (this->_length + size > this->_capacity)
        {
            this->_expandAndMove(this->_length + size);
            position = this->begin() + index;
        }

        // Move elements to make space for the new element
        std::move_backward(position, this->end(), this->end() + size);

        //Emplace object at a now-freed position
        this->_Emplace_elements(position, std::forward<Args>(elems)...);
    }

    template <typename... Args>
    void emplace_back(Args&&... elems)
    {
        this->emplace(this->end(), std::forward<Args>(elems)...);
    }

    template<typename U>
    void push_back(U&& obj)
    {
        this->emplace(this->end(), std::forward<U>(obj));
    }

    //Erase elements at [_First, _Last)
    void erase(Iterator _First, Iterator _Last)
    {
        if (this->empty() || _First < this->begin() || _Last > this->end() || _First >= _Last)
            return;

        //Shift data after _Last to the left 
        std::move(_Last, this->end(), _First);

        //nullptr the now-dangled pointers
        auto _dist = std::distance(_First, _Last);
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

        this->_change_Array(nullptr, 0, this->_init_capacity);
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