#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Price.h"
#include "Weight.h"

// легковес хранит общие характеристики для товаров
class ProductType
{
private:
    std::string category_;
    Weight baseWeight_;
    Price basePrice_;

    static std::unordered_map< //контейнер стандартной библиотеки C++ который хранит пары ключ -> значение нужен чтобы находить уже созданный ProductType по строковому ключу если тип найден — используем его повторно если нет то создаём и добавляем в таблицу
        std::string,
        std::shared_ptr<ProductType>
    > pool_;

    // закрытый конструктор нужен для создания типа через пул
    ProductType(const std::string& category,
        const Weight& baseWeight,
        const Price& basePrice);

public:
    // получаем общий тип по категории весу и базовой цене
    static std::shared_ptr<ProductType> getType(
        const std::string& category,
        const Weight& baseWeight,
        const Price& basePrice);

    const std::string& getCategory() const;
    const Weight& getBaseWeight() const;
    const Price& getBasePrice() const;
};