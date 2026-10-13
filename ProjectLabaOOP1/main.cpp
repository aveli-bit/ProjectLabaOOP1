#include <iostream>
#include <windows.h>
#include <utility>
#include <exception>
#include <limits>
#include "Warehouse.h"
#include "WarehouseIterator.h"
#include "ProductFactory.h"
#include "Electronics.h"
#include "Clothing.h"
#include "Food.h"

using namespace std;

// Вывод всех товаров склада
void printWarehouse(const Warehouse& warehouse)
{
    cout << "Количество товаров: "
        << warehouse.getCurrentSize()
        << " из " << warehouse.getMaxSize() << '\n';

    for (const auto& product : warehouse)
    {
        cout << "ID: " << product.getId()
            << " | Название: " << product.getName()
            << " | Категория: " << product.getCategory()
            << " | Цена: " << product.getPrice().getValue()
            << " | Количество: " << product.getQuantity().getValue()
            << " | Вес: " << product.getWeight().getValue()
            << '\n';
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    try
    {
        // 1. Создание товаров через фабрики
        cout << " 1. СОЗДАНИЕ ТОВАРОВ \n";

        Warehouse warehouse1(10);

        SmartphoneFactory smartphoneFactory;
        LaptopFactory laptopFactory;
        TShirtFactory tshirtFactory;
        JacketFactory jacketFactory;
        MilkFactory milkFactory;
        BreadFactory breadFactory;

        warehouse1.addProduct(
            smartphoneFactory.createProduct(
                101, "iPhone 15", 80000, 10, 0.2)
        );

        warehouse1.addProduct(
            laptopFactory.createProduct(
                102, "Lenovo Legion", 120000, 5, 2.5)
        );

        warehouse1.addProduct(
            tshirtFactory.createProduct(
                103, "Футболка", 1500, 20, 0.3)
        );

        warehouse1.addProduct(
            jacketFactory.createProduct(
                104, "Куртка", 7000, 8, 1.2)
        );

        warehouse1.addProduct(
            milkFactory.createProduct(
                105, "Молоко", 120, 30, 1.0)
        );

        warehouse1.addProduct(
            breadFactory.createProduct(
                106, "Хлеб", 70, 40, 0.5)
        );

        printWarehouse(warehouse1);

        // 2. Поиск и изменение товаров
        cout << "\n 2. ПОИСК И ИЗМЕНЕНИЕ \n";

        Product* phone = warehouse1.findById(101);

        if (phone != nullptr)
        {
            cout << "Найден товар: " << phone->getName() << '\n';

            phone->setPrice(Price(75000));
            phone->changeQuantity(-2);

            cout << "Новая цена: "
                << phone->getPrice().getValue() << '\n';

            cout << "Новое количество: "
                << phone->getQuantity().getValue() << '\n';
        }

        auto food = warehouse1.findByCategory("food");

        cout << "Товаров категории food: " << food.size() << '\n';

        auto cheapProducts = warehouse1.findByPrice(Price(120));

        cout << "Товаров с ценой 120: "
            << cheapProducts.size() << '\n';

        // 3. Глубокое копирование
        cout << "\n 3. КОПИРОВАНИЕ \n";

        Warehouse warehouseCopy = warehouse1;

        warehouseCopy.findById(101)->setPrice(Price(50000));

        cout << "Цена оригинала: "
            << warehouse1.findById(101)->getPrice().getValue()
            << '\n';

        cout << "Цена копии: "
            << warehouseCopy.findById(101)->getPrice().getValue()
            << '\n';

        // 4. Перемещение товара между складами
        cout << "\n 4. ПЕРЕМЕЩЕНИЕ ТОВАРА \n";

        Warehouse warehouse2(10);

        Smartphone* smartphone =
            dynamic_cast<Smartphone*>(warehouse1.findById(101));

        if (smartphone != nullptr)
        {
            smartphone->transferTo(warehouse2);
        }
        // Находим перемещённый товар на втором складе
        Product* transferred = warehouse2.findById(101);

        if (transferred != nullptr)
        {
            // Изменяем цену и количество после перемещения
            transferred->setPrice(Price(70000));
            transferred->changeQuantity(-3);

            cout << "Товар: " << transferred->getName() << '\n';

            cout << "Новая цена: "
                << transferred->getPrice().getValue() << '\n';

            cout << "Новое количество: "
                << transferred->getQuantity().getValue() << '\n';
        }
        cout << "Первый склад: "
            << warehouse1.getCurrentSize() << '\n';

        cout << "Второй склад: "
            << warehouse2.getCurrentSize() << '\n';

        // 5. Удаление и перемещение склада
        cout << "\n 5. УДАЛЕНИЕ И ПЕРЕМЕЩЕНИЕ \n";

        warehouse1.removeById(102);
        warehouse1.removeByIndex(0);

        cout << "После удаления:\n";
        printWarehouse(warehouse1);

        // Запоминаем адрес товара до перемещения
        Product* addressBefore = warehouse2.findById(101);

        // Перемещаем склад
        Warehouse warehouse3 = std::move(warehouse2);

        // Получаем адрес товара после перемещения
        Product* addressAfter = warehouse3.findById(101);

        cout << "\nПосле перемещения склада:\n";

        cout << "Новый склад: "
            << warehouse3.getCurrentSize() << '\n';

        cout << "Старый склад: "
            << warehouse2.getCurrentSize() << '\n';

        // Проверяем отсутствие копирования
        if (addressBefore == addressAfter && addressAfter != nullptr)
        {
            cout << "Товар перемещён без копирования\n";
        }
        else
        {
            cout << "Проверка перемещения не пройдена\n";
        }

        // 6. Проверка обработки ошибок
        cout << "\n 6. ОБРАБОТКА ОШИБОК \n";

        try
        {
            Price invalidPrice(-100);
        }
        catch (const exception& error)
        {
            cout << "Проверка отрицательной цены: "
                << error.what() << '\n';
        }

        try
        {
            Warehouse smallWarehouse(1);

            smallWarehouse.addProduct(
                milkFactory.createProduct(
                    201, "Молоко 2", 100, 5, 1.0)
            );

            smallWarehouse.addProduct(
                breadFactory.createProduct(
                    202, "Хлеб 2", 60, 5, 0.5)
            );
        }
        catch (const exception& error)
        {
            cout << "Проверка переполнения склада: "
                << error.what() << '\n';
        }
        // Проверка переполнения количества
        try
        {
            Quantity quantity(2147483647);
            quantity.change(1);
        }
        catch (const exception& error)
        {
            cout << "Проверка переполнения количества: "
                << error.what() << '\n';
        }
        // Проверка некорректного веса
        try
        {
            Weight weight(std::numeric_limits<double>::quiet_NaN());
        }
        catch (const exception& error)
        {
            cout << "Проверка некорректного веса: "
                << error.what() << '\n';
        }
        // Проверка максимального значения ID
        try
        {
            SmartphoneFactory testFactory;
            auto product = testFactory.createProduct(
                (std::numeric_limits<int>::max)(),
                "Тестовый смартфон",
                1000, 1, 0.2
            );

            cout << "Создан товар с максимальным ID: "
                << product->getId() << '\n';
        }
        catch (const exception& error)
        {
            cout << "Ошибка ID: " << error.what() << '\n';
        }
        // Проверка исчерпания автоматических ID
        try
        {
            Smartphone phone(
                "Проверочный смартфон",
                Price(1000),
                Quantity(1),
                Weight(0.2)
            );

            cout << "Создан товар с ID: "
                << phone.getId() << '\n';
        }
        catch (const exception& error)
        {
            cout << "Проверка автоматического ID: "
                << error.what() << '\n';
        }

        cout << "\n ВСЕ СЦЕНАРИИ ЗАВЕРШЕНЫ \n";
    }
    catch (const exception& error)
    {
        cout << "Ошибка программы: " << error.what() << '\n';
        return 1;
    }

    return 0;
}