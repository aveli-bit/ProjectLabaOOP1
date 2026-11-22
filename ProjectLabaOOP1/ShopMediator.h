// паттерн посредник координация подсистем при работе с заказами
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

class ShopInventory;
class OrderManager;
class PaymentSystem;
class DeliveryService;
class Order;

// паттерн посредник согласует работу склада заказов оплаты и доставки
class ShopMediator
{
private:
    // подсистемы созданы снаружи и доступны посреднику по ссылке
    ShopInventory& warehouse_;
    OrderManager& orders_;
    PaymentSystem& payment_;
    DeliveryService& delivery_;

    // получаем заказ по ID или сообщаем что он не найден
    Order& requireOrder(int id);

public:
    ShopMediator(ShopInventory& warehouse, OrderManager& orders,
        PaymentSystem& payment, DeliveryService& delivery);

    int createOrder(const std::string& customer, const std::vector<int>& ids,
        const std::string& deliveryType);
    void confirmPayment(int id);
    void shipOrder(int id);
    void cancelOrder(int id);
    // объединяем данные менеджера заказов и склада
    void getStatistics(std::ostream& output) const;

    // сюда фасад передаёт события об изменениях товаров
    void onProductAdded(int id) const noexcept;
    void onProductChanged(int id) const noexcept;
    void onProductRemoved(int id) const noexcept;
};

