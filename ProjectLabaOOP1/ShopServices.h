// подсистемы паттерна посредник склад оплата и доставка
// способы доставки используют паттерн стратегия из второй лабораторной
#pragma once

#include "Warehouse.h"
#include "Delivery.h"
#include <array>
#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <unordered_set>

class Order;
class ShopMediator;

// хранит два склада и ищет товар по общему списку ID
class ShopInventory
{
private:
    friend class ShopMediator;

    struct ProductLocation
    {
        std::size_t warehouseIndex;
        Product* product;
    };

    std::array<Warehouse, 2> warehouses_{Warehouse(100), Warehouse(100)};
    // по ID находим номер склада и адрес товара без передачи владения
    std::unordered_map<int, ProductLocation> productsById_;
    // храним количество единиц отложенных для заказов
    std::unordered_map<int, int> reserved_;
    long long nextProductId_ = 1;

    void swapState(ShopInventory& other) noexcept;
    ProductLocation& locate(int id);
    int reservedQuantity(int id) const;
    std::shared_ptr<Product> prepareItem(int id);
    void reserve(int id);
    void release(int id) noexcept;
    void consume(int id) noexcept;

public:
    ShopInventory() = default;
    ShopInventory(const ShopInventory& other);
    ShopInventory& operator=(const ShopInventory& other);
    ShopInventory(ShopInventory&& other);
    ShopInventory& operator=(ShopInventory&& other);

    int addProduct(const std::string& type, const std::string& name, int price, int quantity);
    void changePrice(int id, int price);
    void changeQuantity(int id, int delta);
    void moveProduct(int id, int warehouseId);
    void removeProduct(int id);
    void listProducts(std::ostream& output) const;

    std::size_t getUsedSlots() const;
    std::size_t getTotalSlots() const;
};

// хранит запросы на оплату и учитывает списания клиента
class PaymentSystem
{
private:
    struct Payment
    {
        std::string customer;
        double amount;
        bool paid = false;
    };
    // ключом записи об оплате служит ID заказа
    std::unordered_map<int, Payment> payments_;

public:
    double checkBalance(const std::string& customer) const;
    void requestPayment(int orderId, const std::string& customer, double amount);
    void processPayment(int orderId, double amount);
    void resetPayment(int orderId) noexcept;
    void refund(int orderId) noexcept;
};

// выбирает способ доставки и хранит планы по ID заказа
class DeliveryService
{
private:
    CourierDelivery couriers_;
    PickupDelivery pickupPoints_;
    PostDelivery postOffices_;
    // в плане храним ID заказов которые ещё нужно отправить
    std::unordered_set<int> scheduled_;

public:
    std::shared_ptr<Delivery> select(const std::string& type) const;
    bool isAvailable(const Delivery& delivery, const Order& order) const;
    double calculateCost(const Delivery& delivery, const Order& order) const;
    void scheduleDelivery(const Delivery& delivery, const Order& order);
    bool hasSchedule(int orderId) const;
    void removeSchedule(int orderId) noexcept;
};

