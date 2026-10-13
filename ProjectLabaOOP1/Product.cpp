#include "Product.h"
#include <stdexcept>
#include "Warehouse.h"
#include "LoggingProxy.h"
#include <limits>


long long Product::nextId_ = 1;

// Конструктор товара с автоматическим ID
Product::Product(const std::string& name, // абстрактный класс / список инициализации конструктора
    const Price& price,
    const Quantity& quantity,
    const Weight& weight,
    const std::string& category) // конст чтобы не изменять объект если он константен (относится к 11 строке)
    : id_(0),
    name_(name),
    price_(price),
    quantity_(quantity),
    weight_(weight),
    category_(category) // хар-ки
{
    // Проверка доступности идентификаторов
    if (nextId_ > std::numeric_limits<int>::max())
    {
        throw std::overflow_error("Закончились доступные ID товаров");
    }

    id_ = static_cast<int>(nextId_);
    ++nextId_;
} 

int Product::getId() const
{   
    return id_;
}

const std::string& Product::getName() const
{
    return name_;
}

const Price& Product::getPrice() const
{
    return price_;
}

const Quantity& Product::getQuantity() const
{
    return quantity_;
}

const Weight& Product::getWeight() const
{
    return weight_;
}

const std::string& Product::getCategory() const
{
    return category_;
}

void Product::changeQuantity(int delta)
{
    quantity_.change(delta);
    LoggingProxy::log("PRODUCT", "Количество товара изменено");
}

void Product::setPrice(const Price& newPrice)
{
    price_ = newPrice;
    LoggingProxy::log("PRODUCT", "Цена товара изменена");
}
Product::Product(int id,
    const std::string& name,
    const Price& price,
    const Quantity& quantity,
    const Weight& weight,
    const std::string& category)
    : id_(id),
    name_(name),
    price_(price),
    quantity_(quantity),
    weight_(weight),
    category_(category)
{
    // Проверка корректности ID
        if (id <= 0)
        {
            throw std::invalid_argument(
                "ID товара должен быть положительным"
            );
        }

        // Обновление счётчика автоматических ID
        if (id >= nextId_)
        {
            nextId_ = static_cast<long long>(id) + 1;
        }
}
// Перемещение товара на другой склад
void Product::transferTo(Warehouse& other)
{
    if (owner_ == nullptr)
    {
        throw std::runtime_error(
            "Товар не принадлежит ни одному складу"
        );
    }

    // Передаём товар через его текущий склад
    owner_->transferTo(id_, other);
}
// Конструктор копирования товара
Product::Product(const Product& other)
    : id_(other.id_),
    name_(other.name_),
    price_(other.price_),
    quantity_(other.quantity_),
    weight_(other.weight_),
    category_(other.category_),
    owner_(nullptr)
{
}