#pragma once

#include <vector>
#include <memory> // unique_ptr make_unique
#include <iterator>  // для взаимодействия с итератором

#include "Product.h"

// Обычный итератор склада
class WarehouseIterator
{
private:
    using Iterator = std::vector<std::unique_ptr<Product>>::iterator;

    Iterator current_;
    Iterator end_;

public:
    WarehouseIterator(Iterator current, Iterator end);

    // получаем сам товар на текущей позиции
    Product& operator*() const;

    // переходим к следующей позиции вектора
    WarehouseIterator& operator++();

    bool operator!=(const WarehouseIterator& other) const;

    bool operator==(const WarehouseIterator& other) const;
};

// Константный итератор склада
class ConstWarehouseIterator
{
private:
    using Iterator =
        std::vector<std::unique_ptr<Product>>::const_iterator;

    Iterator current_;
    Iterator end_;

public:
    ConstWarehouseIterator(Iterator current, Iterator end);

    // получаем товар без возможности менять его через эту ссылку
    const Product& operator*() const;

    // двигаемся дальше при чтении константного склада
    ConstWarehouseIterator& operator++();

    bool operator!=(const ConstWarehouseIterator& other) const;

    bool operator==(const ConstWarehouseIterator& other) const;
};