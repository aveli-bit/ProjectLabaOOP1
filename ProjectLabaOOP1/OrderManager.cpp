#include "OrderManager.h"
#include "LoggingProxy.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

OrderManager::OrderManager(
    std::size_t maxActiveOrders,
    std::size_t maxTotalActiveOrders
)
    : maxActiveOrders_(maxActiveOrders),
      maxTotalActiveOrders_(maxTotalActiveOrders)
{
    if (maxActiveOrders == 0 || maxTotalActiveOrders == 0)
    {
        throw std::invalid_argument(
            "Лимиты активных заказов должны быть положительными"
        );
    }

    LoggingProxy::log("MANAGER", "Менеджер заказов создан");
}

// копируем статистику без повторного начисления оплат и бонусов
OrderManager::OrderManager(const OrderManager& other)
    : customers_(other.customers_),
      turnover_(other.turnover_),
      maxActiveOrders_(other.maxActiveOrders_),
      maxTotalActiveOrders_(other.maxTotalActiveOrders_),
      activeOrders_(other.activeOrders_),
      nextOrderId_(other.nextOrderId_)
{
    other.checkNotNotifying();
    orders_.reserve(other.orders_.size());
    records_.reserve(other.records_.size());

    for (const auto& order : other.orders_)
    {
        auto copy = std::make_unique<Order>(*order);
        copy->subscribe(this);
        copy->manager_ = this;

        OrderRecord record = other.records_.at(order->getId());
        record.order = copy.get();
        records_.emplace(copy->getId(), record);
        orders_.push_back(std::move(copy));
    }

    LoggingProxy::log("MANAGER", "Создана глубокая копия менеджера");
}

// временная копия сохраняет старый менеджер если подготовка завершилась ошибкой
OrderManager& OrderManager::operator=(const OrderManager& other)
{
    if (this != &other)
    {
        checkNotNotifying();
        OrderManager copy(other);
        swapState(copy);
    }

    return *this;
}

// сами объекты заказов сохраняют свои адреса при переносе менеджера
OrderManager::OrderManager(OrderManager&& other)
    : maxActiveOrders_(other.maxActiveOrders_),
      maxTotalActiveOrders_(other.maxTotalActiveOrders_)
{
    other.checkNotNotifying();
    swapState(other);

    LoggingProxy::log("MANAGER", "Менеджер заказов перемещён");
}

OrderManager& OrderManager::operator=(OrderManager&& other)
{
    if (this != &other)
    {
        checkNotNotifying();
        other.checkNotNotifying();
        OrderManager moved(std::move(other));
        swapState(moved);
    }

    return *this;
}

// во время уведомления нельзя менять владельца выполняющегося заказа
void OrderManager::checkNotNotifying() const
{
    for (const auto& order : orders_)
    {
        if (order->notifying_)
        {
            throw std::logic_error("Нельзя заменять менеджер во время уведомления");
        }
    }
}

// переносим контейнеры и сразу связываем заказы с их новым менеджером
void OrderManager::swapState(OrderManager& other) noexcept
{
    orders_.swap(other.orders_);
    customers_.swap(other.customers_);
    records_.swap(other.records_);
    std::swap(turnover_, other.turnover_);
    std::swap(maxActiveOrders_, other.maxActiveOrders_);
    std::swap(maxTotalActiveOrders_, other.maxTotalActiveOrders_);
    std::swap(activeOrders_, other.activeOrders_);
    std::swap(nextOrderId_, other.nextOrderId_);

    rebindOrders(&other);
    other.rebindOrders(this);
}

// заменяем указатель на прежний менеджер без новых подписок и выделения памяти
void OrderManager::rebindOrders(OrderManager* previous) noexcept
{
    for (auto& order : orders_)
    {
        order->manager_ = this;

        for (IObserver*& observer : order->observers_)
        {
            if (observer == previous)
            {
                observer = this;
            }
        }
    }
}

// сначала выполняем проверки и подготовку а потом сохраняем заказ в менеджере
Order& OrderManager::createOrder(
    const std::string& customer,
    const std::vector<std::shared_ptr<Product>>& items
)
{
    if (customer.empty())
    {
        throw std::invalid_argument(
            "Идентификатор клиента не должен быть пустым"
        );
    }

    const auto customerIt = customers_.find(customer);

    if (customerIt != customers_.end()
        && customerIt->second.activeOrders >= maxActiveOrders_)
    {
        throw std::runtime_error(
            "Достигнут лимит активных заказов клиента"
        );
    }

    if (activeOrders_ >= maxTotalActiveOrders_)
    {
        throw std::runtime_error(
            "Достигнут общий лимит активных заказов"
        );
    }

    if (nextOrderId_ > (std::numeric_limits<int>::max)())
    {
        throw std::overflow_error("Закончились доступные ID заказов");
    }

    auto order = std::make_unique<Order>(
        static_cast<int>(nextOrderId_), customer
    );

    for (const auto& item : items)
    {
        if (!item)
        {
            throw std::invalid_argument(
                "В заказе не должно быть пустых указателей на товары"
            );
        }

        order->addItem(*item, item->getQuantity().getValue());
    }

    order->subscribe(this);

    // увеличиваем вместимость с запасом чтобы не перемещать указатели при каждом заказе
    if (orders_.size() == orders_.capacity())
    {
        const auto maxSize = orders_.max_size();

        if (orders_.size() == maxSize)
        {
            throw std::length_error("Достигнут предел хранения заказов");
        }

        const auto newCapacity = orders_.capacity() > maxSize / 2
            ? maxSize
            : (orders_.capacity() == 0 ? 1 : orders_.capacity() * 2);

        orders_.reserve(newCapacity);
    }

    auto insertedCustomer = customers_.emplace(customer, CustomerStats{});

    try
    {
        OrderRecord record;
        record.order = order.get();

        if (!records_.emplace(order->getId(), record).second)
        {
            throw std::logic_error("Повторный ID заказа в менеджере");
        }
    }
    catch (...)
    {
        if (insertedCustomer.second)
        {
            customers_.erase(insertedCustomer.first);
        }

        throw;
    }

    // место в векторе уже выделено а перемещение unique_ptr не бросает
    orders_.push_back(std::move(order));
    orders_.back()->manager_ = this;
    ++insertedCustomer.first->second.activeOrders;
    ++activeOrders_;
    ++nextOrderId_;

    LoggingProxy::log("MANAGER", "Заказ принят и менеджер подписан на события");

    return *orders_.back();
}

