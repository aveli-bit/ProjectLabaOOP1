#pragma once // тут у наc "наблюдаемый" "наблюдатель" и "ивенты"

#include <chrono>

// возможные состояния заказа
enum class OrderStatus
{
    Pending,
    Paid,
    Shipped,
    Delivered,
    Cancelled
};

// событие хранит номер заказа старый и новый статус и время
struct OrderEvent
{
    int orderId;
    OrderStatus oldStatus;
    OrderStatus newStatus;
    std::chrono::system_clock::time_point timestamp;
};

class IObserver
{
public:
    virtual ~IObserver() = default;

    // подписчик получает событие и обрабатывает его
    virtual void onOrderEvent(const OrderEvent& event) = 0;
};

class IObservable
{
public:
    virtual ~IObservable() = default;

    // добавляем объект который будет получать события
    virtual void subscribe(IObserver* observer) = 0;

    // убираем объект из списка подписчиков
    virtual void unsubscribe(IObserver* observer) = 0;

    // передаём событие подписанным объектам
    virtual void notify(const OrderEvent& event) = 0;
};
