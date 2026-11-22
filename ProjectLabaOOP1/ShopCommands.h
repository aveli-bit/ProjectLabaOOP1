// паттерн команда объявления команд магазина и их истории
#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

class ShopFacade;

// паттерн команда хранит ссылку на фасад и задаёт общий метод выполнения
class Command
{
protected:
    // все команды работают с фасадом через одну ссылку
    ShopFacade& facade_;

public:
    explicit Command(ShopFacade& facade);
    virtual ~Command() = default;

    // действие определяет конкретная команда
    virtual void execute() = 0;
};

// храним команды которые прошли выполнение
class CommandHistory
{
private:
    // история владеет объектами выполненных команд
    std::list<std::unique_ptr<Command>> commands_;

public:
    void push(std::unique_ptr<Command> command);
    void executeAndPush(std::unique_ptr<Command> command);
    Command* getLastCommand() noexcept;
    void undo() noexcept;
};

// команды для товаров передают фасаду сохранённые параметры
class AddProductCommand : public Command
{
private:
    std::string type_;
    std::string name_;
    int price_;
    int quantity_;

public:
    AddProductCommand(ShopFacade& facade, std::string type, std::string name, int price, int quantity);
    void execute() override;
};

class ChangePriceCommand : public Command
{
private:
    int productId_;
    int newPrice_;

public:
    ChangePriceCommand(ShopFacade& facade, int productId, int newPrice);
    void execute() override;
};

// хранит изменение количества со знаком
class ChangeQuantityCommand : public Command
{
private:
    int productId_;
    int delta_;

public:
    ChangeQuantityCommand(ShopFacade& facade, int productId, int delta);
    void execute() override;
};

class MoveProductCommand : public Command
{
private:
    int productId_;
    int newWarehouseId_;

public:
    MoveProductCommand(ShopFacade& facade, int productId, int newWarehouseId);
    void execute() override;
};

class RemoveProductCommand : public Command
{
private:
    int productId_;

public:
    RemoveProductCommand(ShopFacade& facade, int productId);
    void execute() override;
};

// команда хранит данные заказа до вызова фасада
class CreateOrderCommand : public Command
{
private:
    std::string customer_;
    std::vector<int> items_;
    std::string deliveryType_;

public:
    CreateOrderCommand(ShopFacade& facade, std::string customer, std::vector<int> items, std::string deliveryType);
    void execute() override;
};

// для отмены достаточно сохранить ID заказа
class CancelOrderCommand : public Command
{
private:
    int orderId_;

public:
    CancelOrderCommand(ShopFacade& facade, int orderId);
    void execute() override;
};

class ConfirmPaymentCommand : public Command
{
private:
    int orderId_;

public:
    ConfirmPaymentCommand(ShopFacade& facade, int orderId);
    void execute() override;
};

class ShipOrderCommand : public Command
{
private:
    int orderId_;

public:
    ShipOrderCommand(ShopFacade& facade, int orderId);
    void execute() override;
};

// команды просмотра запрашивают данные через фасад
class ShowStatisticsCommand : public Command
{
public:
    using Command::Command;
    void execute() override;
};

class ShowProductsCommand : public Command
{
public:
    using Command::Command;
    void execute() override;
};

class ShowOrdersCommand : public Command
{
public:
    using Command::Command;
    void execute() override;
};

class ShowSettingsCommand : public Command
{
public:
    using Command::Command;
    void execute() override;
};

// запускает вывод справки через фасад
class HelpCommand : public Command
{
public:
    using Command::Command;
    void execute() override;
};