// статус меняет сам заказ а статистика обновляется через событие
void OrderManager::updateStatus(int orderId, OrderStatus newStatus)
{
    Order* order = findById(orderId);

    if (order == nullptr)
    {
        throw std::out_of_range("Заказ не найден в менеджере");
    }

    order->setStatus(newStatus);
}

Order* OrderManager::findById(int orderId) noexcept
{
    const auto it = records_.find(orderId);
    return it == records_.end() ? nullptr : it->second.order;
}

const Order* OrderManager::findById(int orderId) const noexcept
{
    const auto it = records_.find(orderId);
    return it == records_.end() ? nullptr : it->second.order;
}

double OrderManager::getTurnover() const noexcept
{
    return turnover_;
}

std::size_t OrderManager::getCustomerCount() const noexcept
{
    return customers_.size();
}

std::size_t OrderManager::getOrderCount() const noexcept
{
    return orders_.size();
}

std::size_t OrderManager::getActiveOrderCount() const noexcept
{
    return activeOrders_;
}

std::size_t OrderManager::getActiveOrderCount(
    const std::string& customer) const
{
    const auto it = customers_.find(customer);
    return it == customers_.end() ? 0 : it->second.activeOrders;
}

double OrderManager::getBonusBalance(const std::string& customer) const
{
    const auto it = customers_.find(customer);
    return it == customers_.end() ? 0.0 : it->second.bonusBalance;
}

double OrderManager::getTurnover(
    std::chrono::system_clock::time_point from,
    std::chrono::system_clock::time_point to
) const
{
    if (from > to)
    {
        throw std::invalid_argument("Начало периода должно быть не позже конца");
    }

    double result = 0.0;

    for (const auto& entry : records_)
    {
        const auto& record = entry.second;

        if (record.paidAt >= from && record.paidAt < to)
        {
            result += record.paidAmount;
        }
    }

    return std::round(result * 100.0) / 100.0;
}

// обработчик использует готовые записи без добавления элементов в контейнеры
void OrderManager::onOrderEvent(const OrderEvent& event) noexcept
{
    const auto it = records_.find(event.orderId);

    if (it == records_.end())
    {
        LoggingProxy::log("MANAGER", "Событие чужого заказа пропущено");
        return;
    }

    auto& record = it->second;

    if (event.oldStatus == event.newStatus
        || record.accountedStatus != event.oldStatus
        || record.order->getStatus() != event.newStatus)
    {
        LoggingProxy::log("MANAGER", "Повторное или несогласованное событие пропущено");
        return;
    }

    const auto customerIt = customers_.find(record.order->getCustomer());

    if (customerIt == customers_.end())
    {
        LoggingProxy::log("MANAGER", "Не найден клиент события");
        return;
    }

    auto& customer = customerIt->second;

    if (event.newStatus == OrderStatus::Paid)
    {
        record.paidAmount = record.order->getTotalPrice();
        record.creditedCashback = record.order->getPaidCashback();
        record.paidAt = event.timestamp;

        turnover_ = std::round((turnover_ + record.paidAmount) * 100.0) / 100.0;
        customer.bonusBalance = std::round(
            (customer.bonusBalance + record.creditedCashback) * 100.0
        ) / 100.0;

        LoggingProxy::log("MANAGER", "Оплата учтена и cashback начислен");
    }

    if (event.newStatus == OrderStatus::Cancelled)
    {
        turnover_ = std::round((turnover_ - record.paidAmount) * 100.0) / 100.0;
        customer.bonusBalance = std::round(
            (customer.bonusBalance - record.creditedCashback) * 100.0
        ) / 100.0;

        record.paidAmount = 0.0;
        record.creditedCashback = 0.0;

        LoggingProxy::log("MANAGER", "Отмена учтена и начисления отменены");
    }

    if (event.newStatus == OrderStatus::Delivered
        || event.newStatus == OrderStatus::Cancelled)
    {
        --customer.activeOrders;
        --activeOrders_;

        LoggingProxy::log("MANAGER", "Освобождён лимит активных заказов");
    }

    record.accountedStatus = event.newStatus;
}
