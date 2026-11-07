#include "Discount.h"
#include "Order.h"
#include "LoggingProxy.h"
#include "Product.h"
#include <stdexcept>
#include <utility>

// устанавливаем скидку 10 процентов на товары заказа
void PercentDiscount::apply(Order& order) const
{
    order.setPercentDiscount(10.0);

    LoggingProxy::log(
        "DISCOUNT", "Применена процентная скидка 10%"
    );
}

std::string PercentDiscount::getName() const
{
    return "percent";
}

// создаём отдельную копию стратегии скидки
std::unique_ptr<Discount> PercentDiscount::clone() const
{
    return std::make_unique<PercentDiscount>(*this);
}

// задаём 5 процентов ожидаемых бонусов без уменьшения суммы к оплате
void CashbackDiscount::apply(Order& order) const
{
    order.setCashbackPercent(5.0);

    LoggingProxy::log(
        "DISCOUNT", "Применена акция cashback 5%"
    );
}

std::string CashbackDiscount::getName() const
{
    return "cashback";
}

std::unique_ptr<Discount> CashbackDiscount::clone() const
{
    return std::make_unique<CashbackDiscount>(*this);
}

// обнуляем оплату доставки но её ограничения остаются
void FreeDeliveryDiscount::apply(Order& order) const
{
    order.setFreeDelivery(true);

    LoggingProxy::log(
        "DISCOUNT", "Применена акция бесплатной доставки"
    );
}

std::string FreeDeliveryDiscount::getName() const
{
    return "free_delivery";
}

std::unique_ptr<Discount> FreeDeliveryDiscount::clone() const
{
    return std::make_unique<FreeDeliveryDiscount>(*this);
}

// создаём копию товара с нулевой ценой и количеством один
GiftDiscount::GiftDiscount(const Product& gift)
{
    auto copy = gift.clone();

    if (!copy)
    {
        throw std::runtime_error(
            "Не удалось создать копию подарка"
        );
    }

    copy->setPrice(Price(0));

    copy->changeQuantity(
        1 - copy->getQuantity().getValue()
    );

    gift_ = std::shared_ptr<const Product>(std::move(copy));
}

// добавляем в заказ отдельную копию подарка
void GiftDiscount::apply(Order& order) const
{
    if (!gift_)
    {
        throw std::runtime_error(
            "Подарок не задан"
        );
    }

    order.addItem(*gift_, 1);

    LoggingProxy::log(
        "DISCOUNT", "В заказ добавлен бесплатный подарок"
    );
}

std::string GiftDiscount::getName() const
{
    return "gift";
}

// копируем акцию с общим неизменяемым образцом подарка
std::unique_ptr<Discount> GiftDiscount::clone() const
{
    return std::make_unique<GiftDiscount>(*this);
}