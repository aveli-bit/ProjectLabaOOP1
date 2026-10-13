#pragma once

#include "Product.h"

class Warehouse;

class Food : public Product
{
public:
    Food(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Food(int id, const std::string& name,
        const Price& price, 
        const Quantity& quantity,
        const Weight& weight);

    virtual ~Food() = default;
};

class Milk : public Food
{
public:
    Milk(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Milk(int id, const std::string& name,
        const Price& price, 
        const Quantity& quantity,
        const Weight& weight);
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;

    
};

class Bread : public Food
{
public:
    Bread(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Bread(int id, const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;

    
};