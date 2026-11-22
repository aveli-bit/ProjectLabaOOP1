#include "Warehouse.h"
#include <algorithm> // find_if
#include <stdexcept> // invalid_argument, out_of_range, runtime_error, overflow_error
#include <utility> // move swap
#include "WarehouseIterator.h"
#include "LoggingProxy.h"
#include "ProductFactory.h"
#include <limits>
#include <ostream>

Warehouse::Warehouse()
    : Warehouse(100)
{
    locationCount_ = 2;
}

// Конструктор склада
Warehouse::Warehouse(size_t maxSize)
    : maxSize_(maxSize)
{
    if (maxSize == 0)
    {
        throw std::invalid_argument(
            "Размерность склада должна быть положительной"
        );
    }
    LoggingProxy::log("WAREHOUSE", "Склад создан");
}

// Конструктор копирования
Warehouse::Warehouse(const Warehouse& other)
    : maxSize_(other.maxSize_), locationCount_(other.locationCount_), nextProductId_(other.nextProductId_)
{
    for (const auto& product : other.products_)
    {
        // делаем отдельный товар того же конкретного типа
        auto copy = product->clone();
        copy->owner_ = this;
        StockEntry entry = other.productsById_.at(copy->getId());
        entry.product = copy.get();
        productsById_.emplace(copy->getId(), entry);
        products_.push_back(std::move(copy)); 
    }

    LoggingProxy::log("WAREHOUSE", "Создана глубокая копия склада");
}

// Оператор копирующего присваивания
Warehouse& Warehouse::operator=(const Warehouse& other)
{
    if (this != &other)
    {
        // сначала создаём копию чтобы при ошибке текущий склад не изменился
        Warehouse temp(other);

        swapState(temp);
    }
    LoggingProxy::log("WAREHOUSE", "Выполнено копирующее присваивание склада");
    return *this;
}

// Конструктор перемещения
Warehouse::Warehouse(Warehouse&& other) noexcept
    : maxSize_(other.locationCount_ == 2 ? 100 : 0), locationCount_(other.locationCount_)
{
    swapState(other);
    LoggingProxy::log("WAREHOUSE", "Склад перемещён через конструктор");
}

// Оператор перемещающего присваивания
Warehouse& Warehouse::operator=(Warehouse&& other) noexcept
{
    if (this != &other)
    {
        Warehouse moved(std::move(other));
        swapState(moved);
    }
    LoggingProxy::log("WAREHOUSE", "Выполнено перемещающее присваивание склада");
    return *this;
}

// переносим индекс вместе с товарами и привязываем их к владельцу
void Warehouse::swapState(Warehouse& other) noexcept
{
    products_.swap(other.products_);
    productsById_.swap(other.productsById_);
    std::swap(maxSize_, other.maxSize_);
    std::swap(locationCount_, other.locationCount_);
    std::swap(nextProductId_, other.nextProductId_);
    for (auto& item : products_) item->owner_ = this;
    for (auto& item : other.products_) item->owner_ = &other;
}

size_t Warehouse::countAt(int warehouseId) const
{
    if (locationCount_ == 1) return products_.size();
    return std::count_if(productsById_.begin(), productsById_.end(),
        [warehouseId](const auto& entry) { return entry.second.warehouseId == warehouseId; });
}

// Максимальная вместимость
size_t Warehouse::getMaxSize() const
{
    return maxSize_ * locationCount_;
}

// Текущее количество товаров
size_t Warehouse::getCurrentSize() const
{
    return products_.size();
}

// Добавление товара на склад
void Warehouse::addProduct(std::unique_ptr<Product> product)
{
    // Проверка на пустой указатель
    if (!product)
    {
        throw std::invalid_argument("Нельзя добавить пустой товар");
    }

    // Проверка вместимости склада
    if (countAt(1) >= maxSize_)
    {
        throw std::runtime_error("Склад переполнен");
    }

    // Проверка уникальности ID
    if (findById(product->getId()) != nullptr)
    {
        throw std::invalid_argument(
            "Товар с таким ID уже существует на складе"
        );
    }
    const int id = product->getId();
    productsById_.emplace(id, StockEntry{product.get(), 1, 0});
    try
    {
        product->owner_ = this;
        products_.push_back(std::move(product));
    }
    catch (...)
    {
        productsById_.erase(id);
        throw;
    }
    nextProductId_ = (std::max)(nextProductId_, static_cast<long long>(id) + 1);
    LoggingProxy::log("WAREHOUSE", "Товар добавлен на склад");
}


