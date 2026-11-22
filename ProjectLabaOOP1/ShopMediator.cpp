// паттерн посредник связываем склад заказы оплату и доставку
#include "ShopMediator.h"
#include "ShopServices.h"
#include "OrderManager.h"
#include "LoggingProxy.h"
#include <cstdio>
#include <ostream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace
{
    // добавляем ID товара в сообщение посредника
    void logProductEvent(const char* action, int id) noexcept
    {
        char message[128];
        std::snprintf(message, sizeof message, "%s ID %d", action, id);
        LoggingProxy::log("MEDIATOR", message);
    }
}

ShopMediator::ShopMediator(ShopInventory& warehouse, OrderManager& orders,
    PaymentSystem& payment, DeliveryService& delivery)
    : warehouse_(warehouse), orders_(orders), payment_(payment), delivery_(delivery)
{
}

Order& ShopMediator::requireOrder(int id)
{
    auto* order = orders_.findById(id);
    if (!order)
    {
        throw std::out_of_range("Заказ не найден");
    }

    return *order;
}

// резервируем позиции и готовим оплату и доставку до регистрации заказа
int ShopMediator::createOrder(const std::string& customer,
    const std::vector<int>& ids, const std::string& deliveryType)
{
    if (ids.empty())
    {
        throw std::invalid_argument("Укажи хотя бы один товар");
    }

    auto delivery = delivery_.select(deliveryType);
    std::unordered_set<int> distinct;
    std::vector<std::shared_ptr<Product>> items;
    items.reserve(ids.size());

    // собираем позиции из доступных товаров и проверяем повторы
    for (const int id : ids)
    {
        if (!distinct.insert(id).second)
        {
            throw std::invalid_argument("ID товара в заказе не должен повторяться");
        }

        items.push_back(warehouse_.prepareItem(id));
    }

    // при отказе освобождаем только те позиции которые уже успели зарезервировать
    std::size_t reserved = 0;
    int preparedId = 0;
    try
    {
        for (const int id : ids)
        {
            warehouse_.reserve(id);
            ++reserved;
        }

        // передаём менеджеру подготовку которую он вызовет для нового заказа
        return orders_.createOrder(customer, items, std::move(delivery),
            [&](const Order& order)
            {
                payment_.requestPayment(order.getId(), order.getCustomer(), order.getTotalPrice());
                preparedId = order.getId();
                delivery_.scheduleDelivery(*order.getDelivery(), order);
            }).getId();
    }
    catch (...)
    {
        // при ошибке убираем созданный запрос оплаты и план доставки
        if (preparedId != 0)
        {
            payment_.refund(preparedId);
            delivery_.removeSchedule(preparedId);
        }

        while (reserved > 0)
        {
            warehouse_.release(ids[--reserved]);
        }

        throw;
    }
}

// списываем оплату и возвращаем её если статус не удалось поменять
void ShopMediator::confirmPayment(int id)
{
    const auto& order = requireOrder(id);
    if (order.getStatus() != OrderStatus::Pending)
    {
        throw std::logic_error("Оплатить можно только заказ в ожидании оплаты");
    }

    payment_.processPayment(id, order.getTotalPrice());
    try
    {
        orders_.confirmPayment(id);
    }
    catch (...)
    {
        payment_.resetPayment(id);
        throw;
    }
}

void ShopMediator::shipOrder(int id)
{
    const auto& order = requireOrder(id);
    if (order.getStatus() != OrderStatus::Paid || !order.getDelivery())
    {
        throw std::logic_error("Для отправки нужны оплата и способ доставки");
    }

    // перед отправкой повторно проверяем что доставку можно выполнить
    if (!delivery_.hasSchedule(id) || !delivery_.isAvailable(*order.getDelivery(), order))
    {
        throw std::logic_error("Доставка недоступна для заказа");
    }

    orders_.shipOrder(id);
    delivery_.removeSchedule(id);

    // при отправке списываем товар и одновременно закрываем резерв
    for (const auto& item : order.getItems())
    {
        warehouse_.consume(item->getId());
    }
}

// при отмене возвращаем оплату и освобождаем резерв и план доставки
void ShopMediator::cancelOrder(int id)
{
    const auto& order = requireOrder(id);
    const auto status = order.getStatus();
    if (status != OrderStatus::Pending && status != OrderStatus::Paid)
    {
        throw std::logic_error("Отменить можно только неотправленный заказ");
    }

    orders_.cancelOrder(id);
    payment_.refund(id);
    delivery_.removeSchedule(id);

    for (const auto& item : order.getItems())
    {
        warehouse_.release(item->getId());
    }
}

// добавляем загрузку складов к статистике заказов
void ShopMediator::getStatistics(std::ostream& output) const
{
    const auto stats = orders_.getStatistics();
    output << "Всего заказов: " << stats.orders
        << " | Активных: " << stats.activeOrders
        << " | Оборот: " << stats.turnover
        << " | Клиентов: " << stats.customers
        << " | Загрузка складов: " << warehouse_.getUsedSlots()
        << " из " << warehouse_.getTotalSlots() << '\n';
}

// перед удалением проверяем есть ли товар в резерве заказов
void ShopMediator::checkProductRemoval(int id) const
{
    if (warehouse_.reservedQuantity(id) != 0)
    {
        throw std::logic_error("Зарезервированный товар нельзя удалить");
    }
}

void ShopMediator::onProductAdded(int id) const noexcept
{
    logProductEvent("Товар добавлен", id);
}

void ShopMediator::onProductChanged(int id) const noexcept
{
    logProductEvent("Данные товара изменились", id);
}

void ShopMediator::onProductRemoved(int id) const noexcept
{
    logProductEvent("Товар удалён", id);
}

