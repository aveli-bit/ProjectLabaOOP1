#pragma once

class Weight
{
private:
    double value_;

public:
    // конструктор задаёт вес и проверяет значение
    explicit Weight(double value = 0.0);

    double getValue() const;
    // меняем вес только после проверки
    void setValue(double newValue);
};