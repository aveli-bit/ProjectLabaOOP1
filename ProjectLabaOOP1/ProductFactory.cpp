#include "ProductFactory.h"
#include "LoggingProxy.h"
#include "Electronics.h"
#include "Clothing.h"
#include "Food.h"

// Создание смартфона
std::unique_ptr<Product> SmartphoneFactory::createProduct(
    int id, const std::string& name,
    int price, int quantity, double weight) const
{
    // 
    auto product = std::make_unique<Smartphone>(
        id, name, Price(price),
        Quantity(quantity), Weight(weight)
    );

    
    LoggingProxy::log("FACTORY", "Создан смартфон");

   
    return product;
}
// Создание ноутбука
std::unique_ptr<Product> LaptopFactory::createProduct(
    int id, const std::string& name,
    int price, int quantity, double weight) const
{
    auto product = std::make_unique<Laptop>(
        id, name, Price(price),
        Quantity(quantity), Weight(weight)
    );

    LoggingProxy::log("FACTORY", "Создан ноутбук");

    return product;
}

// Создание футболки
std::unique_ptr<Product> TShirtFactory::createProduct(
    int id, const std::string& name,
    int price, int quantity, double weight) const
{
    auto product = std::make_unique<TShirt>(
        id, name, Price(price),
        Quantity(quantity), Weight(weight)
    );

    LoggingProxy::log("FACTORY", "Создана футболка");

    return product;
}

// Создание куртки
std::unique_ptr<Product> JacketFactory::createProduct(
    int id, const std::string& name,
    int price, int quantity, double weight) const
{
    auto product = std::make_unique<Jacket>(
        id, name, Price(price),
        Quantity(quantity), Weight(weight)
    );

    LoggingProxy::log("FACTORY", "Создана куртка");

    return product;
}

// Создание молока
std::unique_ptr<Product> MilkFactory::createProduct(
    int id, const std::string& name,
    int price, int quantity, double weight) const
{
    auto product = std::make_unique<Milk>(
        id, name, Price(price),
        Quantity(quantity), Weight(weight)
    );

    LoggingProxy::log("FACTORY", "Создано молоко");

    return product;
}

// Создание хлеба
std::unique_ptr<Product> BreadFactory::createProduct(
    int id, const std::string& name,
    int price, int quantity, double weight) const
{
    auto product = std::make_unique<Bread>(
        id, name, Price(price),
        Quantity(quantity), Weight(weight)
    );

    LoggingProxy::log("FACTORY", "Создан хлеб");

    return product;
}