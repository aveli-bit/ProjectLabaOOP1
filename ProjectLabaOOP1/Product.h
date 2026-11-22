#pragma once

#include <string>
#include <memory>
#include "ProductType.h"
#include "Price.h"
#include "Quantity.h"
#include "Weight.h"

class Warehouse;
class Product
{
    friend class ProductFactory;
    static int nextAutomaticId();

protected: // чтобы поля были доступны внутри класса продукт и наследующих классов  
    int id_;
    std::string name_;
    Price currentPrice_;
    Quantity quantity_;
    std::shared_ptr<ProductType> type_; //  получаем через поле type_ общую категорию базовый вес и базовую цену
    Warehouse* owner_ = nullptr;
    friend class Warehouse;

    static long long nextId_; // Это один общий счётчик ID для всех товаров, а не отдельный у каждого здесь статическое поле принадлежит сразу всему классу а не одной переменной или объекут
public:
    // Конструктор копирования товара
    Product(const Product& other);
    // Запрещаем присваивание товаров для защиты ID и владельца
    Product& operator=(const Product& other) = delete;
    // создаём товар с автоматическим ID
    Product(const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight,
        const std::string& category);
    // создаём товар с указанным ID
    Product(int id,
        const std::string& name,
        const Price& price,
        const Quantity& quantity,
        const Weight& weight,
        const std::string& category);

    void transferTo(Warehouse& other);

    // виртуальный деструктор нужен для удаления наследника через указатель на Product
    virtual ~Product() = default;

    int getId() const;
    const std::string& getName() const;
    const Price& getPrice() const;
    const Quantity& getQuantity() const;
    const Weight& getWeight() const;
    const std::string& getCategory() const;
    long long getTotalPrice() const;
    double getTotalWeight() const;

    void changeQuantity(int delta);
    void changePrice(const Price& newPrice);
    void setPrice(const Price& newPrice);

    // наследник должен создавать копию своего конкретного типа
    virtual std::unique_ptr<Product> clone() const = 0;
};
