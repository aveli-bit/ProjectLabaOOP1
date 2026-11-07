#include "Price.h"
#include <stdexcept>

// создаём цену через метод с проверкой
Price::Price(int value)
{
    setValue(value);
}

// возвращаем текущее значение цены
int Price::getValue() const
{
    return value_;
}

// проверяем цену и сохраняем новое значение
void Price::setValue(int newValue)
{
    if (newValue < 0)
    {
        throw std::invalid_argument("Цена не может быть отрицательной");
    }

    value_ = newValue;
}