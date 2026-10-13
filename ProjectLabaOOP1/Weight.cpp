#include "Weight.h"
#include <stdexcept>
#include <cmath>

Weight::Weight(double value)
{
    setValue(value);
}

double Weight::getValue() const
{
    return value_;
}

// Изменение веса товара
void Weight::setValue(double newValue)
{
    // Проверка отрицательного веса и некорректных значений
    if (newValue < 0.0 || !std::isfinite(newValue))
    {
        throw std::invalid_argument(
            "Вес должен быть неотрицательным и конечным числом"
        );
    }

    value_ = newValue;
}