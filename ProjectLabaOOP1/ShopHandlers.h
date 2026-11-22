// паттерн цепочка обязанностей объявления обработчиков текстовых команд
#pragma once

#include <memory>
#include <string>

class ShopFacade;

// паттерн цепочка обязанностей каждое звено хранит следующее
class Handler
{
private:
    // первый обработчик через эти указатели владеет всей цепочкой
    std::unique_ptr<Handler> next_;

protected:
    // наследник разбирает только свою команду
    virtual bool tryHandle(const std::string& input, ShopFacade& facade) const = 0;

public:
    virtual ~Handler() = default;

    Handler& setNext(std::unique_ptr<Handler> next);
    // сначала пробуем обработать строку здесь потом передаём дальше
    bool handle(const std::string& input, ShopFacade& facade) const;
};

// читает тип товара название цену и количество
class AddProductHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID товара и новую цену
class ChangePriceHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID товара и на сколько изменить остаток
class ChangeQuantityHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID товара и номер склада назначения
class MoveProductHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID товара для удаления
class RemoveProductHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает покупателя список товаров и способ доставки
class CreateOrderHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID заказа для отмены
class CancelOrderHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID заказа для подтверждения оплаты
class ConfirmPaymentHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// читает ID заказа для отправки
class ShipOrderHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// распознаёт запрос статистики магазина
class ShowStatisticsHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// распознаёт запрос списка товаров
class ShowProductsHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// распознаёт запрос списка активных заказов
class ShowOrdersHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// распознаёт запрос текущих настроек
class ShowSettingsHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

// распознаёт запрос списка команд
class HelpHandler : public Handler
{
protected:
    bool tryHandle(const std::string& input, ShopFacade& facade) const override;
};

