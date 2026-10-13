#pragma once

#include "Product.h"

class Warehouse;

class Clothing : public Product
{
public:
    Clothing(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Clothing(int id, const std::string& name,
        const Price& price, 
        const Quantity& quantity,
        const Weight& weight);

    virtual ~Clothing() = default;
};

class TShirt : public Clothing
{
public:
    TShirt(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    TShirt(int id, const std::string& name,
        const Price& price, 
        const Quantity& quantity,
        const Weight& weight);
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;

    
};

class Jacket : public Clothing
{
public:
    Jacket(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight);
    Jacket(int id, const std::string& name,
        const Price& price, 
        const Quantity& quantity,
        const Weight& weight);
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;


}; 