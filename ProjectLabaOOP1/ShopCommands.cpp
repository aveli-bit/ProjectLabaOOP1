// паттерн команда выполнение действий через фасад и хранение истории
#include "ShopCommands.h"
#include "ShopFacade.h"
#include <stdexcept>
#include <utility>

// привязываем команду к фасаду при создании
Command::Command(ShopFacade& facade)
    : facade_(facade)
{
}

// передаём владение командой в историю
void CommandHistory::push(std::unique_ptr<Command> command)
{
    commands_.push_back(std::move(command));
}

void CommandHistory::executeAndPush(std::unique_ptr<Command> command)
{
    if (!command)
    {
        throw std::invalid_argument("Пустая команда");
    }

    // готовим место отдельно и переносим запись в историю после выполнения
    std::list<std::unique_ptr<Command>> entry;
    entry.push_back(std::move(command));
    entry.back()->execute();
    commands_.splice(commands_.end(), entry);
}

// даём доступ к последней команде без передачи владения
Command* CommandHistory::getLastCommand() noexcept
{
    return commands_.empty() ? nullptr : commands_.back().get();
}

// убираем последнюю запись из истории без отмены операции
void CommandHistory::undo() noexcept
{
    if (!commands_.empty())
    {
        commands_.pop_back();
    }
}

// сохраняем параметры товара до выполнения команды
AddProductCommand::AddProductCommand(ShopFacade& facade, std::string type, std::string name, int price, int quantity)
    : Command(facade),
      type_(std::move(type)),
      name_(std::move(name)),
      price_(price),
      quantity_(quantity)
{
}

void AddProductCommand::execute()
{
    facade_.addProduct(type_, name_, price_, quantity_);
}

ChangePriceCommand::ChangePriceCommand(ShopFacade& facade, int productId, int newPrice)
    : Command(facade),
      productId_(productId),
      newPrice_(newPrice)
{
}

void ChangePriceCommand::execute()
{
    facade_.changePrice(productId_, newPrice_);
}

ChangeQuantityCommand::ChangeQuantityCommand(ShopFacade& facade, int productId, int delta)
    : Command(facade),
      productId_(productId),
      delta_(delta)
{
}

// знак изменения определяет увеличится остаток или уменьшится
void ChangeQuantityCommand::execute()
{
    facade_.changeQuantity(productId_, delta_);
}

MoveProductCommand::MoveProductCommand(ShopFacade& facade, int productId, int newWarehouseId)
    : Command(facade),
      productId_(productId),
      newWarehouseId_(newWarehouseId)
{
}

// передаём ID товара и склад назначения
void MoveProductCommand::execute()
{
    facade_.moveProduct(productId_, newWarehouseId_);
}

RemoveProductCommand::RemoveProductCommand(ShopFacade& facade, int productId)
    : Command(facade),
      productId_(productId)
{
}

void RemoveProductCommand::execute()
{
    facade_.removeProduct(productId_);
}

CreateOrderCommand::CreateOrderCommand(ShopFacade& facade, std::string customer, std::vector<int> items, std::string deliveryType)
    : Command(facade),
      customer_(std::move(customer)),
      items_(std::move(items)),
      deliveryType_(std::move(deliveryType))
{
}

// передаём фасаду покупателя товары и доставку из полей команды
void CreateOrderCommand::execute()
{
    facade_.createOrder(customer_, items_, deliveryType_);
}

CancelOrderCommand::CancelOrderCommand(ShopFacade& facade, int orderId)
    : Command(facade),
      orderId_(orderId)
{
}

void CancelOrderCommand::execute()
{
    facade_.cancelOrder(orderId_);
}

ConfirmPaymentCommand::ConfirmPaymentCommand(ShopFacade& facade, int orderId)
    : Command(facade),
      orderId_(orderId)
{
}

void ConfirmPaymentCommand::execute()
{
    facade_.confirmPayment(orderId_);
}

ShipOrderCommand::ShipOrderCommand(ShopFacade& facade, int orderId)
    : Command(facade),
      orderId_(orderId)
{
}

// фасад передаёт ID заказа посреднику для отправки
void ShipOrderCommand::execute()
{
    facade_.shipOrder(orderId_);
}

// команды просмотра вызывают готовые методы вывода у фасада
void ShowStatisticsCommand::execute()
{
    facade_.getStatistics();
}

void ShowProductsCommand::execute()
{
    facade_.listProducts();
}

void ShowOrdersCommand::execute()
{
    facade_.listOrders();
}

void ShowSettingsCommand::execute()
{
    facade_.showSettings();
}

void HelpCommand::execute()
{
    facade_.showHelp();
}

