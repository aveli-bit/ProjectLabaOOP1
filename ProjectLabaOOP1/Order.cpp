#include "Order.h"
#include "LoggingProxy.h"
#include "OrderManager.h"
#include <algorithm> // std::find_if
#include <utility> // std::move.
#include <cmath>
#include <stdexcept>
#include <ostream>

namespace
{
    // получаем название статуса для вывода заказа
    const char* orderStatusName(OrderStatus status) noexcept
    {
        switch (status)
        {
        case OrderStatus::Pending:
            return "Pending";
        case OrderStatus::Paid:
            return "Paid";
        case OrderStatus::Shipped:
            return "Shipped";
        case OrderStatus::Delivered:
            return "Delivered";
        case OrderStatus::Cancelled:
            return "Cancelled";
        }

        return "Unknown";
    }
}


// сохраняем номер и клиента и проверяем их значения
Order::Order(int id, const std::string& customer)
    : id_(id),
    customer_(customer)
{
    if (id <= 0)
    {
        throw std::invalid_argument(
            "ID заказа должен быть положительным"
        );
    }

    if (customer.empty())
    {
        throw std::invalid_argument(
            "Идентификатор клиента не должен быть пустым"
        );
    }

    LoggingProxy::log("ORDER", "Заказ создан");
}

// создаём отдельные товары доставку и акции без подписчиков исходного заказа
Order::Order(const Order& other)
    : id_(other.id_),
      customer_(other.customer_),
      region_(other.region_),
      status_(other.status_),
      percentDiscount_(other.percentDiscount_),
      cashbackPercent_(other.cashbackPercent_),
      paidCashback_(other.paidCashback_),
      freeDelivery_(other.freeDelivery_),
      totalPrice_(other.totalPrice_)
{
    if (other.notifying_)
    {
        throw std::logic_error("Нельзя копировать заказ во время уведомления");
    }

    items_.reserve(other.items_.size());

    for (const auto& item : other.items_)
    {
        auto copy = item->clone();

        if (!copy)
        {
            throw std::runtime_error("Не удалось скопировать товар заказа");
        }

        items_.push_back(std::shared_ptr<Product>(std::move(copy)));
    }

    if (other.delivery_)
    {
        auto copy = other.delivery_->clone();

        if (!copy)
        {
            throw std::runtime_error("Не удалось скопировать доставку заказа");
        }

        delivery_ = std::shared_ptr<Delivery>(std::move(copy));
    }

    discounts_.reserve(other.discounts_.size());

    for (const auto& discount : other.discounts_)
    {
        auto copy = discount->clone();

        if (!copy)
        {
            throw std::runtime_error("Не удалось скопировать акцию заказа");
        }

        discounts_.push_back(std::shared_ptr<const Discount>(std::move(copy)));
    }

    LoggingProxy::log("ORDER", "Создана глубокая копия заказа");
}

// сначала готовим копию а затем заменяем данные сохраняя подписчиков получателя
Order& Order::operator=(const Order& other)
{
    if (this != &other)
    {
        checkCanReplace();
        Order copy(other);
        swapData(copy);
    }

    return *this;
}

// переносим содержимое без клонирования товаров
Order::Order(Order&& other)
    : id_(0)
{
    other.checkCanReplace();
    swapData(other);
    observers_.swap(other.observers_);
    other.resetMovedFrom();

    LoggingProxy::log("ORDER", "Заказ перемещён через конструктор");
}

Order& Order::operator=(Order&& other)
{
    if (this != &other)
    {
        checkCanReplace();
        other.checkCanReplace();
        swapData(other);
        observers_.swap(other.observers_);
        other.resetMovedFrom();
    }

    return *this;
}

// заказ внутри менеджера заменяем только через операции самого менеджера
void Order::checkCanReplace() const
{
    if (notifying_)
    {
        throw std::logic_error("Нельзя заменять заказ во время уведомления");
    }

    if (manager_ != nullptr)
    {
        throw std::logic_error("Нельзя перемещать или заменять заказ внутри менеджера");
    }
}

// обмениваем данные без смены владельца и подписчиков
void Order::swapData(Order& other) noexcept
{
    std::swap(id_, other.id_);
    customer_.swap(other.customer_);
    region_.swap(other.region_);
    items_.swap(other.items_);
    std::swap(status_, other.status_);
    delivery_.swap(other.delivery_);
    discounts_.swap(other.discounts_);
    std::swap(percentDiscount_, other.percentDiscount_);
    std::swap(cashbackPercent_, other.cashbackPercent_);
    std::swap(paidCashback_, other.paidCashback_);
    std::swap(freeDelivery_, other.freeDelivery_);
    std::swap(totalPrice_, other.totalPrice_);
}

