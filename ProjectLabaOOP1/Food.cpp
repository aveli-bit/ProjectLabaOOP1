#include "Food.h"

// передаём характеристики и категорию в Product
Food::Food(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Product(name, price, quantity, weight, "food")
{
}

// конструктор молока с автоматическим ID
Milk::Milk(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Food(name, price, quantity, weight)
{
}

// создаём отдельную копию молока и возвращаем владеющий указатель
std::unique_ptr<Product> Milk::clone() const
{
    return std::make_unique<Milk>(*this);
}

// конструктор хлеба с автоматическим ID
Bread::Bread(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Food(name, price, quantity, weight)
{
}

// создаём отдельную копию хлеба и возвращаем владеющий указатель
std::unique_ptr<Product> Bread::clone() const
{
    return std::make_unique<Bread>(*this);
}
// Конструктор продуктов питания с заданным ID
Food::Food(int id, const std::string& name,
    const Price& price, 
    const Quantity& quantity,
    const Weight& weight)
    : Product(id, name, price, quantity, weight, "food")
{
}

// Конструктор молока с заданным ID
Milk::Milk(int id, const std::string& name,
    const Price& price, 
    const Quantity& quantity,
    const Weight& weight)
    : Food(id, name, price, quantity, weight)
{
}

// Конструктор хлеба с заданным ID
Bread::Bread(int id, const std::string& name,
    const Price& price, 
    const Quantity& quantity,
    const Weight& weight)
    : Food(id, name, price, quantity, weight)
{
}