#pragma once

#include <iosfwd>
#include <unordered_map>
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

    struct StockEntry
    {
        Product* product;
        int warehouseId;
        int reserved;
    };

    // по ID находим товар и его резерв а вектор сохраняет порядок обхода
    std::unordered_map<int, StockEntry> productsById_;
    size_t maxSize_;
    int locationCount_ = 1;
    long long nextProductId_ = 1;

    StockEntry& stockFor(int id);
    size_t countAt(int warehouseId) const;
    void swapState(Warehouse& other) noexcept;
    friend class ShopMediator;
    friend class Product;
    int reservedQuantity(int id) const noexcept;
    std::shared_ptr<Product> prepareItem(int id);
    void reserve(int id);
    void release(int id) noexcept;
    void consume(int id) noexcept;
    Product& requireProduct(int id);

public:
    // магазин начинает работу с двумя складами по сто мест
    Warehouse();

    // Конструктор
    explicit Warehouse(size_t maxSize);

    // Правило пяти
    Warehouse(const Warehouse& other);

    Warehouse& operator=(const Warehouse& other);// copying 

    Warehouse(Warehouse&& other) noexcept; // ссылаемся на временный или перемещаемый объект(rvalue link)  а тут уже перемещеинг

    Warehouse& operator=(Warehouse&& other) noexcept;

    // при удалении склада unique_ptr автоматически удалят его товары
    ~Warehouse() = default;

    // Добавление и удаление
    void addProduct(std::unique_ptr<Product> product);
    int addProduct(const std::string& type, const std::string& name, int price, int quantity);
    void moveProduct(int id, int warehouseId);
    void listProducts(std::ostream& output) const;
    void changePrice(int id, const Price& price);
    void changePrice(int id, int newPrice);
    void changeQuantity(int id, int delta);
    void moveProduct(int id, Warehouse& other);
    void removeProduct(int id);

    void removeById(int id);

    void removeByIndex(size_t index);
    void transferTo(int id, Warehouse& other); // Перемещение товара на другой склад

    // Поиск товаров
    // возвращаем указатель на товар без передачи владения
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
