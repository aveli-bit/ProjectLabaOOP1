#include "WarehouseIterator.h"
#include <stdexcept>

// Конструктор обычного итератора
WarehouseIterator::WarehouseIterator(Iterator current, Iterator end)
    : current_(current), end_(end)
{
}

// Получение текущего товара
Product& WarehouseIterator::operator*() const
{
    if (current_ == end_)
    {
        throw std::out_of_range("Достигнут конец склада");
    }

    return **current_;
}

// Переход к следующему товару
WarehouseIterator& WarehouseIterator::operator++()
{
    if (current_ == end_)
    {
        throw std::out_of_range("Нельзя перейти за конец склада");
    }

    ++current_;
    return *this;
}

// Проверка неравенства итераторов
bool WarehouseIterator::operator!=(
    const WarehouseIterator& other) const
{
    return current_ != other.current_;
}

// Проверка равенства итераторов
bool WarehouseIterator::operator==(
    const WarehouseIterator& other) const
{
    return current_ == other.current_;
}


// Конструктор константного итератора
ConstWarehouseIterator::ConstWarehouseIterator(
    Iterator current, Iterator end)
    : current_(current), end_(end)
{
}

// Получение текущего товара
const Product& ConstWarehouseIterator::operator*() const
{
    if (current_ == end_)
    {
        throw std::out_of_range("Достигнут конец склада");
    }

    return **current_;
}

// Переход к следующему товару
ConstWarehouseIterator& ConstWarehouseIterator::operator++()
{
    if (current_ == end_)
    {
        throw std::out_of_range("Нельзя перейти за конец склада");
    }

    ++current_;
    return *this;
}

// Проверка неравенства итераторов
bool ConstWarehouseIterator::operator!=(
    const ConstWarehouseIterator& other) const
{
    return current_ != other.current_;
}

// Проверка равенства итераторов
bool ConstWarehouseIterator::operator==(
    const ConstWarehouseIterator& other) const
{
    return current_ == other.current_;
}