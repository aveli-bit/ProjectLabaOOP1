// подсистемы паттерна посредник операции склада оплаты и доставки
// для расчёта доставки вызываем выбранную стратегию
#include "ShopServices.h"
#include "ProductFactory.h"
#include "WarehouseIterator.h"
#include "Delivery.h"
#include "Order.h"
#include <cmath>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>

// склады клонируют товары а индекс получает адреса копий
ShopInventory::ShopInventory(const ShopInventory& other)
    : warehouses_(other.warehouses_), reserved_(other.reserved_),
      nextProductId_(other.nextProductId_)
{
    productsById_.reserve(other.productsById_.size());
    for (std::size_t index = 0; index < warehouses_.size(); ++index)
    {
        for (auto& product : warehouses_[index])
        {
            productsById_.emplace(product.getId(), ProductLocation{index, &product});
        }
    }
}

// сначала собираем копию целиком чтобы при ошибке не потерять текущий склад
ShopInventory& ShopInventory::operator=(const ShopInventory& other)
{
    if (this != &other)
    {
        ShopInventory copy(other);
        swapState(copy);
    }

    return *this;
}

// забираем состояние а у исходного объекта остаются пустые склады
ShopInventory::ShopInventory(ShopInventory&& other)
    : ShopInventory()
{
    swapState(other);
}

ShopInventory& ShopInventory::operator=(ShopInventory&& other)
{
    if (this != &other)
    {
        ShopInventory moved(std::move(other));
        swapState(moved);
    }

    return *this;
}

// при переносе меняется владелец склада а адрес товара сохраняется
void ShopInventory::swapState(ShopInventory& other) noexcept
{
    warehouses_.swap(other.warehouses_);
    productsById_.swap(other.productsById_);
    reserved_.swap(other.reserved_);
    std::swap(nextProductId_, other.nextProductId_);
}

// ищем товар в общем индексе обоих складов
ShopInventory::ProductLocation& ShopInventory::locate(int id)
{
    const auto entry = productsById_.find(id);
    if (entry == productsById_.end())
    {
        throw std::out_of_range("Товар не найден");
    }

    return entry->second;
}

int ShopInventory::reservedQuantity(int id) const
{
    const auto entry = reserved_.find(id);
    return entry == reserved_.end() ? 0 : entry->second;
}

// создаём товар через фабрику и добавляем его адрес в индекс
int ShopInventory::addProduct(const std::string& type, const std::string& name,
    int price, int quantity)
{
    if (name.empty() || nextProductId_ > (std::numeric_limits<int>::max)())
    {
        throw std::invalid_argument("Нельзя создать товар с такими параметрами");
    }

    const int id = static_cast<int>(nextProductId_);
    auto product = ProductFactory::createProduct(type, name, price, quantity, id);
    const auto entry = productsById_.emplace(id, ProductLocation{0, product.get()});
    if (!entry.second)
    {
        throw std::logic_error("ID товара уже существует");
    }

    try
    {
        warehouses_.front().addProduct(std::move(product));
    }
    catch (...)
    {
        // при отказе склада убираем адрес товара из индекса
        productsById_.erase(entry.first);
        throw;
    }

    ++nextProductId_;
    return id;
}

void ShopInventory::changePrice(int id, int price)
{
    warehouses_[locate(id).warehouseIndex].changePrice(id, Price(price));
}

// остаток после изменения должен покрывать уже сделанный резерв
void ShopInventory::changeQuantity(int id, int delta)
{
    const auto& location = locate(id);
    const auto after = static_cast<long long>(location.product->getQuantity().getValue()) + delta;

    if (after < reservedQuantity(id))
    {
        throw std::invalid_argument("Остаток не может быть меньше резерва");
    }

    warehouses_[location.warehouseIndex].changeQuantity(id, delta);
}

void ShopInventory::moveProduct(int id, int warehouseId)
{
    if (warehouseId != 1 && warehouseId != 2)
    {
        throw std::invalid_argument("Номер склада должен быть 1 или 2");
    }

    if (reservedQuantity(id) != 0)
    {
        throw std::logic_error("Зарезервированный товар нельзя переместить");
    }

    ProductLocation& location = locate(id);
    const auto target = static_cast<std::size_t>(warehouseId - 1);
    // обновляем склад в индексе только после успешного переноса
    warehouses_[location.warehouseIndex].moveProduct(id, warehouses_[target]);
    location.warehouseIndex = target;
}

void ShopInventory::removeProduct(int id)
{
    if (reservedQuantity(id) != 0)
    {
        throw std::logic_error("Зарезервированный товар нельзя удалить");
    }

    warehouses_[locate(id).warehouseIndex].removeProduct(id);
    productsById_.erase(id);
}

// показываем доступное количество отдельно от резерва
void ShopInventory::listProducts(std::ostream& output) const
{
    for (std::size_t index = 0; index < warehouses_.size(); ++index)
    {
        const auto& warehouse = warehouses_[index];
        output << "Склад " << index + 1 << " | Товаров: "
            << warehouse.getCurrentSize() << '\n';
        for (const auto& product : warehouse)
        {
            const int reserved = reservedQuantity(product.getId());
            output << "ID: " << product.getId()
                << " | Название: " << product.getName()
                << " | Цена: " << product.getPrice().getValue()
                << " | Остаток: " << product.getQuantity().getValue() - reserved
                << " | Резерв: " << reserved << '\n';
        }
    }
}

