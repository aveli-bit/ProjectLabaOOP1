// паттерн фасад направляем команды к складу и посреднику
#include "ShopFacade.h"
#include "ShopHandlers.h"
#include "ShopMediator.h"
#include "ShopServices.h"
#include "OrderManager.h"
#include "LoggingProxy.h"
#include <ostream>
#include <stdexcept>
#include <utility>

// сохраняем ссылки на готовые подсистемы и создаём первое звено
ShopFacade::ShopFacade(ShopInventory& warehouse, ShopMediator& mediator,
    const OrderManager& orders, std::ostream& output)
    : warehouse_(warehouse),
      mediator_(mediator),
      orders_(orders),
      output_(output),
      firstHandler_(std::make_unique<AddProductHandler>())
{
    // собираем цепочку каждый вызов возвращает добавленное звено
    firstHandler_
        ->setNext(std::make_unique<ChangePriceHandler>())
        .setNext(std::make_unique<ChangeQuantityHandler>())
        .setNext(std::make_unique<MoveProductHandler>())
        .setNext(std::make_unique<RemoveProductHandler>())
        .setNext(std::make_unique<CreateOrderHandler>())
        .setNext(std::make_unique<CancelOrderHandler>())
        .setNext(std::make_unique<ConfirmPaymentHandler>())
        .setNext(std::make_unique<ShipOrderHandler>())
        .setNext(std::make_unique<ShowStatisticsHandler>())
        .setNext(std::make_unique<ShowProductsHandler>())
        .setNext(std::make_unique<ShowOrdersHandler>())
        .setNext(std::make_unique<ShowSettingsHandler>())
        .setNext(std::make_unique<HelpHandler>());
}

ShopFacade::~ShopFacade() = default;

// передаём ввод первому звену цепочки
void ShopFacade::processInput(const std::string& input)
{
    // пустая строка не запускает обработчики
    if (input.find_first_not_of(" \t\r\n") == std::string::npos)
    {
        return;
    }

    LoggingProxy::log("SHOP", input.c_str());

    try
    {
        if (!firstHandler_->handle(input, *this))
        {
            throw std::invalid_argument("Неизвестная команда. Введи help");
        }

        LoggingProxy::log("SHOP", "Команда выполнена");
    }
    catch (const std::exception& error)
    {
        // пишем ошибку в лог и выводим её пользователю
        LoggingProxy::log("SHOP", error.what());
        output_ << "Ошибка: " << error.what() << '\n';
    }
}

void ShopFacade::executeCommand(std::unique_ptr<Command> command)
{
    if (!command)
    {
        throw std::invalid_argument("Пустая команда");
    }

    // готовим запись до выполнения и убираем её при ошибке
    history_.push(std::move(command));
    try
    {
        history_.getLastCommand()->execute();
    }
    catch (...)
    {
        history_.undo();
        throw;
    }
}

// склад добавляет товар а посредник получает событие с его ID
void ShopFacade::addProduct(const std::string& type, const std::string& name,
    int price, int quantity)
{
    mediator_.onProductAdded(warehouse_.addProduct(type, name, price, quantity));
}

// сообщаем посреднику об изменении только после успешной операции склада
void ShopFacade::changePrice(int id, int price)
{
    warehouse_.changePrice(id, price);
    mediator_.onProductChanged(id);
}

void ShopFacade::changeQuantity(int id, int delta)
{
    warehouse_.changeQuantity(id, delta);
    mediator_.onProductChanged(id);
}

void ShopFacade::moveProduct(int id, int warehouseId)
{
    warehouse_.moveProduct(id, warehouseId);
    mediator_.onProductChanged(id);
}

void ShopFacade::removeProduct(int id)
{
    mediator_.checkProductRemoval(id);
    warehouse_.removeProduct(id);
    mediator_.onProductRemoved(id);
}

void ShopFacade::listProducts() const
{
    warehouse_.listProducts(output_);
}

// посредник оформляет заказ а фасад выводит его номер
void ShopFacade::createOrder(const std::string& customer, const std::vector<int>& ids,
    const std::string& deliveryType)
{
    const int id = mediator_.createOrder(customer, ids, deliveryType);
    output_ << "Заказ создан: " << id << '\n';
}

void ShopFacade::cancelOrder(int id)
{
    mediator_.cancelOrder(id);
}

void ShopFacade::confirmPayment(int id)
{
    mediator_.confirmPayment(id);
}

void ShopFacade::shipOrder(int id)
{
    mediator_.shipOrder(id);
}

// получаем число активных заказов и их список у менеджера
void ShopFacade::listOrders() const
{
    output_ << "Активных заказов: " << orders_.getActiveOrderCount() << '\n';
    orders_.listActiveOrders(output_);
}

void ShopFacade::getStatistics() const
{
    mediator_.getStatistics(output_);
}

void ShopFacade::showSettings() const
{
    output_ << "Лимит на клиента: " << orders_.getMaxActiveOrders()
        << " | Общий лимит: " << orders_.getMaxTotalActiveOrders() << '\n'
        << "Доставка: courier pickup post\n"
        << "Активные акции: нет\n";
}

void ShopFacade::showHelp() const
{
    output_ << "Товары\n"
        << "  add product Smartphone \"iPhone 15\" 100000 5\n"
        << "  change price ID ЦЕНА\n"
        << "  change quantity ID ИЗМЕНЕНИЕ\n"
        << "  move product ID to СКЛАД\n"
        << "  remove product ID\n"
        << "  show products\n"
        << "Заказы\n"
        << "  create order \"Иван\" 1,2 courier\n"
        << "  confirm payment ID\n"
        << "  ship order ID\n"
        << "  cancel order ID\n"
        << "  show orders\n"
        << "  show statistics\n"
        << "  show settings\n"
        << "Ctrl+Z и Enter - завершить ввод\n";
}