// фабрика создаёт товар с очередным ID магазина
int Warehouse::addProduct(const std::string& type, const std::string& name, int price, int quantity)
{
    if (name.empty() || nextProductId_ > (std::numeric_limits<int>::max)())
    {
        throw std::invalid_argument("Нельзя создать товар с такими параметрами");
    }
    const int id = static_cast<int>(nextProductId_);
    addProduct(ProductFactory::createProduct(type, name, price, quantity, id));
    return id;
}

// Удаление товара по ID
void Warehouse::removeById(int id)
{
    removeProduct(id);
}

void Warehouse::removeProduct(int id)
{
    auto it = std::find_if(
        products_.begin(),
        products_.end(),
        [id](const std::unique_ptr<Product>& product)
        {
            return product->getId() == id;
        }
    );

    if (it == products_.end())
    {
        throw std::out_of_range("Не найден айди продукта");
    }

    if (reservedQuantity(id) != 0) throw std::logic_error("Зарезервированный товар нельзя удалить");
    productsById_.erase(id);
    products_.erase(it);
    LoggingProxy::log("WAREHOUSE", "Товар удалён по ID");
}

// Удаление товара по индексу
void Warehouse::removeByIndex(size_t index)
{
    if (index >= products_.size())
    {
        throw std::out_of_range("Недопустимый индекс");
    }

    const int id = products_[index]->getId();
    if (reservedQuantity(id) != 0) throw std::logic_error("Зарезервированный товар нельзя удалить");
    productsById_.erase(id);
    products_.erase(products_.begin() + index);
    LoggingProxy::log("WAREHOUSE", "Товар удалён по индексу");
}

// Поиск товара по ID
Product* Warehouse::findById(int id)
{
    const auto it = productsById_.find(id);
    return it == productsById_.end() ? nullptr : it->second.product;
}

// изменение по ID возможно только для товара на этом складе
Product& Warehouse::requireProduct(int id)
{
    auto* product = findById(id);
    if (!product)
    {
        throw std::out_of_range("Товар не найден");
    }
    return *product;
}

void Warehouse::changePrice(int id, const Price& price)
{
    requireProduct(id).changePrice(price);
}

void Warehouse::changePrice(int id, int newPrice)
{
    changePrice(id, Price(newPrice));
}

void Warehouse::changeQuantity(int id, int delta)
{
    requireProduct(id).changeQuantity(delta);
}

// Поиск товаров по категории
std::vector<Product*> Warehouse::findByCategory(
    const std::string& category)
{
    std::vector<Product*> result;

    for (auto& product : products_)
    {
        if (product->getCategory() == category)
        {
            result.push_back(product.get());
        }
    }

    LoggingProxy::log("WAREHOUSE", "Выполнен поиск по категории");

    return result;
}

// Поиск товаров по цене
std::vector<Product*> Warehouse::findByPrice(
    const Price& price)
{
    std::vector<Product*> result;

    for (auto& product : products_)
    {
        if (product->getPrice().getValue() == price.getValue())
        {
            result.push_back(product.get());
        }
    }

    LoggingProxy::log("WAREHOUSE", "Выполнен поиск по цене");

    return result;
}

// Перемещение товара между складами
void Warehouse::transferTo(int id, Warehouse& other)
{
    moveProduct(id, other);
}

