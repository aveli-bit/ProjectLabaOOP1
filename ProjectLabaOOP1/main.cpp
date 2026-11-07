#include <iostream>
#include <windows.h>
#include <utility> // move swap
#include <exception> // except
#include <limits> // границы, NaN
#include "Warehouse.h"
#include "WarehouseIterator.h"
#include "ProductFactory.h"
#include "Electronics.h"
#include "Clothing.h"
#include "Food.h"
#include "ProductType.h"
#include "Order.h"
#include "Delivery.h"
#include <memory>
#include "Discount.h"
#include "OrderManager.h"

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
            << " | Вес единицы: " << product.getWeight().getValue()
            << " | Общая стоимость: " << product.getTotalPrice()
            << " | Общий вес: " << product.getTotalWeight()
            << '\n';

    }
}

// проверяем менеджер через создание оплату доставку и отмену заказов
void demonstrateOrderManager(const Product& product)
{
    std::cout << "\n МЕНЕДЖЕР ЗАКАЗОВ \n";

    std::shared_ptr<Product> position(product.clone());
    position->changeQuantity(2 - position->getQuantity().getValue());
    const std::vector<std::shared_ptr<Product>> items{ position };

    OrderManager manager(2, 3);
    Order& firstOrder = manager.createOrder("manager-customer-1", items);

    DeliveryProxy delivery(
        std::make_shared<CourierDelivery>()
    );
    delivery << firstOrder;

    PercentDiscount percentDiscount;
    CashbackDiscount cashbackDiscount;
    firstOrder + percentDiscount + cashbackDiscount;

    Order& secondOrder = manager.createOrder("manager-customer-1");
    Order& thirdOrder = manager.createOrder("manager-customer-2");

    try
    {
        manager.createOrder("manager-customer-1");
    }
    catch (const std::exception& error)
    {
        std::cout << "Лимит клиента: " << error.what() << '\n';
    }

    try
    {
        manager.createOrder("manager-customer-3");
    }
    catch (const std::exception& error)
    {
        std::cout << "Общий лимит: " << error.what() << '\n';
    }

    const auto periodStart = std::chrono::system_clock::now();
    manager.updateStatus(firstOrder.getId(), OrderStatus::Paid);
    firstOrder.setStatus(OrderStatus::Paid);

    std::cout << "Оборот после оплаты и её повтора: "
        << manager.getTurnover() << '\n';
    std::cout << "Начисленные бонусы клиента: "
        << manager.getBonusBalance("manager-customer-1") << '\n';

    std::cout << "Вывод заказа через оператор: " << firstOrder << '\n';

    manager.updateStatus(firstOrder.getId(), OrderStatus::Shipped);
    manager.updateStatus(firstOrder.getId(), OrderStatus::Delivered);

    std::cout << "Активных заказов клиента после доставки: "
        << manager.getActiveOrderCount("manager-customer-1") << '\n';

    manager.updateStatus(secondOrder.getId(), OrderStatus::Cancelled);
    Order& replacementOrder = manager.createOrder("manager-customer-1", items);
    replacementOrder.applyDiscount(cashbackDiscount);
    manager.updateStatus(replacementOrder.getId(), OrderStatus::Paid);

    std::cout << "Оборот до отмены оплаты: "
        << manager.getTurnover() << '\n';
    std::cout << "Бонусы до отмены оплаты: "
        << manager.getBonusBalance("manager-customer-1") << '\n';

    manager.updateStatus(replacementOrder.getId(), OrderStatus::Cancelled);
    manager.updateStatus(thirdOrder.getId(), OrderStatus::Cancelled);

    std::cout << "Оборот после отмены оплаты: "
        << manager.getTurnover() << '\n';
    std::cout << "Бонусы после отмены оплаты: "
        << manager.getBonusBalance("manager-customer-1") << '\n';
    std::cout << "Всего созданных заказов: "
        << manager.getOrderCount() << '\n';
    std::cout << "Уникальных клиентов: "
        << manager.getCustomerCount() << '\n';
    std::cout << "Активных заказов в конце: "
        << manager.getActiveOrderCount() << '\n';

    const auto periodEnd = std::chrono::system_clock::now()
        + std::chrono::seconds(1);

    std::cout << "Оборот за проверяемый период: "
        << manager.getTurnover(periodStart, periodEnd) << '\n';
}

