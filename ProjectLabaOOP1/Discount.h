#pragma once

#include <memory>
#include <string>

class Product;
class Order;

// общий интерфейс для стратегий акций
class Discount
{
public:
    virtual ~Discount() = default;

    // каждая акция меняет заказ по своим правилам
    virtual void apply(Order& order) const = 0;

    // имя нужно для проверки повторного применения акции
    virtual std::string getName() const = 0;

    // создаём копию акции чтобы сохранить её в заказе
    virtual std::unique_ptr<Discount> clone() const = 0;
};

class PercentDiscount : public Discount
{
public:
    void apply(Order& order) const override;

    std::string getName() const override;

    std::unique_ptr<Discount> clone() const override;
};

class CashbackDiscount : public Discount
{
public:
    void apply(Order& order) const override;

    std::string getName() const override;

    std::unique_ptr<Discount> clone() const override;
};

class FreeDeliveryDiscount : public Discount
{
public:
    void apply(Order& order) const override;

    std::string getName() const override;

    std::unique_ptr<Discount> clone() const override;
};

class GiftDiscount : public Discount
{
private:
    std::shared_ptr<const Product> gift_;

public:
    // готовим образец бесплатного подарка из переданного товара
    explicit GiftDiscount(const Product& gift);

    void apply(Order& order) const override;

    std::string getName() const override;

    std::unique_ptr<Discount> clone() const override;
};