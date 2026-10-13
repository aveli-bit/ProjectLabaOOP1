#pragma once

class Price
{
private:
    int value_;

public:
    explicit Price(int value = 0);

    int getValue() const;
    void setValue(int newValue);
};