// оставляем источник пустым и пригодным для нового присваивания
void Order::resetMovedFrom() noexcept
{
    id_ = 0;
    customer_.clear();
    region_.clear();
    items_.clear();
    status_ = OrderStatus::Cancelled;
    observers_.clear();
    delivery_.reset();
    discounts_.clear();
    percentDiscount_ = 0.0;
    cashbackPercent_ = 0.0;
    paidCashback_ = 0.0;
    freeDelivery_ = false;
    totalPrice_ = 0.0;
}

// эти методы возвращают ID заказа его клиента и статус
int Order::getId() const
{
    return id_;
}

const std::string& Order::getCustomer() const
{
    return customer_;
}

OrderStatus Order::getStatus() const
{
    return status_;
}

// возвращаем список позиций без копирования вектора

const std::vector<std::shared_ptr<const Product>>& 

Order::getItems() const
{
    return items_;
}

double Order::getTotalPrice() const

{
    return totalPrice_;
}

// складываем вес всех позиций заказа
double Order::getTotalWeight() const
{
    double totalWeight = 0.0;

    for (const auto& item : items_)
    {
        totalWeight += item->getTotalWeight();

        if (!std::isfinite(totalWeight))
        {
            throw std::overflow_error(
                "Общий вес заказа слишком большой"
            );
        }
    }

    return totalWeight;
}
// добавляем копию товара с нужным количеством и пересчитываем итог
void Order::addItem(const Product& product, int quantity)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Состав заказа можно менять только до оплаты"
        );
    }

    if (quantity <= 0)
    {
        throw std::invalid_argument(
            "Количество в заказе должно быть положительным"
        );
    }

    for (const auto& item : items_)
    {
        if (item->getId() == product.getId())
        {
            throw std::invalid_argument(
                "Товар с таким ID уже есть в заказе"
            );
        }
    }

    auto copy = product.clone();

    copy->changeQuantity(
        quantity - copy->getQuantity().getValue()
    );

    const double newTotalWeight = getTotalWeight()
        + copy->getTotalWeight();

    if (!std::isfinite(newTotalWeight))
    {
        throw std::overflow_error(
            "Вес заказа слишком большой"
        );
    }

    std::shared_ptr<Product> item(std::move(copy));

    items_.push_back(std::move(item));

    try
    {
        totalPrice_ = calculateTotalPrice(delivery_);
    }
    catch (...)
    {
        items_.pop_back(); // удаляет  только что добавленную позицию
        throw;
    }

    LoggingProxy::log("ORDER", "Товар добавлен в заказ");

}
// удаляем позицию по ID и возвращаем её назад при ошибке пересчёта
void Order::removeItem(int productId)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Состав заказа можно менять только до оплаты"
        );
    }

    auto it = std::find_if(
        items_.begin(),
        items_.end(),
        [productId](const std::shared_ptr<const Product>& item)
        {
            return item->getId() == productId;
        }
    );

    if (it == items_.end())
    {
        throw std::out_of_range(
            "Товар не найден в заказе"
        );
    }

    const auto index = it - items_.begin();
    auto removedItem = *it;

    items_.erase(it);

    try
    {
        totalPrice_ = calculateTotalPrice(delivery_);
    }
    catch (...)
    {
        items_.insert(
            items_.begin() + index,
            std::move(removedItem)
        );

        throw;
    }

    LoggingProxy::log("ORDER", "Товар удалён из заказа");
}

