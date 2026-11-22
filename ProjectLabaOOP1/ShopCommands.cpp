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

ChangePriceCommand::ChangePriceCommand(ShopFacade& facade, int id, int price)
    : Command(facade),
      id_(id),
      price_(price)
{
}

void ChangePriceCommand::execute()
{
    facade_.changePrice(id_, price_);
}

ChangeQuantityCommand::ChangeQuantityCommand(ShopFacade& facade, int id, int delta)
    : Command(facade),
      id_(id),
      delta_(delta)
{
}

// знак изменения определяет увеличится остаток или уменьшится
void ChangeQuantityCommand::execute()
{
    facade_.changeQuantity(id_, delta_);
}

MoveProductCommand::MoveProductCommand(ShopFacade& facade, int id, int warehouseId)
    : Command(facade),
      id_(id),
      warehouseId_(warehouseId)
{
}

// передаём ID товара и склад назначения
void MoveProductCommand::execute()
{
    facade_.moveProduct(id_, warehouseId_);
}

RemoveProductCommand::RemoveProductCommand(ShopFacade& facade, int id)
    : Command(facade),
      id_(id)
{
}

void RemoveProductCommand::execute()
{
    facade_.removeProduct(id_);
}

CreateOrderCommand::CreateOrderCommand(ShopFacade& facade, std::string customer, std::vector<int> ids, std::string deliveryType)
    : Command(facade),
      customer_(std::move(customer)),
      ids_(std::move(ids)),
      deliveryType_(std::move(deliveryType))
{
}

// передаём фасаду покупателя товары и доставку из полей команды
void CreateOrderCommand::execute()
{
    facade_.createOrder(customer_, ids_, deliveryType_);
}

CancelOrderCommand::CancelOrderCommand(ShopFacade& facade, int id)
    : Command(facade),
      id_(id)
{
}

void CancelOrderCommand::execute()
{
    facade_.cancelOrder(id_);
}

ConfirmPaymentCommand::ConfirmPaymentCommand(ShopFacade& facade, int id)
    : Command(facade),
      id_(id)
{
}

void ConfirmPaymentCommand::execute()
{
    facade_.confirmPayment(id_);
}

ShipOrderCommand::ShipOrderCommand(ShopFacade& facade, int id)
    : Command(facade),
      id_(id)
{
}

// фасад передаёт ID заказа посреднику для отправки
void ShipOrderCommand::execute()
{
    facade_.shipOrder(id_);
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

