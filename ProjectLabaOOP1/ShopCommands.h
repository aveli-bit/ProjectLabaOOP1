// паттерн команда объявления команд магазина и их истории
#pragma once

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
    std::vector<std::unique_ptr<Command>> commands_;

public:
    void push(std::unique_ptr<Command> command);
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
    int id_;
    int price_;

public:
    ChangePriceCommand(ShopFacade& facade, int id, int price);
    void execute() override;
};

// хранит изменение количества со знаком
class ChangeQuantityCommand : public Command
{
private:
    int id_;
    int delta_;

public:
    ChangeQuantityCommand(ShopFacade& facade, int id, int delta);
    void execute() override;
};

class MoveProductCommand : public Command
{
private:
    int id_;
    int warehouseId_;

public:
    MoveProductCommand(ShopFacade& facade, int id, int warehouseId);
    void execute() override;
};

class RemoveProductCommand : public Command
{
private:
    int id_;

public:
    RemoveProductCommand(ShopFacade& facade, int id);
    void execute() override;
};

// команда хранит данные заказа до вызова фасада
class CreateOrderCommand : public Command
{
private:
    std::string customer_;
    std::vector<int> ids_;
    std::string deliveryType_;

public:
    CreateOrderCommand(ShopFacade& facade, std::string customer, std::vector<int> ids, std::string deliveryType);
    void execute() override;
};

// для отмены достаточно сохранить ID заказа
class CancelOrderCommand : public Command
{
private:
    int id_;

public:
    CancelOrderCommand(ShopFacade& facade, int id);
    void execute() override;
};

class ConfirmPaymentCommand : public Command
{
private:
    int id_;

public:
    ConfirmPaymentCommand(ShopFacade& facade, int id);
    void execute() override;
};

class ShipOrderCommand : public Command
{
private:
    int id_;

public:
    ShipOrderCommand(ShopFacade& facade, int id);
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