// проверяем доставку и считаем сумму с учётом скидки и бесплатной доставки
double Order::calculateTotalPrice(
    const std::shared_ptr<Delivery>& delivery) const
{
    double result = calculateItemsPrice();  

    if (delivery)
    {
        if (!delivery->isAvailable(*this))
        {
            throw std::runtime_error(
                "Выбранная доставка недоступна для заказа"
            );
        }

        const double deliveryCost = delivery->getCost(*this); // this указатель на текущий заказ а *this сам текущий, именно его передаём доставке для проверки веса

        if (!std::isfinite(deliveryCost) || deliveryCost < 0.0)
        {
            throw std::runtime_error(
                "Некорректная стоимость доставки"
            );
        }

        if (!freeDelivery_)
        {
            result += deliveryCost;
        }
    }

    if (!std::isfinite(result))
    {
        throw std::overflow_error(
            "Стоимость заказа слишком большая"
        );
    }

    return result;
}
// сначала проверяем новый способ доставки потом сохраняем его в заказе
void Order::setDelivery(std::shared_ptr<Delivery> delivery)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Доставку можно менять только до оплаты"
        );
    }

    if (!delivery)
    {
        throw std::invalid_argument(
            "Нельзя назначить пустую доставку"
        );
    }

    const double newTotalPrice = calculateTotalPrice(delivery);

    delivery_ = std::move(delivery); // std::move позволяет передать указатель в поле,перемещается shared_ptr а не создаётся копия объекта доставки
    totalPrice_ = newTotalPrice;

    LoggingProxy::log("ORDER", "Назначен способ доставки");
}
// проверяем повтор акции и применяем её копию к заказу
void Order::applyDiscount(const Discount& discount)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Акции можно применять только до оплаты"
        );
    }

    const std::string name = discount.getName();

    for (const auto& applied : discounts_)
    {
        if (applied->getName() == name)
        {
            throw std::invalid_argument(
                "Такая акция уже применена к заказу"
            );
        }
    }

    std::shared_ptr<const Discount> copy(discount.clone());

    if (!copy)
    {
        throw std::runtime_error(
            "Не удалось создать копию акции"
        );
    }

    discounts_.reserve(discounts_.size() + 1);

    copy->apply(*this);

    discounts_.push_back(std::move(copy));

    LoggingProxy::log("ORDER", "Акция добавлена к заказу");
}
// меняем процент скидки и откатываем его если пересчёт не прошёл
void Order::setPercentDiscount(double percent)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Скидку можно менять только до оплаты"
        );
    }

    if (!std::isfinite(percent)
        || percent < 0.0
        || percent > 100.0)
    {
        throw std::invalid_argument(
            "Процент скидки должен быть от 0 до 100"
        );
    }

    const double oldPercent = percentDiscount_;

    percentDiscount_ = percent;

    try
    {
        totalPrice_ = calculateTotalPrice(delivery_);
    }
    catch (...)
    {
        percentDiscount_ = oldPercent;
        throw;
    }
}
// считаем товары со скидкой и округляем сумму до копеек
double Order::calculateItemsPrice() const
{
    double result = 0.0;

    for (const auto& item : items_)
    {
        result += static_cast<double>(item->getTotalPrice());
    }

    if (!std::isfinite(result))
    {
        throw std::overflow_error(
            "Стоимость товаров слишком большая"
        );
    }

    result *= 1.0 - percentDiscount_ / 100.0;

    result = std::round(result * 100.0) / 100.0;

    if (!std::isfinite(result))
    {
        throw std::overflow_error(
            "Ошибка округления стоимости товаров"
        );
    }

    return result;
}
// проверяем и сохраняем процент будущих бонусов
void Order::setCashbackPercent(double percent)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Cashback можно менять только до оплаты"
        );
    }

    if (!std::isfinite(percent)
        || percent < 0.0
        || percent > 100.0)
    {
        throw std::invalid_argument(
            "Процент cashback должен быть от 0 до 100"
        );
    }

    cashbackPercent_ = percent;
}
// считаем ожидаемые бонусы от стоимости товаров после скидки
double Order::getCashback() const
{
    const double cashback = calculateItemsPrice()
        * (cashbackPercent_ / 100.0);

    const double roundedCashback =
        std::round(cashback * 100.0) / 100.0;

    if (!std::isfinite(roundedCashback))
    {
        throw std::overflow_error(
            "Сумма cashback слишком большая"
        );
    }

    return roundedCashback;
}

double Order::getPaidCashback() const noexcept
{
    return paidCashback_;
}

// включаем или выключаем бесплатную доставку и пересчитываем итог
void Order::setFreeDelivery(bool enabled)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Условия доставки можно менять только до оплаты"
        );
    }

    const bool oldFreeDelivery = freeDelivery_;

    freeDelivery_ = enabled;

    try
    {
        totalPrice_ = calculateTotalPrice(delivery_);
    }
    catch (...)
    {
        freeDelivery_ = oldFreeDelivery;
        throw;
    }
}

const std::string& Order::getRegion() const
{
    return region_;
}