void Warehouse::moveProduct(int id, Warehouse& other)
{
    // Проверяем, что склады разные
    if (this == &other)
    {
        throw std::invalid_argument(
            "Нельзя перемещать на тот же склад, склады должны быть разные"
        );
    }

    // Ищем товар по ID
    auto it = std::find_if(
        products_.begin(),
        products_.end(),
        [id](const std::unique_ptr<Product>& product)
        {
            return product->getId() == id;
        }
    );

    if (it == products_.end())
    {
        throw std::out_of_range("Продукт не найден");
    }

    // Проверяем вместимость второго склада
    if (other.countAt(1) >= other.maxSize_)
    {
        throw std::runtime_error(
            "Склад назначения переполнен"
        );
    }

    // Проверяем отсутствие одинакового ID
    if (other.findById(id) != nullptr)
    {
        throw std::runtime_error(
            "Такой айди продукта уже существует"
        );
    }

    if (reservedQuantity(id) != 0) throw std::logic_error("Зарезервированный товар нельзя переместить");

    // Выделяем место до перемещения товара
    other.products_.reserve(other.products_.size() + 1);

    other.productsById_.emplace(id, StockEntry{it->get(), 1, 0});
    other.nextProductId_ = (std::max)(other.nextProductId_, static_cast<long long>(id) + 1);
    productsById_.erase(id);

    // Передаём владение товаром
    other.products_.push_back(std::move(*it));
    other.products_.back()->owner_ = &other;

    // Удаляем пустой указатель с первого склада(из контейнера)
    products_.erase(it);

    LoggingProxy::log("WAREHOUSE", "Товар перемещён между складами");
}

Warehouse::StockEntry& Warehouse::stockFor(int id)
{
    auto it = productsById_.find(id);
    if (it == productsById_.end()) throw std::out_of_range("Товар не найден");
    return it->second;
}

int Warehouse::reservedQuantity(int id) const noexcept
{
    const auto it = productsById_.find(id);
    return it == productsById_.end() ? 0 : it->second.reserved;
}

// у каждого товара есть номер склада и общий для всех заказов резерв
void Warehouse::moveProduct(int id, int warehouseId)
{
    if (warehouseId < 1 || warehouseId > locationCount_)
        throw std::invalid_argument("Указанный склад не существует");
    auto& entry = stockFor(id);
    if (entry.reserved != 0) throw std::logic_error("Зарезервированный товар нельзя переместить");
    if (entry.warehouseId == warehouseId) throw std::invalid_argument("Нельзя перемещать на тот же склад");
    if (countAt(warehouseId) >= maxSize_) throw std::runtime_error("Склад назначения переполнен");
    entry.warehouseId = warehouseId;
    LoggingProxy::log("WAREHOUSE", "Товар перемещён между складами");
}

void Warehouse::listProducts(std::ostream& output) const
{
    for (int id = 1; id <= locationCount_; ++id)
    {
        output << "Склад " << id << " | Товаров: " << countAt(id) << '\n';
        for (const auto& product : products_)
        {
            const auto& entry = productsById_.at(product->getId());
            if (entry.warehouseId != id) continue;
            output << "ID: " << product->getId() << " | Название: " << product->getName()
                << " | Цена: " << product->getPrice().getValue()
                << " | Остаток: " << product->getQuantity().getValue() - entry.reserved
                << " | Резерв: " << entry.reserved << '\n';
        }
    }
}

// для позиции заказа копируем одну доступную единицу
std::shared_ptr<Product> Warehouse::prepareItem(int id)
{
    const auto& entry = stockFor(id);
    const int quantity = entry.product->getQuantity().getValue();
    if (quantity <= entry.reserved) throw std::runtime_error("Товара нет в свободном остатке");
    auto copy = entry.product->clone();
    copy->changeQuantity(1 - quantity);
    return std::shared_ptr<Product>(std::move(copy));
}

void Warehouse::reserve(int id)
{
    auto& entry = stockFor(id);
    if (entry.product->getQuantity().getValue() <= entry.reserved)
        throw std::runtime_error("Товара нет в свободном остатке");
    ++entry.reserved;
}

void Warehouse::release(int id) noexcept
{
    --productsById_.find(id)->second.reserved;
}

// при отправке снимаем одну единицу с резерва и списываем её со склада
void Warehouse::consume(int id) noexcept
{
    release(id);
    findById(id)->changeQuantity(-1);
}

// Итератор на первый товар склада
WarehouseIterator Warehouse::begin()
{
    return WarehouseIterator(
        products_.begin(), products_.end()
    );
}

// Итератор на конец склада
WarehouseIterator Warehouse::end()
{
    return WarehouseIterator(
        products_.end(), products_.end()
    );
}

// Константный итератор на первый товар
ConstWarehouseIterator Warehouse::begin() const
{
    return ConstWarehouseIterator(
        products_.cbegin(), products_.cend()
    );
}

// Константный итератор на конец склада
ConstWarehouseIterator Warehouse::end() const
{
    return ConstWarehouseIterator(
        products_.cend(), products_.cend()
    );
}
