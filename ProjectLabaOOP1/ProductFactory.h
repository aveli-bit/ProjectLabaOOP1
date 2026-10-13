#pragma once

#include <memory>
#include <string>
// фабрики в одном классе. product factory входит в Фабричный метод, oна определяет общий интерфейс  а конкретные фабрики создают товары.

#include "Product.h"

// Абстрактная фабрика товаров
class ProductFactory
{
public:
    virtual ~ProductFactory() = default;

    virtual std::unique_ptr<Product> createProduct(
        int id,
        const std::string& name,
        int price,
        int quantity,
        double weight
    ) const = 0;
};

// Фабрика смартфонов
class SmartphoneFactory : public ProductFactory
{
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name,
        int price, int quantity, double weight
    ) const override;
};

// Фабрика ноутбуков
class LaptopFactory : public ProductFactory
{
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name,
        int price, int quantity, double weight
    ) const override;
};

// Фабрика футболок
class TShirtFactory : public ProductFactory
{
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name,
        int price, int quantity, double weight
    ) const override;
};

// Фабрика курток
class JacketFactory : public ProductFactory
{
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name,
        int price, int quantity, double weight
    ) const override;
};

// Фабрика молока
class MilkFactory : public ProductFactory
{
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name,
        int price, int quantity, double weight
    ) const override;
};

// Фабрика хлеба
class BreadFactory : public ProductFactory
{
public:
    std::unique_ptr<Product> createProduct(
        int id, const std::string& name,
        int price, int quantity, double weight
    ) const override;
};