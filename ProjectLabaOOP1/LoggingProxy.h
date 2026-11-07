#pragma once

#include <iostream>


class LoggingProxy
{
public:
    // выводим источник и сообщение а ошибки вывода перехватываем внутри
    static void log(const char* source,
        const char* message) noexcept
    {
        try
        {
            std::cout << "[" << source << "] "
                << message << '\n';
        }
        catch (...)
        {
            
        }
    }
};