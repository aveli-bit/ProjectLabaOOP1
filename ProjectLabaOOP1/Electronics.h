#pragma once

#include "Product.h"

class Warehouse;

class Electronics : public Product
{
public:
    // общие конструкторы для электроники
    Electronics(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Electronics(int id,
        const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);

    virtual ~Electronics() = default;
};

class Smartphone : public Electronics
{
public:
    Smartphone(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Smartphone(int id,
        const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    // передаём перемещение товара методу родительского класса
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;
    
};

class Laptop : public Electronics
{
public:
    Laptop(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Laptop(int id,
        const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    // передаём перемещение товара методу родительского класса
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;
    
};
