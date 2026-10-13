#pragma once

class Quantity
{
private:
    int value_;

public:
    explicit Quantity(int value = 0);

    int getValue() const;
    void setValue(int newValue);
    void change(int delta);
};