// проверяем независимость копий и работу подписок после перемещения
void demonstrateCopyAndMove(const Product& product)
{
    std::cout << "\n КОПИРОВАНИЕ И ПЕРЕМЕЩЕНИЕ ЗАКАЗОВ \n";

    Order original(6, "copy-customer");
    original.addItem(product, 2);
    PostDelivery post;
    PercentDiscount percent;
    CashbackDiscount cashback;
    post << original;
    original + percent + cashback;

    Order copy(original);
    copy.removeItem(product.getId());
    copy.addItem(product, 3);

    std::cout << "Цена исходного заказа: " << original.getTotalPrice() << '\n';
    std::cout << "Цена изменённой копии: " << copy.getTotalPrice() << '\n';

    const Product* itemBefore = copy.getItems().front().get();
    Order moved(std::move(copy));

    std::cout << "Адрес товара после перемещения сохранён: "
        << (itemBefore == moved.getItems().front().get()) << '\n';
    std::cout << "Позиций в источнике после перемещения: "
        << copy.getItems().size() << '\n';

    Order assigned(7, "assigned-customer");
    assigned = original;
    Order moveAssigned(8, "move-assigned-customer");
    moveAssigned = std::move(assigned);

    std::cout << "Стоимость после двух присваиваний: "
        << moveAssigned.getTotalPrice() << '\n';

    OrderManager source(2, 4);
    Order& sourceOrder = source.createOrder("manager-copy");
    sourceOrder.addItem(product, 2);
    sourceOrder + cashback;
    source.updateStatus(sourceOrder.getId(), OrderStatus::Paid);

    OrderManager managerCopy(source);
    managerCopy.updateStatus(sourceOrder.getId(), OrderStatus::Cancelled);

    std::cout << "Оборот исходного менеджера после отмены в копии: "
        << source.getTurnover() << '\n';
    std::cout << "Оборот копии менеджера после отмены: "
        << managerCopy.getTurnover() << '\n';
    std::cout << "Бонусы исходного клиента после отмены в копии: "
        << source.getBonusBalance("manager-copy") << '\n';

    OrderManager managerAssigned;
    managerAssigned = source;
    Order* orderBefore = managerAssigned.findById(sourceOrder.getId());
    OrderManager managerMoved(std::move(managerAssigned));

    std::cout << "Адрес заказа после перемещения менеджера сохранён: "
        << (orderBefore == managerMoved.findById(sourceOrder.getId())) << '\n';
    std::cout << "Заказов в перемещённом источнике: "
        << managerAssigned.getOrderCount() << '\n';

    managerMoved.updateStatus(sourceOrder.getId(), OrderStatus::Cancelled);
    std::cout << "Оборот после отмены у нового менеджера: "
        << managerMoved.getTurnover() << '\n';

    OrderManager managerMoveAssigned;
    managerMoveAssigned = std::move(source);
    managerMoveAssigned.updateStatus(sourceOrder.getId(), OrderStatus::Cancelled);

    std::cout << "Бонусы после перемещающего присваивания и отмены: "
        << managerMoveAssigned.getBonusBalance("manager-copy") << '\n';
    std::cout << "Заказов в источнике после присваивания: "
        << source.getOrderCount() << '\n';
}

