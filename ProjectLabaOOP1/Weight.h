#pragma once

class Weight
{
private:
    double value_;

public:
    explicit Weight(double value = 0.0);

    double getValue() const;
    void setValue(double newValue);
};