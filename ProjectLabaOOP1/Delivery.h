#pragma once

#include <string>
#include <memory>

class Order;

// тут общий интерфейс для разных способов доставки
class Delivery
{
public:
    virtual ~Delivery() = default;

    // создаём копию конкретного способа доставки для хранения в заказе
    virtual std::unique_ptr<Delivery> clone() const = 0;

    // получаем стоимость доставки для конкретного заказа
    virtual double getCost(const Order& order) const = 0;

    // возвращаем примерный срок доставки
    virtual std::string getEstimatedTime() const = 0;

    // проверяем доступность способа для заказа
    virtual bool isAvailable(const Order& order) const = 0;
};

class CourierDelivery : public Delivery
{
public:
    std::unique_ptr<Delivery> clone() const override;

    double getCost(const Order& order) const override;

    std::string getEstimatedTime() const override;

    bool isAvailable(const Order& order) const override;
};

class PickupDelivery : public Delivery
{
public:
    std::unique_ptr<Delivery> clone() const override;

    double getCost(const Order& order) const override;

    std::string getEstimatedTime() const override;

    bool isAvailable(const Order& order) const override;
};

class PostDelivery : public Delivery
{
public:
    std::unique_ptr<Delivery> clone() const override;

    double getCost(const Order& order) const override;

    std::string getEstimatedTime() const override;

    bool isAvailable(const Order& order) const override;
};

// прокси проверяет условия и передаёт вызовы реальной доставке
class DeliveryProxy : public Delivery
{
private:
    std::shared_ptr<Delivery> realDelivery_;
    std::string supportedRegion_;

    mutable bool cacheValid_ = false;
    mutable double cachedWeight_ = 0.0;
    mutable double cachedCost_ = 0.0;

public:
    explicit DeliveryProxy(
        std::shared_ptr<Delivery> realDelivery,
        const std::string& supportedRegion = "local"
    );

    std::unique_ptr<Delivery> clone() const override;

    double getCost(const Order& order) const override;

    std::string getEstimatedTime() const override;

    bool isAvailable(const Order& order) const override;
};
