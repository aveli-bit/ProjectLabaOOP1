#pragma once

class Price
{
private:
    int value_;

public:
    // конструктор задаёт цену и проверяет значение
    explicit Price(int value = 0);

    int getValue() const;
    // меняем цену только после проверки
    void setValue(int newValue);
};