// тут запускаем примеры работы склада заказов доставки и акций
int main()
{
    SetConsoleOutputCP(CP_UTF8);

    try
    
    {
        // проверяем что товары с одинаковыми характеристиками используют общий тип
        auto milkType1 = ProductType::getType(
            "food", Weight(1.0), Price(120));

        auto milkType2 = ProductType::getType(
            "food", Weight(1.0), Price(120));

        auto otherType = ProductType::getType(
            "food", Weight(2.0), Price(120));

        std::cout << "Одинаковые характеристики — общий объект: "
            << (milkType1 == milkType2) << '\n';

        std::cout << "Разный вес — общий объект: "
            << (milkType1 == otherType) << '\n';
        // 1. Создание товаров через фабрики
        cout << " 1. СОЗДАНИЕ ТОВАРОВ \n";

        Order testOrder(1, "customer-1");

        std::cout << "ID заказа: " << testOrder.getId() << '\n';
        std::cout << "Клиент: " << testOrder.getCustomer() << '\n';
        std::cout << "Количество позиций: "
            << testOrder.getItems().size() << '\n';
        std::cout << "Стоимость заказа: "
            << testOrder.getTotalPrice() << '\n';
        std::cout << "Вес заказа: "
            << testOrder.getTotalWeight() << '\n';

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

        const Product* deliveryMilk = warehouse1.findById(105);

        if (deliveryMilk == nullptr)
        {
            throw std::runtime_error(
                "Не найден товар для проверки доставки"
            );
        }

        class TestOrderObserver : public IObserver
        {
        public:
            unsigned int eventCount = 0;

            OrderStatus lastStatus = OrderStatus::Pending;

            // считаем полученные события и запоминаем последний статус
            void onOrderEvent(const OrderEvent& event) override
            {
                ++eventCount;

                lastStatus = event.newStatus;
            }
        };

        Order deliveryOrder(2, "customer-2");
        deliveryOrder.addItem(*deliveryMilk, 2);

        CourierDelivery courier;
        PickupDelivery pickup;
        PostDelivery post;

        const Delivery* methods[] = {
            &courier,
            &pickup,
            &post
        };

        for (const Delivery* delivery : methods)
        {
            std::cout << "Стоимость: "
                << delivery->getCost(deliveryOrder)
                << " | Срок: "
                << delivery->getEstimatedTime()
                << '\n';
        }

        deliveryOrder.removeItem(105);
        deliveryOrder.addItem(*deliveryMilk, 21);

        std::cout << "Вес заказа: "
            << deliveryOrder.getTotalWeight() << '\n';

        std::cout << "Курьер доступен: "
            << courier.isAvailable(deliveryOrder) << '\n';

        std::cout << "Самовывоз доступен: "
            << pickup.isAvailable(deliveryOrder) << '\n';

        std::cout << "Почта доступна: "
            << post.isAvailable(deliveryOrder) << '\n';
       
        deliveryOrder.setDelivery(
            std::make_shared<PickupDelivery>()
        );

        std::cout << "21 кг с самовывозом: "
            << deliveryOrder.getTotalPrice() << '\n';

        deliveryOrder.removeItem(105);
        deliveryOrder.addItem(*deliveryMilk, 2);

        deliveryOrder.setDelivery(
            std::make_shared<CourierDelivery>()
        );

        std::cout << "2 кг с курьером: "
            << deliveryOrder.getTotalPrice() << '\n';

        deliveryOrder.setDelivery(
            std::make_shared<PostDelivery>()
        );

        std::cout << "2 кг с почтой: "
            << deliveryOrder.getTotalPrice() << '\n';

        PercentDiscount percentDiscount;

        deliveryOrder.applyDiscount(percentDiscount);

        std::cout << "Со скидкой 10% и почтой: "
            << deliveryOrder.getTotalPrice() << '\n';

        deliveryOrder.setDelivery(
            std::make_shared<CourierDelivery>()
        );

        std::cout << "Со скидкой 10% и курьером: "
            << deliveryOrder.getTotalPrice() << '\n';

        try
        {
            deliveryOrder.applyDiscount(percentDiscount);
        }
        catch (const std::exception& error)
        {
            std::cout << "Повторная акция: "
                << error.what() << '\n';
        }

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
        
        CashbackDiscount cashbackDiscount;

        deliveryOrder.applyDiscount(cashbackDiscount);

        std::cout << "Сумма к оплате после cashback: "
            << deliveryOrder.getTotalPrice() << '\n';

        std::cout << "Ожидаемые бонусы: "
            << deliveryOrder.getCashback() << '\n';

        std::cout << "Повторное чтение бонусов: "
            << deliveryOrder.getCashback() << '\n';

        try
        {
            deliveryOrder.applyDiscount(cashbackDiscount);
        }
        catch (const std::exception& error)
        {
            std::cout << "Повторный cashback: "
                << error.what() << '\n';
        }

        FreeDeliveryDiscount freeDeliveryDiscount;

        deliveryOrder.applyDiscount(freeDeliveryDiscount);

        std::cout << "После бесплатной доставки: "
            << deliveryOrder.getTotalPrice() << '\n';

        std::cout << "Бонусы после бесплатной доставки: "
            << deliveryOrder.getCashback() << '\n';

        deliveryOrder.setDelivery(
            std::make_shared<PostDelivery>()
        );

        std::cout << "После смены доставки на почту: "
            << deliveryOrder.getTotalPrice() << '\n';

        try
        {
            deliveryOrder.applyDiscount(freeDeliveryDiscount);
        }
        catch (const std::exception& error)
        {
            std::cout << "Повторная бесплатная доставка: "
                << error.what() << '\n';
        }

        Order heavyFreeOrder(3, "customer-3");

        heavyFreeOrder.addItem(*deliveryMilk, 21);
        heavyFreeOrder.applyDiscount(freeDeliveryDiscount);

        try
        {
            heavyFreeOrder.setDelivery(
                std::make_shared<CourierDelivery>()
            );
        }
        catch (const std::exception& error)
        {
            std::cout << "Бесплатная доставка для 21 кг: "
                << error.what() << '\n';
        }

        auto giftProduct = breadFactory.createProduct(
            107, "Подарочный хлеб", 70, 1, 0.5
        );

        GiftDiscount giftDiscount(*giftProduct);

        deliveryOrder.applyDiscount(giftDiscount);

        std::cout << "Позиций после подарка: "
            << deliveryOrder.getItems().size() << '\n';

        std::cout << "Стоимость с подарком: "
            << deliveryOrder.getTotalPrice() << '\n';

        std::cout << "Вес с подарком: "
            << deliveryOrder.getTotalWeight() << '\n';

        std::cout << "Бонусы с подарком: "
            << deliveryOrder.getCashback() << '\n';

        std::cout << "Цена исходного образца: "
            << giftProduct->getPrice().getValue() << '\n';

        try
        {
            deliveryOrder.applyDiscount(giftDiscount);
        }
        catch (const std::exception& error)
        {
            std::cout << "Повторный подарок: "
                << error.what() << '\n';
        }
        
        Order giftWeightOrder(4, "customer-4");

        giftWeightOrder.addItem(*deliveryMilk, 20);

        giftWeightOrder.setDelivery(
            std::make_shared<CourierDelivery>()
        );

        try
        {
            giftWeightOrder.applyDiscount(giftDiscount);
        }
        catch (const std::exception& error)
        {
            std::cout << "Подарок сверх ограничения веса: "
                << error.what() << '\n';
        }

        std::cout << "Позиций после отказа: "
            << giftWeightOrder.getItems().size() << '\n';

        std::cout << "Вес после отказа: "
            << giftWeightOrder.getTotalWeight() << '\n';

        std::cout << "Стоимость после отказа: "
            << giftWeightOrder.getTotalPrice() << '\n';

        Order proxyOrder(5, "customer-5");

        proxyOrder.addItem(*deliveryMilk, 2);

        auto deliveryProxy = std::make_shared<DeliveryProxy>(
            std::make_shared<CourierDelivery>()
        );

        proxyOrder.setDelivery(deliveryProxy);

        std::cout << "Заказ с доставкой через прокси: "
            << proxyOrder.getTotalPrice() << '\n';

        const double cachedCost =
            deliveryProxy->getCost(proxyOrder);

        std::cout << "Повторный расчёт доставки: "
            << cachedCost << '\n';

        try
        {
            proxyOrder.setRegion("other");
        }
        catch (const std::exception& error)
        {
            std::cout << "Недоступный регион: "
                << error.what() << '\n';
        }

        std::cout << "Регион после отказа: "
            << proxyOrder.getRegion() << '\n';

        std::cout << "Стоимость после отказа: "
            << proxyOrder.getTotalPrice() << '\n';

        std::cout << "Прокси доступен для заказа весом 21 кг: "
            << deliveryProxy->isAvailable(heavyFreeOrder) << '\n';


        TestOrderObserver observer;

        proxyOrder.subscribe(&observer);
        proxyOrder.subscribe(&observer);

        try
        {
            proxyOrder.setStatus(OrderStatus::Shipped);
        }
        catch (const std::exception& error)
        {
            std::cout << "Отправка до оплаты: "
                << error.what() << '\n';
        }

        proxyOrder.setStatus(OrderStatus::Paid);
        proxyOrder.setStatus(OrderStatus::Paid);

        std::cout << "Событий после оплаты: "
            << observer.eventCount << '\n';

        try
        {
            proxyOrder.applyDiscount(percentDiscount);
        }
        catch (const std::exception& error)
        {
            std::cout << "Скидка после оплаты: "
                << error.what() << '\n';
        }

        proxyOrder.setStatus(OrderStatus::Shipped);
        proxyOrder.setStatus(OrderStatus::Delivered);

        std::cout << "Событий после доставки: "
            << observer.eventCount << '\n';

        std::cout << "Подписчик получил Delivered: "
            << (observer.lastStatus == OrderStatus::Delivered)
            << '\n';

        proxyOrder.unsubscribe(&observer);

        heavyFreeOrder.subscribe(&observer);
        heavyFreeOrder.setStatus(OrderStatus::Cancelled);
        heavyFreeOrder.unsubscribe(&observer);

        std::cout << "Событий с учётом отмены: "
            << observer.eventCount << '\n';
        
        demonstrateOrderManager(*deliveryMilk);
        demonstrateCopyAndMove(*deliveryMilk);

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
