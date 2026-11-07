#pragma once

#include <memory> // shared_ptr
#include <iosfwd>
#include <string>
#include <vector>
#include "Product.h"
#include "OrderEvents.h"
#include "Delivery.h"
#include "Discount.h"

class OrderManager;

class Order : public IObservable
{
private:
    friend class OrderManager;

    int id_;
    std::string customer_;
    std::string region_ = "local";
    std::vector<std::shared_ptr<const Product>> items_;
   
    OrderStatus status_ = OrderStatus::Pending;
    std::vector<IObserver*> observers_;
    bool notifying_ = false;
    OrderManager* manager_ = nullptr;

    void checkCanReplace() const;
    void swapData(Order& other) noexcept;
    void resetMovedFrom() noexcept;
    
    // считаем итог с учётом товаров скидки и доставки
    double calculateTotalPrice(
        const std::shared_ptr<Delivery>& delivery) const;

    std::shared_ptr<Delivery> delivery_;
    std::vector<std::shared_ptr<const Discount>> discounts_;

    double percentDiscount_ = 0.0;
    double cashbackPercent_ = 0.0;
    double paidCashback_ = 0.0;
    double calculateItemsPrice() const;
    bool freeDelivery_ = false;

    double totalPrice_ = 0.0;

public:
    // создаём заказ для указанного клиента
    Order(int id, const std::string& customer);

    // копируем данные заказа а подписки на исходник не переносим
    Order(const Order& other);
    Order& operator=(const Order& other);

    // переносим данные и подписки самостоятельного заказа
    Order(Order&& other);
    Order& operator=(Order&& other);

    ~Order() override = default;

    // применяем акцию и сохраняем её в заказе
    void applyDiscount(const Discount& discount);
    void setPercentDiscount(double percent);
    void setCashbackPercent(double percent);
    void setFreeDelivery(bool enabled);
    void setRegion(const std::string& region);
    void setStatus(OrderStatus newStatus);
    void subscribe(IObserver* observer) override;
    void unsubscribe(IObserver* observer) override;
    void notify(const OrderEvent& event) noexcept override;

    // возвращаем ожидаемые бонусы без их начисления
    double getCashback() const;
    // возвращаем бонусы зафиксированные при оплате
    double getPaidCashback() const noexcept;
    
    int getId() const;
    const std::string& getCustomer() const;
    OrderStatus getStatus() const;

    // возвращаем позиции только для чтения
    const std::vector<std::shared_ptr<const Product>>& getItems() const;
    const std::string& getRegion() const;

    // методы для добавления и удаления позиций заказа
    void addItem(const Product& product, int quantity);
    void removeItem(int productId);

    double getTotalWeight() const;
    double getTotalPrice() const;
    // назначаем способ доставки после проверки
    void setDelivery(std::shared_ptr<Delivery> delivery);
};

// применяем акцию к исходному заказу и возвращаем этот же заказ
Order& operator+(Order& order, const Discount& discount);

// сохраняем в заказе собственную копию переданной доставки
Order& operator<<(Delivery& delivery, Order& order);

// выводим основные сведения о заказе в переданный поток
std::ostream& operator<<(std::ostream& output, const Order& order);
