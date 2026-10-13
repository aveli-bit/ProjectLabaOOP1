#include "Clothing.h"

Clothing::Clothing(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Product(name, price, quantity, weight, "clothing")
{
}

TShirt::TShirt(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Clothing(name, price, quantity, weight)
{
}

std::unique_ptr<Product> TShirt::clone() const
{
    return std::make_unique<TShirt>(*this);
}

Jacket::Jacket(const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Clothing(name, price, quantity, weight)
{
}

std::unique_ptr<Product> Jacket::clone() const
{
    return std::make_unique<Jacket>(*this);
}
// Конструктор одежды с заданным ID
Clothing::Clothing(int id, const std::string& name,
    const Price& price, 
    const Quantity& quantity,
    const Weight& weight)
    : Product(id, name, price, quantity, weight, "clothing")
{
}

// Конструктор футболки с заданным ID
TShirt::TShirt(int id, const std::string& name,
    const Price& price, 
    const Quantity& quantity,
    const Weight& weight)
    : Clothing(id, name, price, quantity, weight)
{
}

// Конструктор куртки с заданным ID
Jacket::Jacket(int id, const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight)
    : Clothing(id, name, price, quantity, weight)
{
}