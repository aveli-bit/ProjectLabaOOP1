#include "ProductType.h"
#include "LoggingProxy.h"

#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>

std::unordered_map<
    std::string,
    std::shared_ptr<ProductType>
> ProductType::pool_;

// сохраняем общие характеристики и проверяем категорию
ProductType::ProductType(
    const std::string& category,
    const Weight& baseWeight,
    const Price& basePrice)
    : category_(category),
    baseWeight_(baseWeight),
    basePrice_(basePrice)
{
    if (category.empty()) // пустая ли строка категории
    {
        throw std::invalid_argument(
            "Категория товара не должна быть пустой"
        );
    }
}
// ищем тип в пуле а если его ещё нет создаём новый
std::shared_ptr<ProductType> ProductType::getType(
    const std::string& category,
    const Weight& baseWeight,
    const Price& basePrice)
{
    std::ostringstream keyStream; // формируем ключ
    keyStream.imbue(std::locale::classic());

    keyStream << category << '|'
        << std::setprecision(
            std::numeric_limits<double>::max_digits10)
        << baseWeight.getValue() << '|'
        << basePrice.getValue();

    const std::string key = keyStream.str();

    auto it = pool_.find(key);

    if (it != pool_.end())
    {
        return it->second; // значение найденной пары в unordered_map it->first — ключ этой пары то есть строка
    }

    auto type = std::shared_ptr<ProductType>(
        new ProductType(category, baseWeight, basePrice)
    );

    pool_.emplace(key, type);

    LoggingProxy::log("PRODUCT", "Создан общий тип товара");

    return type;
}
// возвращаем общие характеристики без создания копий
const std::string& ProductType::getCategory() const
{
    return category_;
}

const Weight& ProductType::getBaseWeight() const
{
    return baseWeight_;
}

const Price& ProductType::getBasePrice() const
{
    return basePrice_;
}