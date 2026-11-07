#include "Delivery.h"
#include "Order.h"
#include "LoggingProxy.h"
#include <utility>
#include <cmath>
#include <stdexcept>

// копируем курьерскую доставку с сохранением конкретного типа
std::unique_ptr<Delivery> CourierDelivery::clone() const
{
    return std::make_unique<CourierDelivery>(*this);
}

// проверяем доступность курьера и возвращаем стоимость 500 рублей
double CourierDelivery::getCost(const Order& order) const
{
    if (!isAvailable(order))
    {
        throw std::runtime_error(
            "Курьерская доставка недоступна: вес превышает 20 кг"
        );
    }

    LoggingProxy::log(
        "DELIVERY", "Рассчитана стоимость курьерской доставки"
    );

    return 500.0;
}

std::string CourierDelivery::getEstimatedTime() const
{
    return "1-2 дня";
}

// курьер доступен для веса до 20 кг включительно
bool CourierDelivery::isAvailable(const Order& order) const
{
    return order.getTotalWeight() <= 20.0;
}

// создаём отдельный объект самовывоза
std::unique_ptr<Delivery> PickupDelivery::clone() const
{
    return std::make_unique<PickupDelivery>(*this);
}

// для самовывоза стоимость доставки равна нулю
double PickupDelivery::getCost(const Order&) const
{
    LoggingProxy::log(
        "DELIVERY", "Рассчитана стоимость самовывоза"
    );

    return 0.0;
}

std::string PickupDelivery::getEstimatedTime() const
{
    return "1-3 дня";
}

// у самовывоза нет ограничения по весу
bool PickupDelivery::isAvailable(const Order&) const
{
    return true;
}

// создаём отдельный объект почтовой доставки
std::unique_ptr<Delivery> PostDelivery::clone() const
{
    return std::make_unique<PostDelivery>(*this);
}

// проверяем доступность почты и возвращаем стоимость 200 рублей
double PostDelivery::getCost(const Order& order) const
{
    if (!isAvailable(order))
    {
        throw std::runtime_error(
            "Почтовая доставка недоступна: вес превышает 10 кг"
        );
    }

    LoggingProxy::log(
        "DELIVERY", "Рассчитана стоимость почтовой доставки"
    );

    return 200.0;
}

std::string PostDelivery::getEstimatedTime() const
{
    return "5-10 дней";
}

// почта доступна для веса до 10 кг включительно
bool PostDelivery::isAvailable(const Order& order) const
{
    return order.getTotalWeight() <= 10.0;
}

// сохраняем реальную доставку и обслуживаемый регион
DeliveryProxy::DeliveryProxy(
    std::shared_ptr<Delivery> realDelivery,
    const std::string& supportedRegion
)
    : realDelivery_(std::move(realDelivery)),
    supportedRegion_(supportedRegion)
{
    if (!realDelivery_)
    {
        throw std::invalid_argument(
            "Прокси должен получить реальную доставку"
        );
    }

    if (supportedRegion_.empty())
    {
        throw std::invalid_argument(
            "Регион доставки не должен быть пустым"
        );
    }
}

// копируем прокси вместе с реальной доставкой а кэш заполним заново
std::unique_ptr<Delivery> DeliveryProxy::clone() const
{
    std::shared_ptr<Delivery> realCopy(realDelivery_->clone());

    return std::make_unique<DeliveryProxy>(
        std::move(realCopy), supportedRegion_
    );
}

// сначала проверяем регион потом ограничения реальной доставки
bool DeliveryProxy::isAvailable(const Order& order) const
{
    if (order.getRegion() != supportedRegion_)
    {
        return false;
    }

    return realDelivery_->isAvailable(order);
}

// берём стоимость из кэша или рассчитываем через реальную доставку
double DeliveryProxy::getCost(const Order& order) const
{
    LoggingProxy::log(
        "DELIVERY", "Прокси получил запрос стоимости"
    );

    if (!isAvailable(order))
    {
        throw std::runtime_error(
            "Доставка недоступна по весу или региону"
        );
    }

    const double weight = order.getTotalWeight();

    if (cacheValid_ && cachedWeight_ == weight)
    {
        LoggingProxy::log(
            "DELIVERY", "Стоимость получена из кэша"
        );

        return cachedCost_;
    }

    const double cost = realDelivery_->getCost(order);

    if (!std::isfinite(cost) || cost < 0.0)
    {
        throw std::runtime_error(
            "Доставка вернула некорректную стоимость"
        );
    }

    cachedWeight_ = weight;
    cachedCost_ = cost;
    cacheValid_ = true;

    LoggingProxy::log(
        "DELIVERY", "Стоимость сохранена в кэше"
    );

    return cost;
}

// получаем срок у реального способа доставки
std::string DeliveryProxy::getEstimatedTime() const
{
    return realDelivery_->getEstimatedTime();
}
