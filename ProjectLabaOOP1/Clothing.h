#pragma once

#include "Product.h"

class Warehouse;

class Clothing : public Product
{
public:
    // общие конструкторы для одежды
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
    // передаём перемещение товара методу родительского класса
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override; // слово переопределения, использующаяся для назначения функций,которые переопределяют виртуальную функцию в базовом классe

    
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
    // передаём перемещение товара методу родительского класса
    void transferTo(Warehouse& other)
    {
        Product::transferTo(other);
    }

    std::unique_ptr<Product> clone() const override;


}; 