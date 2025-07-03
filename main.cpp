#include "Orderbook.h"
#include <iostream>

int main()
{
    Orderbook orderbook;

    const OrderId orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));
    std::cout << "Order book size after adding order: " << orderbook.Size() << std::endl;
    
    orderbook.CancelOrder(orderId);
    std::cout << "Order book size after canceling order: " << orderbook.Size() << std::endl;

    // Test with matching orders
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Buy, 100, 10));
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 100, 5));
    std::cout << "Order book size after matching orders: " << orderbook.Size() << std::endl;

    // Test Fill and Kill
    auto trades = orderbook.AddOrder(std::make_shared<Order>(OrderType::FillAndKill, 4, Side::Sell, 99, 20));
    std::cout << "Number of trades from Fill and Kill order: " << trades.size() << std::endl;
    std::cout << "Final order book size: " << orderbook.Size() << std::endl;

    return 0;
}
