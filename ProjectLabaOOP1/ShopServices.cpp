// подсистемы паттерна посредник оплата и доставка
// для расчёта доставки вызываем выбранную стратегию
#include "ShopServices.h"
#include "Delivery.h"
#include "Order.h"
#include <cmath>
#include <ostream>
#include <stdexcept>
#include <utility>

// из начального баланса вычитаем только подтверждённые платежи клиента
double PaymentSystem::checkBalance(const std::string& customer) const
{
    double balance = 1000000.0;
    for (const auto& entry : payments_)
    {
        if (entry.second.paid && entry.second.customer == customer)
        {
            balance -= entry.second.amount;
        }
    }

    return balance;
}

// запрос фиксирует сумму а деньги списываются при подтверждении
void PaymentSystem::requestPayment(int orderId, const std::string& customer, double amount)
{
    if (!std::isfinite(amount) || amount < 0)
    {
        throw std::runtime_error("Оплата отклонена");
    }

    if (!payments_.emplace(orderId, Payment{customer, amount}).second)
    {
        throw std::logic_error("Запрос оплаты уже зарегистрирован");
    }
}

// проверяем сумму запроса и баланс перед списанием
void PaymentSystem::processPayment(int orderId, double amount)
{
    auto entry = payments_.find(orderId);
    if (entry == payments_.end() || entry->second.paid)
    {
        throw std::logic_error("Нет запроса на оплату заказа");
    }

    auto& payment = entry->second;
    if (payment.amount != amount)
    {
        throw std::logic_error("Сумма заказа изменилась после запроса оплаты");
    }

    if (payment.amount > checkBalance(payment.customer))
    {
        throw std::runtime_error("Недостаточно средств для оплаты");
    }

    payment.paid = true;
}

// возвращаем запрос в ожидание если заказ не перешёл в оплаченный статус
void PaymentSystem::resetPayment(int orderId) noexcept
{
    const auto entry = payments_.find(orderId);
    if (entry != payments_.end())
    {
        entry->second.paid = false;
    }
}

void PaymentSystem::refund(int orderId) noexcept
{
    payments_.erase(orderId);
}

// создаём для заказа выбранный способ доставки
std::shared_ptr<Delivery> DeliveryService::select(const std::string& type) const
{
    if (type == "courier") return std::make_shared<CourierDelivery>(couriers_);
    if (type == "pickup") return std::make_shared<PickupDelivery>(pickupPoints_);
    if (type == "post") return std::make_shared<PostDelivery>(postOffices_);

    throw std::invalid_argument("Неизвестный способ доставки");
}

bool DeliveryService::isAvailable(const Delivery& delivery, const Order& order) const
{
    return delivery.isAvailable(order);
}

// стоимость считает конкретный способ доставки
double DeliveryService::calculateCost(const Delivery& delivery, const Order& order) const
{
    return delivery.getCost(order);
}

// сохраняем план доставки только для доступного способа
void DeliveryService::scheduleDelivery(const Delivery& delivery, const Order& order)
{
    if (!isAvailable(delivery, order))
    {
        throw std::runtime_error("Доставка недоступна для заказа");
    }

    if (!scheduled_.insert(order.getId()).second)
    {
        throw std::logic_error("Доставка уже запланирована");
    }
}

bool DeliveryService::hasSchedule(int orderId) const
{
    return scheduled_.find(orderId) != scheduled_.end();
}

void DeliveryService::removeSchedule(int orderId) noexcept
{
    scheduled_.erase(orderId);
}