// копируем одну единицу товара для позиции заказа
std::shared_ptr<Product> ShopInventory::prepareItem(int id)
{
    const Product& product = *locate(id).product;
    if (product.getQuantity().getValue() <= reservedQuantity(id))
    {
        throw std::runtime_error("Товара нет в свободном остатке");
    }

    auto copy = product.clone();
    copy->changeQuantity(1 - copy->getQuantity().getValue());
    return std::shared_ptr<Product>(std::move(copy));
}

// резерв уменьшает доступный остаток до отправки
void ShopInventory::reserve(int id)
{
    const auto quantity = locate(id).product->getQuantity().getValue();
    if (quantity <= reservedQuantity(id))
    {
        throw std::runtime_error("Товара нет в свободном остатке");
    }

    ++reserved_[id];
}

// снимаем одну единицу резерва и удаляем запись при нуле
void ShopInventory::release(int id) noexcept
{
    const auto entry = reserved_.find(id);
    if (entry != reserved_.end() && --entry->second == 0)
    {
        reserved_.erase(entry);
    }
}

void ShopInventory::consume(int id) noexcept
{
    // зарезервированный товар нельзя удалить и его остаток не меньше резерва
    productsById_.find(id)->second.product->changeQuantity(-1);
    release(id);
}

std::size_t ShopInventory::getUsedSlots() const
{
    return warehouses_[0].getCurrentSize() + warehouses_[1].getCurrentSize();
}

std::size_t ShopInventory::getTotalSlots() const
{
    return warehouses_[0].getMaxSize() + warehouses_[1].getMaxSize();
}

// из начального баланса вычитаем только подтверждённые платежи клиента
double PaymentSystem::checkBalance(const std::string& customer) const
{
    double balance = 1000000.0;
    for (const auto& entry : payments_)
    {
        if (entry.second.paid && entry.second.customer == customer)
        {
            balance -= entry.second.amount;
        }
    }

    return balance;
}

// запрос фиксирует сумму а деньги списываются при подтверждении
void PaymentSystem::requestPayment(int orderId, const std::string& customer, double amount)
{
    if (!std::isfinite(amount) || amount < 0)
    {
        throw std::runtime_error("Оплата отклонена");
    }

    if (!payments_.emplace(orderId, Payment{customer, amount}).second)
    {
        throw std::logic_error("Запрос оплаты уже зарегистрирован");
    }
}

// проверяем сумму запроса и баланс перед списанием
void PaymentSystem::processPayment(int orderId, double amount)
{
    auto entry = payments_.find(orderId);
    if (entry == payments_.end() || entry->second.paid)
    {
        throw std::logic_error("Нет запроса на оплату заказа");
    }

    auto& payment = entry->second;
    if (payment.amount != amount)
    {
        throw std::logic_error("Сумма заказа изменилась после запроса оплаты");
    }

    if (payment.amount > checkBalance(payment.customer))
    {
        throw std::runtime_error("Недостаточно средств для оплаты");
    }

    payment.paid = true;
}

// возвращаем запрос в ожидание если заказ не перешёл в оплаченный статус
void PaymentSystem::resetPayment(int orderId) noexcept
{
    const auto entry = payments_.find(orderId);
    if (entry != payments_.end())
    {
        entry->second.paid = false;
    }
}

void PaymentSystem::refund(int orderId) noexcept
{
    payments_.erase(orderId);
}

// создаём для заказа выбранный способ доставки
std::shared_ptr<Delivery> DeliveryService::select(const std::string& type) const
{
    if (type == "courier") return std::make_shared<CourierDelivery>(couriers_);
    if (type == "pickup") return std::make_shared<PickupDelivery>(pickupPoints_);
    if (type == "post") return std::make_shared<PostDelivery>(postOffices_);

    throw std::invalid_argument("Неизвестный способ доставки");
}

bool DeliveryService::isAvailable(const Delivery& delivery, const Order& order) const
{
    return delivery.isAvailable(order);
}

// стоимость считает конкретный способ доставки
double DeliveryService::calculateCost(const Delivery& delivery, const Order& order) const
{
    return delivery.getCost(order);
}

// сохраняем план доставки только для доступного способа
void DeliveryService::scheduleDelivery(const Delivery& delivery, const Order& order)
{
    if (!isAvailable(delivery, order))
    {
        throw std::runtime_error("Доставка недоступна для заказа");
    }

    if (!scheduled_.insert(order.getId()).second)
    {
        throw std::logic_error("Доставка уже запланирована");
    }
}

bool DeliveryService::hasSchedule(int orderId) const
{
    return scheduled_.find(orderId) != scheduled_.end();
}

void DeliveryService::removeSchedule(int orderId) noexcept
{
    scheduled_.erase(orderId);
}

