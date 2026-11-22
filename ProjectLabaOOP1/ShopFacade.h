// паттерн фасад общий интерфейс для работы с магазином
#pragma once

#include "ShopCommands.h"
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class ShopInventory;
class ShopMediator;
class OrderManager;
class Handler;

// паттерн фасад единая точка входа для команд магазина
class ShopFacade
{
private:
    ShopInventory& warehouse_;
    ShopMediator& mediator_;
    const OrderManager& orders_;
    std::ostream& output_;
    // фасад хранит начало цепочки и историю выполненных команд
    std::unique_ptr<Handler> firstHandler_;
    CommandHistory history_;

public:
    ShopFacade(ShopInventory& warehouse, ShopMediator& mediator,
        const OrderManager& orders, std::ostream& output);
    ~ShopFacade();

    // сюда приходит текст из консоли и отсюда начинается его разбор
    void processInput(const std::string& input);
    void executeCommand(std::unique_ptr<Command> command);

    // операции с товарами передаём складу
    void addProduct(const std::string& type, const std::string& name, int price, int quantity);
    void changePrice(int id, int price);
    void changeQuantity(int id, int delta);
    void moveProduct(int id, int warehouseId);
    void removeProduct(int id);
    void listProducts() const;

    // создание заказа и смену его статуса передаём посреднику
    void createOrder(const std::string& customer, const std::vector<int>& ids,
        const std::string& deliveryType);
    void cancelOrder(int id);
    void confirmPayment(int id);
    void shipOrder(int id);
    void listOrders() const;
    void getStatistics() const;
    void showSettings() const;
    void showHelp() const;
};

