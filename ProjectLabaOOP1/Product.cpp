#include "Product.h"
#include <stdexcept>
#include "Warehouse.h"
#include "LoggingProxy.h"
#include <limits>
#include <cmath>


long long Product::nextId_ = 1;

// проверяем общий счётчик перед созданием товара с автоматическим ID
int Product::nextAutomaticId()
{
    if (nextId_ > std::numeric_limits<int>::max())
    {
        throw std::overflow_error("Закончились доступные ID товаров");
    }
    return static_cast<int>(nextId_);
}

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
    type_(ProductType::getType(category, weight, price)),
    category_(type_->getCategory())
{
    // Проверка доступности идентификаторов
    id_ = nextAutomaticId();
    ++nextId_;
} 

// эти методы возвращают текущие характеристики товара
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
    return type_->getBaseWeight();
}

const std::string& Product::getCategory() const
{
    return category_;
}

// меняем количество через проверку в Quantity
void Product::changeQuantity(int delta)
{
    if (owner_ && delta < 0)
    {
        const int reserved = owner_->reservedQuantity(id_);
        if (reserved > 0 && static_cast<long long>(quantity_.getValue()) + delta < reserved)
            throw std::invalid_argument("Остаток не может быть меньше резерва");
    }
    quantity_.change(delta);
    LoggingProxy::log("PRODUCT", "Количество товара изменено");
}

// меняем текущую цену самого товара
void Product::setPrice(const Price& newPrice)
{
    changePrice(newPrice);
}

void Product::changePrice(const Price& newPrice)
{
    price_ = newPrice;

    LoggingProxy::log("PRODUCT", "Цена товара изменена");
}
// создаём товар с заданным ID и обновляем общий счётчик
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
    type_(ProductType::getType(category, weight, price)),
    category_(type_->getCategory())
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
    type_(other.type_),
    category_(type_->getCategory()),
    owner_(nullptr)
{
}
// считаем стоимость всего количества товара
long long Product::getTotalPrice() const
{
    return static_cast<long long>(price_.getValue())
        * quantity_.getValue();
}
// считаем общий вес и проверяем результат
double Product::getTotalWeight() const
{
    const double totalWeight =
        getWeight().getValue() * quantity_.getValue();

    if (!std::isfinite(totalWeight))
    {
        throw std::overflow_error(
            "Общий вес товара слишком большой"
        );
    }

    return totalWeight;
}
