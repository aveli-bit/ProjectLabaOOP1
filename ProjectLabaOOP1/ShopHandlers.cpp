// паттерн цепочка обязанностей разбираем ввод и создаём команды
#include "ShopHandlers.h"
#include "ShopCommands.h"
#include "ShopFacade.h"
#include <charconv>
#include <locale>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

// звено сохраняет следующий обработчик и возвращает ссылку на него
Handler& Handler::setNext(std::unique_ptr<Handler> next)
{
    if (!next)
    {
        throw std::invalid_argument("Нужен следующий обработчик");
    }

    next_ = std::move(next);
    return *next_;
}

// передаём строку дальше если обработчик не узнал команду
bool Handler::handle(const std::string& input) const
{
    if (tryHandle(input, facade_))
    {
        return true;
    }

    return next_ && next_->handle(input);
}

namespace
{
    // сверяем первые слова чтобы понять подходит ли этот обработчик
    bool matches(std::istringstream& stream, const char* verb, const char* object = nullptr)
    {
        std::string word;
        if (!(stream >> word) || word != verb)
        {
            return false;
        }

        return !object || (stream >> word && word == object);
    }

    // каждый параметр начинается после пробела или табуляции
    template<class T>
    bool readArg(std::istringstream& stream, T&& value)
    {
        const int next = stream.peek();
        return next != std::char_traits<char>::eof()
            && std::isspace(static_cast<char>(next), stream.getloc())
            && static_cast<bool>(stream >> std::forward<T>(value));
    }

    // читаем ожидаемые параметры и проверяем что за ними ничего нет
    template<class... Args>
    void readArgs(std::istringstream& stream, Args&&... args)
    {
        if (!(readArg(stream, std::forward<Args>(args)) && ...))
        {
            throw std::invalid_argument("Не удалось прочитать параметры команды");
        }

        std::string extra;
        if (stream >> extra)
        {
            throw std::invalid_argument("Лишние параметры команды");
        }
    }

    // разбиваем список по запятым и проверяем каждый ID целиком
    std::vector<int> parseIds(const std::string& text)
    {
        std::vector<int> result;
        const char* start = text.data();
        const char* end = start + text.size();
        const char* part = start;

        for (const char* cursor = start;; ++cursor)
        {
            if (cursor == end || *cursor == ',')
            {
                int id = 0;
                const auto parsed = std::from_chars(part, cursor, id);
                if (parsed.ec != std::errc{} || parsed.ptr != cursor || id <= 0)
                {
                    throw std::invalid_argument("Нужен список ID через запятую");
                }

                result.push_back(id);
                if (cursor == end)
                {
                    return result;
                }

                part = cursor + 1;
            }
        }
    }
}

bool AddProductHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "add", "product"))
    {
        return false;
    }

    std::string type, name;
    int price = 0, quantity = 0;
    // кавычки позволяют прочитать название с пробелами
    readArgs(stream, type, std::quoted(name), price, quantity);
    facade.executeCommand(std::make_unique<AddProductCommand>(facade, type, name, price, quantity));
    return true;
}

bool ChangePriceHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "change", "price"))
    {
        return false;
    }

    int id = 0, price = 0;
    readArgs(stream, id, price);
    facade.executeCommand(std::make_unique<ChangePriceCommand>(facade, id, price));
    return true;
}

bool ChangeQuantityHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "change", "quantity"))
    {
        return false;
    }

    int id = 0, delta = 0;
    readArgs(stream, id, delta);
    facade.executeCommand(std::make_unique<ChangeQuantityCommand>(facade, id, delta));
    return true;
}

bool MoveProductHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "move", "product"))
    {
        return false;
    }

    int id = 0, warehouseId = 0;
    std::string to;
    // проверяем слово между ID товара и номером склада
    readArgs(stream, id, to, warehouseId);
    if (to != "to") throw std::invalid_argument("Используй move product ID to СКЛАД");
    facade.executeCommand(std::make_unique<MoveProductCommand>(facade, id, warehouseId));
    return true;
}

bool RemoveProductHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "remove", "product"))
    {
        return false;
    }

    int id = 0;
    readArgs(stream, id);
    facade.executeCommand(std::make_unique<RemoveProductCommand>(facade, id));
    return true;
}

bool CreateOrderHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "create", "order"))
    {
        return false;
    }

    std::string customer, ids, delivery;
    readArgs(stream, std::quoted(customer), ids, delivery);
    // передаём команде готовый список числовых ID
    facade.executeCommand(std::make_unique<CreateOrderCommand>(facade, customer, parseIds(ids), delivery));
    return true;
}

bool CancelOrderHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "cancel", "order"))
    {
        return false;
    }

    int id = 0;
    readArgs(stream, id);
    facade.executeCommand(std::make_unique<CancelOrderCommand>(facade, id));
    return true;
}

bool ConfirmPaymentHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "confirm", "payment"))
    {
        return false;
    }

    int id = 0;
    readArgs(stream, id);
    facade.executeCommand(std::make_unique<ConfirmPaymentCommand>(facade, id));
    return true;
}

bool ShipOrderHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "ship", "order"))
    {
        return false;
    }

    int id = 0;
    readArgs(stream, id);
    facade.executeCommand(std::make_unique<ShipOrderCommand>(facade, id));
    return true;
}

// у команд просмотра после имени команды параметров нет
bool ShowStatisticsHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "show", "statistics"))
    {
        return false;
    }

    readArgs(stream);
    facade.executeCommand(std::make_unique<ShowStatisticsCommand>(facade));
    return true;
}

bool ShowProductsHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "show", "products"))
    {
        return false;
    }

    readArgs(stream);
    facade.executeCommand(std::make_unique<ShowProductsCommand>(facade));
    return true;
}

bool ShowOrdersHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "show", "orders"))
    {
        return false;
    }

    readArgs(stream);
    facade.executeCommand(std::make_unique<ShowOrdersCommand>(facade));
    return true;
}

bool ShowSettingsHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "show", "settings"))
    {
        return false;
    }

    readArgs(stream);
    facade.executeCommand(std::make_unique<ShowSettingsCommand>(facade));
    return true;
}

bool HelpHandler::tryHandle(const std::string& input, ShopFacade& facade) const
{
    std::istringstream stream(input);
    if (!matches(stream, "help"))
    {
        return false;
    }

    readArgs(stream);
    facade.executeCommand(std::make_unique<HelpCommand>(facade));
    return true;
}

