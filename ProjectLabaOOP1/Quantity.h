#pragma once

class Quantity
{
private:
    int value_;

public:
    // конструктор задаёт количество и проверяет значение
    explicit Quantity(int value = 0);

    int getValue() const;
    // меняем количество только после проверки
    void setValue(int newValue);
    // прибавляем или вычитаем количество товара
    void change(int delta);
};