#pragma once

#include <vector>
#include <memory>
#include <iterator>

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

    Product& operator*() const;

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

    const Product& operator*() const;

    ConstWarehouseIterator& operator++();

    bool operator!=(const ConstWarehouseIterator& other) const;

    bool operator==(const ConstWarehouseIterator& other) const;
};