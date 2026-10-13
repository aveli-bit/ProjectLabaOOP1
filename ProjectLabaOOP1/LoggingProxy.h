#pragma once

#include <iostream>


class LoggingProxy
{
public:
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