#pragma once

#include <string>
#include <memory>

#include "Price.h"
#include "Quantity.h"
#include "Weight.h"

class Warehouse;
class Product
{
protected: // чтобы поля были доступны внутри класса продукт и наследующих классов  
    int id_;
    std::string name_;
    Price price_;
    Quantity quantity_;
    Weight weight_;
    std::string category_;
    Warehouse* owner_ = nullptr;
    friend class Warehouse;

    static long long nextId_; // Это одна переменная для всех объектов продукта и его наследников, чтобы неконстантные товары увеличивались на +1 если требуется
     
public:
    // Конструктор копирования товара
    Product(const Product& other);
    // Запрещаем присваивание товаров для защиты ID и владельца
    Product& operator=(const Product& other) = delete;
    Product(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight,
        const std::string& category);
    Product(int id,
        const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight,
        const std::string& category);

    void transferTo(Warehouse& other);

    virtual ~Product() = default;

    int getId() const;
    const std::string& getName() const;
    const Price& getPrice() const;
    const Quantity& getQuantity() const;
    const Weight& getWeight() const;
    const std::string& getCategory() const;

    void changeQuantity(int delta);
    void setPrice(const Price& newPrice);

    virtual std::unique_ptr<Product> clone() const = 0;
};