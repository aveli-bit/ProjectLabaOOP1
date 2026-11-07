#include "Electronics.h"

// передаём характеристики и категорию в Product
Electronics::Electronics(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Product(name, price, quantity, weight, "electronics")
{
}

// конструктор смартфона с автоматическим ID
Smartphone::Smartphone(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Electronics(name, price, quantity, weight)
{
}

// создаём отдельную копию смартфона и возвращаем владеющий указатель
std::unique_ptr<Product> Smartphone::clone() const
{
    return std::make_unique<Smartphone>(*this);
}

// конструктор ноутбука с автоматическим ID
Laptop::Laptop(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Electronics(name, price, quantity, weight)
{
}

// создаём отдельную копию ноутбука и возвращаем владеющий указатель
std::unique_ptr<Product> Laptop::clone() const
{
    return std::make_unique<Laptop>(*this);
}
// Конструктор электроники с заданным ID
Electronics::Electronics(int id,
    const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Product(id, name, price, quantity, weight, "electronics")
{
}

// Конструктор смартфона с заданным ID
Smartphone::Smartphone(int id,
    const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Electronics(id, name, price, quantity, weight)
{
}

// Конструктор ноутбука с заданным ID
Laptop::Laptop(int id,
    const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Electronics(id, name, price, quantity, weight)
{
}