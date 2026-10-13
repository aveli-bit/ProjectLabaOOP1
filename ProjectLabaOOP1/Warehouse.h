#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstddef>

#include "Product.h"

class WarehouseIterator;
class ConstWarehouseIterator;

class Warehouse
{
private:
    std::vector<std::unique_ptr<Product>> products_;

    size_t maxSize_;
    size_t currentSize_;

public:
    // Конструктор
    explicit Warehouse(size_t maxSize);

    // Правило пяти
    Warehouse(const Warehouse& other);

    Warehouse& operator=(const Warehouse& other);// copying 

    Warehouse(Warehouse&& other) noexcept; // ссылаемся на временный или перемещаемый объект(rvalue link)  а тут уже перемещеинг

    Warehouse& operator=(Warehouse&& other) noexcept;

    ~Warehouse() = default;

    // Добавление и удаление
    void addProduct(std::unique_ptr<Product> product);

    void removeById(int id);

    void removeByIndex(size_t index);
    void transferTo(int id, Warehouse& other); // Перемещение товара на другой склад

    // Поиск товаров
    Product* findById(int id);

    std::vector<Product*> findByCategory(
        const std::string& category);

    std::vector<Product*> findByPrice(
        const Price& price);

    // Информация о складе
    size_t getMaxSize() const;

    size_t getCurrentSize() const;

    // Итераторы
    WarehouseIterator begin();
    WarehouseIterator end();

    ConstWarehouseIterator begin() const;
    ConstWarehouseIterator end() const;
};