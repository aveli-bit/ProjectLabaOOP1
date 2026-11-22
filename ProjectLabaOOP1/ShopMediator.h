// паттерн посредник координация подсистем при работе с заказами
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

class Warehouse;
class OrderManager;
class PaymentSystem;
class DeliveryService;
class Order;

// паттерн посредник согласует работу склада заказов оплаты и доставки
class ShopMediator
{
private:
    // посредник хранит ссылки на подсистемы а обратных ссылок у них нет
    Warehouse& warehouse_;
    OrderManager& orders_;
    PaymentSystem& payment_;
    DeliveryService& delivery_;

    // получаем заказ по ID или сообщаем что он не найден
    Order& requireOrder(int id);

public:
    ShopMediator(Warehouse& warehouse, OrderManager& orders,
        PaymentSystem& payment, DeliveryService& delivery);

    int createOrder(const std::string& customer, const std::vector<int>& ids,
        const std::string& deliveryType);
    void confirmPayment(int id);
    void shipOrder(int id);
    void cancelOrder(int id);
    enum class Report { Summary, ActiveOrders, OrderLimits };
    // через посредника получаем статистику активные заказы и лимиты менеджера
    void getStatistics(std::ostream& output, Report report = Report::Summary) const;

private:
    friend class ShopFacade;
    void checkProductRemoval(int id) const;
    // сюда фасад передаёт события об изменениях товаров
    void onProductAdded(int id) const noexcept;
    void onProductChanged(int id) const noexcept;
    void onProductRemoved(int id) const noexcept;
};