// меняем регион и возвращаем старый если доставка стала недоступна
void Order::setRegion(const std::string& region)
{
    if (status_ != OrderStatus::Pending)
    {
        throw std::runtime_error(
            "Регион можно менять только до оплаты"
        );
    }

    if (region.empty())
    {
        throw std::invalid_argument(
            "Регион не должен быть пустым"
        );
    }

    std::string newRegion = region;

    region_.swap(newRegion);

    try
    {
        totalPrice_ = calculateTotalPrice(delivery_);
    }
    catch (...)
    {
        region_.swap(newRegion);
        throw;
    }

    LoggingProxy::log("ORDER", "Регион заказа изменён");
}

// добавляем подписчика если такого указателя ещё нет в списке
void Order::subscribe(IObserver* observer)
{
    if (notifying_)
    {
        throw std::logic_error(
            "Нельзя менять подписки во время уведомления"
        );
    }

    if (observer == nullptr)
    {
        throw std::invalid_argument(
            "Нельзя подписать пустой указатель"
        );
    }

    auto it = std::find(
        observers_.begin(),
        observers_.end(),
        observer
    );

    if (it == observers_.end())
    {
        observers_.push_back(observer);
    }
}

// удаляем указатель из списка без удаления самого подписчика
void Order::unsubscribe(IObserver* observer)
{
    if (notifying_)
    {
        throw std::logic_error(
            "Нельзя менять подписки во время уведомления"
        );
    }

    if (manager_ != nullptr && observer == manager_)
    {
        throw std::logic_error("Нельзя отписать менеджер который владеет заказом");
    }

    observers_.erase(
        std::remove(
            observers_.begin(),
            observers_.end(),
            observer
        ),
        observers_.end()
    );
}

// передаём событие каждому подписчику и перехватываем ошибки обработки
void Order::notify(const OrderEvent& event) noexcept
{
    if (notifying_)
    {
        LoggingProxy::log(
            "ORDER", "Повторное уведомление пропущено"
        );

        return;
    }

    notifying_ = true;

    for (IObserver* observer : observers_)
    {
        try
        {
            observer->onOrderEvent(event);
        }
        catch (...)
        {
            LoggingProxy::log(
                "ORDER", "Ошибка при обработке события подписчиком"
            );
        }
    }

    notifying_ = false;
}

// проверяем переход статуса и сообщаем подписчикам об изменении
void Order::setStatus(OrderStatus newStatus)
{
    if (notifying_)
    {
        throw std::logic_error(
            "Нельзя менять статус во время уведомления"
        );
    }

    if (newStatus == status_)
    {
        return;
    }

    const bool allowed =
        (status_ == OrderStatus::Pending
            && (newStatus == OrderStatus::Paid
                || newStatus == OrderStatus::Cancelled))
        ||
        (status_ == OrderStatus::Paid
            && (newStatus == OrderStatus::Shipped
                || newStatus == OrderStatus::Cancelled))
        ||
        (status_ == OrderStatus::Shipped
            && newStatus == OrderStatus::Delivered);

    if (!allowed)
    {
        throw std::invalid_argument(
            "Недопустимый переход статуса заказа"
        );
    }

    if (newStatus == OrderStatus::Paid && items_.empty())
    {
        throw std::runtime_error(
            "Нельзя оплатить пустой заказ"
        );
    }

    // готовим бонусы до смены статуса чтобы ошибка расчёта не оставила оплату без учёта
    const double paidCashback = newStatus == OrderStatus::Paid
        ? getCashback() : paidCashback_;

    const OrderEvent event{
        id_,
        status_,
        newStatus,
        std::chrono::system_clock::now()
    };

    paidCashback_ = paidCashback;
    status_ = newStatus;

    LoggingProxy::log("ORDER", "Статус заказа изменён");

    notify(event);
}

// используем обычное применение акции со всеми его проверками
Order& operator+(Order& order, const Discount& discount)
{
    order.applyDiscount(discount);
    return order;
}

// виртуальная копия доставки продолжит жить внутри заказа
Order& operator<<(Delivery& delivery, Order& order)
{
    order.setDelivery(std::shared_ptr<Delivery>(delivery.clone()));
    return order;
}

// читаем данные заказа без изменения его состояния
std::ostream& operator<<(std::ostream& output, const Order& order)
{
    const double weight = order.getTotalWeight();

    output << "Заказ #" << order.getId()
        << " | Клиент: " << order.getCustomer()
        << " | Статус: " << orderStatusName(order.getStatus())
        << " | Регион: " << order.getRegion()
        << " | Позиций: " << order.getItems().size()
        << " | Вес: " << weight
        << " | К оплате: " << order.getTotalPrice();

    return output;
}
