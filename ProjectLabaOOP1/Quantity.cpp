#include "Quantity.h"
#include <stdexcept>
#include <limits>

Quantity::Quantity(int value)
{
    setValue(value);
}

int Quantity::getValue() const
{
    return value_;
}

void Quantity::setValue(int newValue)
{
    if (newValue < 0)
    {
        throw std::invalid_argument("Остаток не может быть отрицательным");
    }

    value_ = newValue;
}

// Изменение количества товара
void Quantity::change(int delta)
{
    // Проверка переполнения при увеличении
    if (delta > 0 &&
        value_ > std::numeric_limits<int>::max() - delta)
    {
        throw std::overflow_error("Превышено максимальное количество товара");
    }

    // Проверка отрицательного результата
    if (delta < 0 && value_ < -static_cast<long long>(delta))
    {
        throw std::invalid_argument("Количество не может быть отрицательным");
    }

    value_ += delta;
}// проверяет допустимость изменения до сложения а не после него.