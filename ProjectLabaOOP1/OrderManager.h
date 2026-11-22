#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Order.h"
//владеет заказами, проверяет лимиты ,хранит статистику
class OrderManager : public IObserver // так как менеджер публично наследуется от наблюдателя то заказ может использовать менеджера как
    // подписчика и сообщать ему у смене статуса заказа через ордер ивент
{
private:
    struct CustomerStats
    {
        std::size_t activeOrders = 0;
        double bonusBalance = 0.0;
    };

    struct OrderRecord
    {
        Order* order = nullptr;
        OrderStatus accountedStatus = OrderStatus::Pending;
        double paidAmount = 0.0;
        double creditedCashback = 0.0;
        std::chrono::system_clock::time_point paidAt{};
    };

    std::vector<std::unique_ptr<Order>> orders_;
    std::unordered_map<std::string, CustomerStats> customers_;
    std::unordered_map<int, OrderRecord> records_;

    double turnover_ = 0.0;
    std::size_t maxActiveOrders_;
    std::size_t maxTotalActiveOrders_;
    std::size_t activeOrders_ = 0;
    long long nextOrderId_ = 1;

    void checkNotNotifying() const;
    void swapState(OrderManager& other) noexcept;
    void rebindOrders(OrderManager* previous) noexcept;

public:
    struct Statistics
    {
        std::size_t orders;
        std::size_t activeOrders;
        double turnover;
        std::size_t customers;
    };

    // задаём лимит на клиента и на все активные заказы
    explicit OrderManager(
        std::size_t maxActiveOrders = 5,
        std::size_t maxTotalActiveOrders = 100
    );

    // копия получает отдельные заказы и подписывается на них сама
    OrderManager(const OrderManager& other);
    OrderManager& operator=(const OrderManager& other);

    // переносим заказы и обновляем указатель на менеджер в подписках
    OrderManager(OrderManager&& other);
    OrderManager& operator=(OrderManager&& other);

    ~OrderManager() override = default;

    // создаём заказ и копируем в него переданные позиции
    Order& createOrder(
        const std::string& customer,
        const std::vector<std::shared_ptr<Product>>& items = {},
        std::shared_ptr<Delivery> delivery = nullptr,
        const std::function<void(const Order&)>& prepare = {}
    );

    void updateStatus(int orderId, OrderStatus newStatus);
    void confirmPayment(int orderId);
    void shipOrder(int orderId);
    void cancelOrder(int orderId);
    Statistics getStatistics() const noexcept;

    Order* findById(int orderId) noexcept;
    const Order* findById(int orderId) const noexcept;

    // получаем общую статистику и бонусы клиента
    double getTurnover() const noexcept;
    std::size_t getCustomerCount() const noexcept;
    std::size_t getOrderCount() const noexcept;
    std::size_t getActiveOrderCount() const noexcept;
    std::size_t getActiveOrderCount(const std::string& customer) const;
    double getBonusBalance(const std::string& customer) const;
    void listActiveOrders(std::ostream& output) const;
    std::size_t getMaxActiveOrders() const noexcept;
    std::size_t getMaxTotalActiveOrders() const noexcept;

    // считаем неотменённые оплаты за период от начала включительно до конца исключительно
    double getTurnover(
        std::chrono::system_clock::time_point from,
        std::chrono::system_clock::time_point to
    ) const;

    // заказ вызывает этот метод при смене статуса
    void onOrderEvent(const OrderEvent& event) noexcept override;
};
