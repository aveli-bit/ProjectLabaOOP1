#include "Price.h"
#include <stdexcept>

Price::Price(int value)
{
    setValue(value);
}

int Price::getValue() const
{
    return value_;
}

void Price::setValue(int newValue)
{
    if (newValue < 0)
    {
        throw std::invalid_argument("Цена не может быть отрицательной");
    }

    value_ = newValue;
}