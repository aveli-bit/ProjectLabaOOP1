#include "Warehouse.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include "WarehouseIterator.h"
#include "LoggingProxy.h"

// Конструктор склада
Warehouse::Warehouse(size_t maxSize)
    : maxSize_(maxSize), currentSize_(0)
{
    if (maxSize == 0)
    {
        throw std::invalid_argument(
            "Размерность склада должна быть положительной"
        );
    }
    LoggingProxy::log("WAREHOUSE", "Склад создан");
}

// Конструктор копирования
Warehouse::Warehouse(const Warehouse& other)
    : maxSize_(other.maxSize_), currentSize_(0)
{
    for (const auto& product : other.products_)
    {
        auto copy = product->clone();
        copy->owner_ = this;
        products_.push_back(std::move(copy)); 
    }

    currentSize_ = products_.size();
    LoggingProxy::log("WAREHOUSE", "Создана глубокая копия склада");
}

// Оператор копирующего присваивания
Warehouse& Warehouse::operator=(const Warehouse& other)
{
    if (this != &other)
    {
        Warehouse temp(other);

        products_.swap(temp.products_);
        std::swap(maxSize_, temp.maxSize_);
        std::swap(currentSize_, temp.currentSize_);
        for (auto& product : products_)
        {
            product->owner_ = this;
        }
    }
    LoggingProxy::log("WAREHOUSE", "Выполнено копирующее присваивание склада");
    return *this;
}

// Конструктор перемещения
Warehouse::Warehouse(Warehouse&& other) noexcept
    : products_(std::move(other.products_)),
    maxSize_(other.maxSize_),
    currentSize_(other.currentSize_)

{
    for (auto& product : products_)
    {
        product->owner_ = this;
    }
    other.maxSize_ = 0;
    other.currentSize_ = 0;
    LoggingProxy::log("WAREHOUSE", "Склад перемещён через конструктор");
}

// Оператор перемещающего присваивания
Warehouse& Warehouse::operator=(Warehouse&& other) noexcept
{
    if (this != &other)
    {
        products_ = std::move(other.products_);
        maxSize_ = other.maxSize_;
        currentSize_ = other.currentSize_;

        other.maxSize_ = 0;
        other.currentSize_ = 0;
        for (auto& product : products_)
        {
            product->owner_ = this;
        }
    }
    LoggingProxy::log("WAREHOUSE", "Выполнено перемещающее присваивание склада");
    return *this;
}

// Максимальная вместимость
size_t Warehouse::getMaxSize() const
{
    return maxSize_;
}

// Текущее количество товаров
size_t Warehouse::getCurrentSize() const
{
    return currentSize_;
}

// Добавление товара на склад
void Warehouse::addProduct(std::unique_ptr<Product> product)
{
    // Проверка на пустой указатель
    if (!product)
    {
        throw std::invalid_argument("Нельзя добавить пустой товар");
    }

    // Проверка вместимости склада
    if (currentSize_ >= maxSize_)
    {
        throw std::runtime_error("Склад переполнен");
    }

    // Проверка уникальности ID
    if (findById(product->getId()) != nullptr)
    {
        throw std::invalid_argument(
            "Товар с таким ID уже существует на складе"
        );
    }
    product->owner_ = this;

    // Передача владения товаром складу
    products_.push_back(std::move(product));

    // Обновление количества товаров
    currentSize_ = products_.size();
    LoggingProxy::log("WAREHOUSE", "Товар добавлен на склад");
}


// Удаление товара по ID
void Warehouse::removeById(int id)
{
    auto it = std::find_if(
        products_.begin(),
        products_.end(),
        [id](const std::unique_ptr<Product>& product)
        {
            return product->getId() == id;
        }
    );

    if (it == products_.end())
    {
        throw std::out_of_range("Не найден айди продукта");
    }

    products_.erase(it);
    currentSize_ = products_.size();
    LoggingProxy::log("WAREHOUSE", "Товар удалён по ID");
}

// Удаление товара по индексу
void Warehouse::removeByIndex(size_t index)
{
    if (index >= currentSize_)
    {
        throw std::out_of_range("Недопустимый индекс");
    }

    products_.erase(products_.begin() + index);
    currentSize_ = products_.size();
    LoggingProxy::log("WAREHOUSE", "Товар удалён по индексу");
}

// Поиск товара по ID
Product* Warehouse::findById(int id)
{
    for (auto& product : products_)
    {
        if (product->getId() == id)
        {
            return product.get();
        }
    }

    return nullptr;
}

// Поиск товаров по категории
std::vector<Product*> Warehouse::findByCategory(
    const std::string& category)
{
    std::vector<Product*> result;

    for (auto& product : products_)
    {
        if (product->getCategory() == category)
        {
            result.push_back(product.get());
        }
    }

    LoggingProxy::log("WAREHOUSE", "Выполнен поиск по категории");

    return result;
}

// Поиск товаров по цене
std::vector<Product*> Warehouse::findByPrice(
    const Price& price)
{
    std::vector<Product*> result;

    for (auto& product : products_)
    {
        if (product->getPrice().getValue() == price.getValue())
        {
            result.push_back(product.get());
        }
    }

    LoggingProxy::log("WAREHOUSE", "Выполнен поиск по цене");

    return result;
}

// Перемещение товара между складами
void Warehouse::transferTo(int id, Warehouse& other)
{
    // Проверяем, что склады разные
    if (this == &other)
    {
        throw std::invalid_argument(
            "Нельзя перемещать на тот же склад, склады должны быть разные"
        );
    }

    // Ищем товар по ID
    auto it = std::find_if(
        products_.begin(),
        products_.end(),
        [id](const std::unique_ptr<Product>& product)
        {
            return product->getId() == id;
        }
    );

    if (it == products_.end())
    {
        throw std::out_of_range("Продукт не найден");
    }

    // Проверяем вместимость второго склада
    if (other.currentSize_ >= other.maxSize_)
    {
        throw std::runtime_error(
            "Склад назначения переполнен"
        );
    }

    // Проверяем отсутствие одинакового ID
    if (other.findById(id) != nullptr)
    {
        throw std::runtime_error(
            "Такой айди продукта уже существует"
        );
    }

    // Выделяем место до перемещения товара
    other.products_.reserve(other.products_.size() + 1);

    // Передаём владение товаром
    other.products_.push_back(std::move(*it));
    other.products_.back()->owner_ = &other;

    // Удаляем пустой указатель с первого склада
    products_.erase(it);

    // Обновляем размеры
    currentSize_ = products_.size();
    other.currentSize_ = other.products_.size();
    LoggingProxy::log("WAREHOUSE", "Товар перемещён между складами");
}

// Итератор на первый товар склада
WarehouseIterator Warehouse::begin()
{
    return WarehouseIterator(
        products_.begin(), products_.end()
    );
}

// Итератор на конец склада
WarehouseIterator Warehouse::end()
{
    return WarehouseIterator(
        products_.end(), products_.end()
    );
}

// Константный итератор на первый товар
ConstWarehouseIterator Warehouse::begin() const
{
    return ConstWarehouseIterator(
        products_.cbegin(), products_.cend()
    );
}

// Константный итератор на конец склада
ConstWarehouseIterator Warehouse::end() const
{
    return ConstWarehouseIterator(
        products_.cend(), products_.cend()
    );
}