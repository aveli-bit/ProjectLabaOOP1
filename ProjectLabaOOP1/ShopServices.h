// подсистемы паттерна посредник оплата и доставка
// способы доставки используют паттерн стратегия из второй лабораторной
#pragma once

#include "Delivery.h"
#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <unordered_set>

class Order;
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

