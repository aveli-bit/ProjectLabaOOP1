#pragma once
// паттерн легковес
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

    static std::unordered_map< //контейнер стандартной библиотеки C++ который хранит пары ключ -> значение нужен чтобы находить уже созданный ProductType по строковому ключу если тип найден то используем его повторно если нет то создаём и добавляем в таблицу
        std::string, // Здесь static делает pool_ одной общей таблицей для всего класса ProductType.
        std::shared_ptr<ProductType>
    > pool_;// first - ключ second - значение 
    // пулн ужен для правильной работы паттерна. Он хранит созданные продукт тайпы чтобы товары с одинаковой категорией весом и базовой ценой использовали один общий объект
    // закрытый конструктор нужен для создания типа через пул
    ProductType(const std::string& category,
        const Weight& baseWeight,
        const Price& basePrice);

public:
    // получаем общий тип по категории весу и базовой цене
    static std::shared_ptr<ProductType> getType(  // static злесь нужен для вызова getType без ProductType, потому что метод сам находит или создаёт объект типа в общем пуле.
        const std::string& category,
        const Weight& baseWeight,
        const Price& basePrice);

    const std::string& getCategory() const;
    const Weight& getBaseWeight() const;
    const Price& getBasePrice() const;
};