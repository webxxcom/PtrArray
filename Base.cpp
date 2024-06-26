#pragma once
#include "stdafx.h"

class Person {
public:
    std::string name;
    int age;

    Person(const std::string& name = "", int age = 0) : name(name), age(age) {}
    Person(Person const& other) = default;
    Person(Person&& other) = default;
    Person& operator=(Person const& other) = default;
    Person& operator=(Person&& other) = default;

    // Overload the + operator
    Person operator+(const Person& other) const {
        return Person(this->name + " " + other.name, this->age + other.age);
    }

    std::strong_ordering operator<=>(Person const& other) const = default;
};

class Base {
protected:
    int field;
public:
    Base(int a) : field(a) { }
    Base(Base const& other) = default;
    Base(Base&& other) = default;
    virtual void display() const = 0; // Pure virtual function
    virtual Base* clone() const = 0;
    int getValue() const { return field; }

    std::strong_ordering operator<=>(Base const& other) const = default;
    bool operator==(int val) const { return this->field == val; }

    Base& operator=(Base&& other) = default;
    Base& operator=(Base const& other) = default;

    static friend std::ostream& operator<<(std::ostream& out, Base const& other)
    {
        out << other.getValue();

        return out;
    }

    virtual ~Base() = default; // Virtual destructor
};

class Derived1 : public Base {
public:
    using Base::Base;
    void display() const override {
        std::cout << "Derived1" << std::endl;
    }

    Base* clone() const override
    {
        return new Derived1(*this);
    }
};

class Derived2 : public Base {
public:
    using Base::Base;
    void display() const override {
        std::cout << "Derived2" << std::endl;
    }

    Base* clone() const override
    {
        return new Derived2(*this);
    }
};